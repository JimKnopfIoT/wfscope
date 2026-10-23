#include "fmsource.h"
#include "fmaudio.h"

#include <QDateTime>
#include <QTimer>
#include <QSettings>
#include <QTime>
#include <QtMath>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>

namespace {
// MTK-FM-ioctls: _IOWR(0xF5, nr, Zeiger) — auf 64 Bit 0xC008F5nn
inline unsigned long fmIoc(unsigned nr)
{
    return _IOWR(0xF5, nr, void *);
}
const unsigned kPowerUp = 0x00, kPowerDown = 0x01, kTune = 0x02, kSetVol = 0x04, kMute = 0x06,
               kGetRssi = 0x07, kRdsOnOff = 0x12, kAnaSwitch = 0x1e, kSoftMute = 0x3f;

// RDS: read() auf /dev/fm liefert die vom Treiber dekodierte struct rds_t (688 Byte).
// Lage der Felder am J2 ermittelt: fertiger PS (8 Zeichen) und fertiger Radiotext (64).
// PS liegt als vier Zeilen zu 8 Zeichen ab 0xf8: 0/1 = Arbeitszeilen, 3 = bestätigt.
const int kRdsSize = 688, kRdsPiOff = 0x18, kRdsPsRowsOff = 0xf8, kRdsPsOff = 0x110, kRdsRtOff = 0x1e2;
const int kStepCh = 10;          // 100 kHz in 10-kHz-Einheiten
const int kBandFirstCh = 8750, kBandLastCh = 10800;   // abgetastet wird immer das ganze Band
const qint64 kRowMs = 1000;      // höchstens eine Wasserfallzeile pro Sekunde

struct TuneParm {                // struct fm_tune_parm
    quint8 err, band, space, hilo, deemphasis;
    quint8 pad;
    quint16 freq;                // 10 kHz
};
struct SoftMute {                // struct fm_softmute_tune_t
    qint32 rssi;                 // roh; dBm = rssi * 3 / 8
    quint16 freq;
    quint16 pad;
    qint32 valid;
};
static_assert(sizeof(TuneParm) == 8, "fm_tune_parm");
static_assert(sizeof(SoftMute) == 12, "fm_softmute_tune_t");
} // namespace

FmSource::FmSource(QObject *parent) : SpectrumSource(parent)
{
    connect(this, &FmSource::rowReady, this,
            [this](const QVector<float> &b, double c, double s) { emit sweep(b, c, s); },
            Qt::QueuedConnection);
    connect(this, &FmSource::workerStatus, this, [this](const QString &s) { setStatus(s); },
            Qt::QueuedConnection);
    connect(this, &FmSource::workerEnded, this, [this] { stop(); }, Qt::QueuedConnection);
    connect(this, &FmSource::stationsReady, this, [this](const QVariantList &l) {
        m_stations = l;
        emit stationsChanged();
    }, Qt::QueuedConnection);
    // Einmal erkannte Sendernamen bleiben (auch über Neustarts) erhalten
    {
        QSettings settings;
        // zuletzt genutzte Frequenz; beim ersten Start der Bandanfang
        m_listenCh = qBound(8750, settings.value(QStringLiteral("fm/lastCh"), 8750).toInt(), 10800);
        m_labelFreqs = settings.value(QStringLiteral("fm/labelFreqs"), true).toBool();
        m_labelNames = settings.value(QStringLiteral("fm/labelNames"), true).toBool();
        // letztes Bandbild in 0,1-dB-Schritten
        for (const QString &v : settings.value(QStringLiteral("fm/lastBand")).toString().split(',', QString::SkipEmptyParts))
            m_lastBand.append(v.toInt() / 10.0f);
        settings.beginGroup(QStringLiteral("fmNames"));
        // Format "pi:name" (pi hex); ältere Einträge ohne PI gelten als ungeprüft
        for (const QString &key : settings.childKeys()) {
            const QString v = settings.value(key).toString();
            FmStationName n;
            const int colon = v.indexOf(':');
            bool ok = false;
            const uint pi = colon == 4 ? v.left(4).toUInt(&ok, 16) : 0;
            n.pi = ok ? quint16(pi) : 0;
            n.name = ok ? v.mid(5) : v;
            m_savedNames.insert(key.toInt(), n);
        }
    }
    connect(this, &FmSource::nameLearned, this, [this](int ch, const QString &name, int pi) {
        FmStationName n;
        n.name = name;
        n.pi = quint16(pi);
        m_savedNames.insert(ch, n);
        QSettings().setValue(QStringLiteral("fmNames/%1").arg(ch),
                             QStringLiteral("%1:%2").arg(pi, 4, 16, QLatin1Char('0')).arg(name));
    }, Qt::QueuedConnection);
    connect(this, &FmSource::rescanState, this, [this](bool on) {
        if (on == m_rescanning) return;
        m_rescanning = on;
        emit rescanningChanged();
    }, Qt::QueuedConnection);
    connect(this, &FmSource::bandScanned, this, [this](const QVector<float> &dbm) {
        m_lastBand = dbm;
        if (m_autoListenAfterScan) {
            m_autoListenAfterScan = false;
            if (m_running && !m_listening)
                setListening(true);
        }
        QStringList v;
        for (float d : dbm)
            v << QString::number(qRound(d * 10));
        QSettings().setValue(QStringLiteral("fm/lastBand"), v.join(','));
    }, Qt::QueuedConnection);
    connect(this, &FmSource::nameForgotten, this, [this](int ch) {
        m_savedNames.remove(ch);
        QSettings().remove(QStringLiteral("fmNames/%1").arg(ch));
    }, Qt::QueuedConnection);
    connect(this, &FmSource::rdsReady, this, [this](const QString &name, const QString &text) {
        if (name == m_rdsName && text == m_rdsText) return;
        m_rdsName = name;
        m_rdsText = text;
        emit rdsChanged();
    }, Qt::QueuedConnection);
    connect(this, &FmSource::levelReady, this, [this](int dbm) {
        m_listenDbm = dbm;
        emit listenLevelChanged();
        setStatus(tr("Listening %1 MHz%3 · %2 dBm").arg(listenMHz(), 0, 'f', 1).arg(dbm)
                      .arg(m_rdsName.isEmpty() ? QString() : QStringLiteral(" · ") + m_rdsName));
    }, Qt::QueuedConnection);
    m_audio = new FmAudio(this);
    m_audio->setVolume(m_volume);
    connect(m_audio, &FmAudio::error, this, [this](const QString &e) { setStatus(e); }, Qt::QueuedConnection);
    connect(this, &SpectrumSource::planChanged, this, [this] {
        QMutexLocker lock(&m_mutex);
        m_viewFirstCh = int(qCeil((m_centerMHz - m_spanMHz / 2) * 10)) * kStepCh;
        m_viewLastCh = qMax(m_viewFirstCh + kStepCh, int(qFloor((m_centerMHz + m_spanMHz / 2) * 10)) * kStepCh);
        ++m_planGen;
    });
    m_centerMHz = 97.75;
    m_spanMHz = 20.5;
    setStatus(tr("Internal FM ready — built-in short antenna"));
}

FmSource::~FmSource()
{
    stop();
}

void FmSource::setListening(bool v)
{
    if (v == m_listening) return;
    m_listening = v;
    m_listenReq = v;
    if (v && !m_running)
        start();
    if (v)
        m_audio->start();
    else
        m_audio->stop();
    emit listenChanged();
}

void FmSource::setListenMHz(double mhz)
{
    const int ch = qBound(8750, int(qRound(mhz * 100.0)), 10800);   // 10-kHz-Einheiten
    if (ch == m_listenCh) return;
    m_listenCh = ch;
    QSettings().setValue(QStringLiteral("fm/lastCh"), ch);
    emit listenChanged();
}

void FmSource::setVolume(int v)
{
    v = qBound(0, v, 100);
    if (v == m_volume) return;
    m_volume = v;
    m_audio->setVolume(v);
    emit listenChanged();
}

void FmSource::listenTo(double mhz)
{
    setListenMHz(mhz);
    setListening(true);
}

void FmSource::setLabelFreqs(bool v)
{
    if (v == m_labelFreqs) return;
    m_labelFreqs = v;
    QSettings().setValue(QStringLiteral("fm/labelFreqs"), v);
    emit settingsChanged();
}

void FmSource::setLabelNames(bool v)
{
    if (v == m_labelNames) return;
    m_labelNames = v;
    QSettings().setValue(QStringLiteral("fm/labelNames"), v);
    emit settingsChanged();
}

void FmSource::setShortAntenna(bool v)
{
    if (v == m_shortAnt) return;
    m_shortAnt = v;
    m_antDirty = true;
    emit settingsChanged();
}

QVariantMap FmSource::rdsInfo(double mhz) const
{
    const int ch = qRound(mhz * 10) * kStepCh;
    QMutexLocker lock(&m_namesMutex);
    const FmRdsStat st = m_rdsStats.value(ch);
    QVariantMap m;
    m.insert(QStringLiteral("listenedSec"), int(st.listenMs / 1000));
    m.insert(QStringLiteral("reads"), st.reads);
    m.insert(QStringLiteral("pi"), st.pi ? QStringLiteral("%1").arg(st.pi, 4, 16, QLatin1Char('0')).toUpper() : QString());
    m.insert(QStringLiteral("dbm"), st.dbm);
    return m;
}

void FmSource::rescan()
{
    if (!m_running)
        return;
    if (!m_listening) {         // ohne Hören tastet die Quelle ohnehin fortlaufend ab
        return;
    }
    m_rescanReq = true;
}

void FmSource::setManualName(double mhz, const QString &name)
{
    const int ch = qRound(mhz * 10) * kStepCh;
    const QString clean = name.simplified();
    quint16 pi = 0;
    {
        QMutexLocker lock(&m_namesMutex);
        if (clean.isEmpty()) {
            m_names.remove(ch);
        } else {
            FmStationName n;
            n.name = clean;
            n.pi = m_rdsStats.value(ch).pi;    // bekannter PI: Name gilt nur für diesen Sender
            n.verified = true;
            pi = n.pi;
            m_names.insert(ch, n);
        }
    }
    if (clean.isEmpty())
        emit nameForgotten(ch);
    else
        emit nameLearned(ch, clean, pi);
    m_namesDirty = true;
}

bool FmSource::ioc(unsigned long cmd, void *arg)
{
    return ::ioctl(m_fd, cmd, arg) == 0;
}

// RDS-Stand lesen; false, wenn nichts Neues vorliegt
bool FmSource::readRds(QString *name, QString *text, bool *confirmed, quint16 *pi)
{
    char buf[kRdsSize];
    const ssize_t n = ::read(m_fd, buf, sizeof buf);
    if (n != kRdsSize)
        return false;
    auto field = [&buf](int off, int len) {
        QString s;
        for (int i = 0; i < len; ++i) {
            const uchar c = uchar(buf[off + i]);
            s += (c >= 0x20 && c < 0x7f) ? QChar(c) : QChar(' ');
        }
        return s.simplified();
    };
    // Bestätigte Zeile, sonst die beiden Arbeitszeilen, wenn sie übereinstimmen und lesbar sind
    auto clean = [&buf](int off) {
        for (int i = 0; i < 8; ++i) {
            const uchar c = uchar(buf[off + i]);
            if (c < 0x20 || c >= 0x7f)
                return false;
        }
        return true;
    };
    *pi = quint16(uchar(buf[kRdsPiOff]) | uchar(buf[kRdsPiOff + 1]) << 8);
    *confirmed = false;
    if (clean(kRdsPsOff) && field(kRdsPsOff, 8) != QString()) {
        *name = field(kRdsPsOff, 8);
        *confirmed = true;
    } else if (clean(kRdsPsRowsOff) && memcmp(buf + kRdsPsRowsOff, buf + kRdsPsRowsOff + 8, 8) == 0)
        *name = field(kRdsPsRowsOff, 8);
    else
        name->clear();
    *text = field(kRdsRtOff, 64);
    return true;
}

void FmSource::start()
{
    if (m_running) return;
    m_names = m_savedNames;                 // vor dem Thread-Start: Thread übernimmt die Namen (ungeprüft)
    for (FmStationName &n : m_names)
        n.verified = false;
    m_fd = ::open("/dev/fm", O_RDWR | O_CLOEXEC | O_NONBLOCK);   // nicht blockierend fürs RDS-read()
    if (m_fd < 0) {
        setStatus(QStringLiteral("/dev/fm: %1").arg(QString::fromLocal8Bit(strerror(errno))));
        return;
    }
    TuneParm tp = { 0, 1 /* Band 87,5–108 */, 1 /* 100 kHz */, 0, 0 /* 50 µs */, 0, 9800 };
    if (!ioc(fmIoc(kPowerUp), &tp) || tp.err != 0) {
        setStatus(tr("The FM chip cannot be powered on"));
        ::close(m_fd);
        m_fd = -1;
        return;
    }
    qint32 one = 1;
    ioc(fmIoc(kMute), &one);              // kein Ton, nur messen
    m_antDirty = true;
    emit planChanged();                    // Kanalbereich aus Mitte/Spanne setzen
    m_stop = false;
    setRunning(true);
    setStatus(tr("Internal FM scanning …"));
    m_thread = std::thread([this] { run(); });
    // Gleich mit Ton auf der zuletzt genutzten Frequenz beginnen. Ohne gespeichertes Bandbild
    // (erster Start) erst einmal das Band abtasten, sonst bliebe das Spektrum leer.
    if (!m_listening) {
        if (m_lastBand.isEmpty())
            m_autoListenAfterScan = true;
        else
            QTimer::singleShot(0, this, [this] { if (m_running && !m_listening) setListening(true); });
    }
}

void FmSource::stop()
{
    if (m_listening) {
        m_listening = false;
        m_listenReq = false;
        m_audio->stop();
        emit listenChanged();
    }
    m_stop = true;
    if (m_thread.joinable())
        m_thread.join();
    if (m_fd >= 0) {
        qint32 zero = 0;
        ioc(fmIoc(kPowerDown), &zero);
        ::close(m_fd);
        m_fd = -1;
        setStatus(tr("Internal FM stopped"));
    }
    setRunning(false);
}

// Abtast-Thread: alle Kanäle des Bands reihum per Soft-Mute-Tune messen, etwa jede
// Sekunde eine Zeile fürs angezeigte Fenster ausgeben. Die Messwerte bleiben beim
// Verschieben der Ansicht erhalten; das Fenster ist nur ein Ausschnitt.
void FmSource::run()
{
    const int first = kBandFirstCh, last = kBandLastCh;
    // −130 = noch nicht gemessen: zählt weder für die Farbskala noch für die Sendererkennung
    QVector<float> dbm((last - first) / kStepCh + 1, -130.0f);
    const bool haveLast = m_lastBand.size() == dbm.size();
    if (haveLast)                           // Startbild: letzter vollständiger Durchlauf
        dbm = m_lastBand;
    QVector<bool> valid(dbm.size(), false);
    if (haveLast)
        emitStations(dbm, valid, first);    // Senderliste/Beschriftung sofort
    int gen = -1, ch = first;
    // Zeile fürs aktuelle Fenster; außerhalb des UKW-Bands leer (unter dem Rauschboden)
    auto emitRow = [&] {
        int viewFirst, viewLast;
        {
            QMutexLocker lock(&m_mutex);
            gen = m_planGen;
            viewFirst = m_viewFirstCh;
            viewLast = m_viewLastCh;
        }
        QVector<float> row((viewLast - viewFirst) / kStepCh + 1, -130.0f);
        for (int i = 0; i < dbm.size(); ++i) {
            const int j = (first + i * kStepCh - viewFirst) / kStepCh;
            if (j >= 0 && j < row.size())
                row[j] = dbm[i];
        }
        const double lo = viewFirst / 100.0, hi = viewLast / 100.0;
        // Bins = Kanalmitten, erster und letzter Kanal liegen auf den Achsenrändern
        emit rowReady(row, (lo + hi) / 2.0, hi - lo);
    };
    qint64 lastRow = QDateTime::currentMSecsSinceEpoch();
    qint64 sweepStart = lastRow;
    int errors = 0;
    while (!m_stop) {
        if (gen != m_planGen) {                // Ansicht verschoben: sofort neu zeichnen
            lastRow = QDateTime::currentMSecsSinceEpoch();
            emitRow();
        }
        if (m_antDirty.exchange(false)) {
            qint32 ana = m_shortAnt ? 1 : 0;   // 1 = kurze (eingebaute) Antenne
            ioc(fmIoc(kAnaSwitch), &ana);
        }
        if (m_listenReq) {
            // Hörmodus: fest abstimmen, Ton an, nur noch Pegel messen
            int tuned = -1;
            qint32 v = 0;
            QString candidate, shownName, shownText;
            int candidateCount = 0;
            quint16 stablePi = 0;
            QVector<quint16> piRecent;              // letzte PI-Lesungen für den Mehrheitsentscheid
            const bool debug = qEnvironmentVariableIsSet("WFSCOPE_DEBUG");
            auto stamp = [] { return QTime::currentTime().toString(QStringLiteral("HH:mm:ss")).toLocal8Bit(); };
            qint64 lastIter = QDateTime::currentMSecsSinceEpoch();
            while (m_listenReq && !m_stop) {
                if (tuned != m_listenCh) {
                    tuned = m_listenCh;
                    quint16 rds = 0;                   // RDS neu aufsetzen: alter Sender soll nicht nachwirken
                    ioc(fmIoc(kRdsOnOff), &rds);
                    TuneParm tp = { 0, 1, 1, 0, 0, 0, quint16(tuned) };
                    ioc(fmIoc(kTune), &tp);
                    v = 0;
                    ioc(fmIoc(kMute), &v);
                    v = 15;
                    ioc(fmIoc(kSetVol), &v);
                    rds = 1;
                    ioc(fmIoc(kRdsOnOff), &rds);
                    if (debug)
                        fprintf(stderr, "%s FmSource: abgestimmt %d\n", stamp().constData(), tuned);
                    QString dummyName, dummyText;
                    bool dummyConfirmed;
                    quint16 dummyPi;
                    readRds(&dummyName, &dummyText, &dummyConfirmed, &dummyPi);   // Reststand verwerfen
                    candidate.clear();
                    candidateCount = 0;
                    stablePi = 0;
                    piRecent.clear();
                    {
                        QMutexLocker lock(&m_namesMutex);
                        shownName = m_names.value(tuned).name;   // gespeichert, evtl. noch ungeprüft
                    }
                    lastIter = QDateTime::currentMSecsSinceEpoch();
                    shownText.clear();
                    emit rdsReady(shownName, shownText);
                }
                {
                    QString name, text;
                    bool confirmed = false;
                    quint16 pi = 0;
                    if (readRds(&name, &text, &confirmed, &pi)) {
                        QMutexLocker lock(&m_namesMutex);
                        ++m_rdsStats[tuned].reads;
                        if (debug)
                            fprintf(stderr, "%s RDS: PI %04x PS '%s'%s\n", stamp().constData(), pi, qPrintable(name),
                                    confirmed ? " (bestätigt)" : "");
                        // PI-Code: der Rohwert der letzten Gruppe hat oft Bitfehler. Sicher ist ein PI
                        // erst, wenn er in den letzten 8 Lesungen mindestens 4× vorkommt.
                        if (pi != 0) {
                            piRecent.append(pi);
                            if (piRecent.size() > 8)
                                piRecent.removeFirst();
                        }
                        quint16 majority = 0;
                        for (quint16 p : piRecent)
                            if (piRecent.count(p) >= 4)
                                majority = p;
                        if (majority != 0 && stablePi != majority) {
                            stablePi = majority;
                            m_rdsStats[tuned].pi = stablePi;
                            auto it = m_names.find(tuned);
                            if (it != m_names.end() && it->pi != 0 && it->pi != stablePi) {
                                // anderer Sender auf dieser Frequenz: alten Namen verwerfen
                                if (debug)
                                    fprintf(stderr, "%s FmSource: PI %04x statt %04x — Name '%s' verworfen\n",
                                            stamp().constData(), stablePi, it->pi, qPrintable(it->name));
                                m_names.erase(it);
                                emit nameForgotten(tuned);
                                shownName.clear();
                                emitStations(dbm, valid, first);
                            } else if (it != m_names.end() && !it->verified) {
                                it->verified = true;
                                if (it->pi == 0) {          // alter Eintrag ohne PI: jetzt ergänzen
                                    it->pi = stablePi;
                                    emit nameLearned(tuned, it->name, stablePi);
                                }
                                emitStations(dbm, valid, first);
                            }
                        }
                        // Name: mehrfach gleich gelesen (ohne Sonderzeichen 2×, sonst 3×) und nur
                        // mit gesichertem PI; ein geprüfter Name wird nur von der bestätigten Zeile ersetzt
                        candidateCount = (!name.isEmpty() && name == candidate) ? candidateCount + 1 : 1;
                        candidate = name;
                        bool plain = true;
                        for (QChar c : name)
                            if (!c.isLetterOrNumber() && c != ' ' && c != '-' && c != '.' && c != '&' && c != '+')
                                plain = false;
                        const bool accept = !name.isEmpty() && name != shownName && stablePi != 0
                                            && candidateCount >= (plain ? 2 : 3)
                                            && (shownName.isEmpty() || confirmed);
                        if (accept) {
                            shownName = name;
                            FmStationName n;
                            n.name = name;
                            n.pi = stablePi;
                            n.verified = true;
                            m_names.insert(tuned, n);
                            emit nameLearned(tuned, name, stablePi);
                            if (debug)
                                fprintf(stderr, "%s FmSource: RDS-Name '%s' (PI %04x)\n", stamp().constData(), qPrintable(name), stablePi);
                            emitStations(dbm, valid, first);
                        }
                        if (!text.isEmpty())
                            shownText = text;
                        emit rdsReady(shownName, shownText);
                    }
                }
                if (m_antDirty.exchange(false)) {
                    qint32 ana = m_shortAnt ? 1 : 0;
                    ioc(fmIoc(kAnaSwitch), &ana);
                }
                if (m_rescanReq.exchange(false)) {
                    // Band einmal neu abtasten: Ton stumm, alle Kanäle messen, dann zurück zum Sender
                    emit rescanState(true);
                    v = 1;
                    ioc(fmIoc(kMute), &v);
                    quint16 rds = 0;
                    ioc(fmIoc(kRdsOnOff), &rds);
                    for (int c = first; c <= last && m_listenReq && !m_stop; c += kStepCh) {
                        SoftMute sm = { 0, quint16(c), 0, 0 };
                        if (ioc(fmIoc(kSoftMute), &sm)) {
                            dbm[(c - first) / kStepCh] = sm.rssi * 3.0f / 8.0f;
                            valid[(c - first) / kStepCh] = sm.valid != 0;
                        }
                        const qint64 now = QDateTime::currentMSecsSinceEpoch();
                        if (now - lastRow >= kRowMs) {
                            lastRow = now;
                            emitRow();
                        }
                    }
                    emitStations(dbm, valid, first);
                    emit bandScanned(dbm);
                    emit rescanState(false);
                    tuned = -1;                          // neu abstimmen, Ton und RDS wieder an
                    continue;
                }
                if (m_namesDirty.exchange(false)) {      // Name von Hand gesetzt
                    {
                        QMutexLocker lock(&m_namesMutex);
                        shownName = m_names.value(tuned).name;
                    }
                    emit rdsReady(shownName, shownText);
                    emitStations(dbm, valid, first);
                }
                qint32 rssi = 0;
                if (ioc(fmIoc(kGetRssi), &rssi)) {
                    emit levelReady(rssi);
                    {
                        const qint64 nowIter = QDateTime::currentMSecsSinceEpoch();
                        QMutexLocker lock(&m_namesMutex);
                        FmRdsStat &st = m_rdsStats[tuned];
                        st.listenMs += nowIter - lastIter;
                        st.dbm = rssi;
                        lastIter = nowIter;
                    }
                    // Wasserfall weiterführen: letztes Bandbild, gehörter Kanal mit Live-Pegel
                    const int i = (tuned - first) / kStepCh;
                    if (i >= 0 && i < dbm.size() && (tuned - first) % kStepCh == 0)
                        dbm[i] = float(rssi);
                    const qint64 now = QDateTime::currentMSecsSinceEpoch();
                    if (now - lastRow >= kRowMs || gen != m_planGen) {
                        lastRow = now;
                        emitRow();
                    }
                }
                for (int i = 0; i < 5 && m_listenReq && !m_stop && tuned == m_listenCh; ++i)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            v = 1;
            ioc(fmIoc(kMute), &v);
            quint16 rds = 0;
            ioc(fmIoc(kRdsOnOff), &rds);
            emit rdsReady(QString(), QString());
            continue;
        }
        if (m_namesDirty.exchange(false))
            emitStations(dbm, valid, first);
        SoftMute sm = { 0, quint16(ch), 0, 0 };
        if (ioc(fmIoc(kSoftMute), &sm)) {
            dbm[(ch - first) / kStepCh] = sm.rssi * 3.0f / 8.0f;
            valid[(ch - first) / kStepCh] = sm.valid != 0;
            errors = 0;
        } else if (++errors > 20) {
            emit workerStatus(tr("FM measurement failed (%1)").arg(QString::fromLocal8Bit(strerror(errno))));
            emit workerEnded();
            return;
        }
        ch += kStepCh;
        const bool wrapped = ch > last;
        if (wrapped)
            ch = first;
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - lastRow >= kRowMs) {
            lastRow = now;
            emitRow();
        }
        if (wrapped) {
            emitStations(dbm, valid, first);
            emit bandScanned(dbm);
            emit workerStatus(tr("Internal FM · %1 channels · %2 s/sweep · %3")
                                  .arg(dbm.size())
                                  .arg((now - sweepStart) / 1000.0, 0, 'f', 1)
                                  .arg(m_shortAnt ? tr("short antenna") : tr("headset antenna")));
            sweepStart = now;
        }
    }
}

// Senderliste: lokale Spitzen ≥ 8 dB über dem Rauschboden (20-%-Perzentil)
// oder vom Chip als gültig gemeldet; stärkste zuerst, höchstens 30
void FmSource::emitStations(const QVector<float> &dbm, const QVector<bool> &valid, int firstCh)
{
    if (dbm.size() < 3)
        return;
    QVector<float> sorted;
    for (float d : dbm)
        if (d > -129.0f)                // nur gemessene Kanäle
            sorted.append(d);
    if (sorted.size() < 3)
        return;
    std::sort(sorted.begin(), sorted.end());
    const float floorDb = sorted[sorted.size() / 5];
    struct St { double mhz; float db; bool valid; };
    QVector<St> list;
    for (int i = 1; i + 1 < dbm.size(); ++i) {
        const bool peak = dbm[i] > -129.0f && dbm[i] >= dbm[i - 1] && dbm[i] >= dbm[i + 1];
        if (peak && (dbm[i] >= floorDb + 8.0f || valid[i]))
            list.append({ (firstCh + i * kStepCh) / 100.0, dbm[i], valid[i] });
    }
    std::sort(list.begin(), list.end(), [](const St &a, const St &b) { return a.db > b.db; });
    QVariantList out;
    for (int i = 0; i < list.size() && i < 30; ++i) {
        QVariantMap m;
        m.insert(QStringLiteral("mhz"), list[i].mhz);
        m.insert(QStringLiteral("dbm"), qRound(list[i].db));
        m.insert(QStringLiteral("valid"), list[i].valid);
        FmStationName n;
        {
            QMutexLocker lock(&m_namesMutex);
            n = m_names.value(qRound(list[i].mhz * 100));
        }
        m.insert(QStringLiteral("name"), n.name);
        m.insert(QStringLiteral("verified"), n.verified);
        out << m;
    }
    emit stationsReady(out);
}
