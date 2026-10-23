#include "icapsource.h"
#include "fft.h"

#include <QDateTime>
#include <QFile>
#include <QtEndian>
#include <QtMath>
#include <complex>
#include <unistd.h>

namespace {

const char *kHelperPath = "/usr/libexec/wfscope/wfscope-icap";
const double kFsMHz = 320.0;
// Unkalibriert: legt das In-Band-Rauschen (Spitzenwert über 32 Segmente) grob auf -95 dBm.
const float kCalDb = -115.0f;

#pragma pack(push, 1)
struct FrameHdr {
    char magic[4];
    quint16 version;
    quint16 status;
    quint32 channel;
    quint32 bw;
    quint32 band;
    quint32 fsHz;
    quint32 nsamples;
};
#pragma pack(pop)

const char *statusText(quint16 s)
{
    switch (s) {
    case 1: return "ioctl fehlgeschlagen";
    case 2: return "Capture-Timeout";
    case 3: return "keine Daten";
    case 4: return "Kommando ungültig";
    default: return "Fehler";
    }
}

// Kanalnummer und Band zu einer Mittenfrequenz (5-MHz-Raster).
void channelFor(double fMHz, int *ch, int *band)
{
    if (fMHz < 3000.0) {
        *band = 0;
        *ch = qBound(1, qRound((fMHz - 2407.0) / 5.0), 13);
    } else if (fMHz < 5935.0) {
        *band = 1;
        *ch = qRound((fMHz - 5000.0) / 5.0);
    } else {
        *band = 2;
        *ch = qRound((fMHz - 5950.0) / 5.0);
    }
}

double centerFor(int ch, int band)
{
    switch (band) {
    case 0: return 2407.0 + 5.0 * ch;
    case 1: return 5000.0 + 5.0 * ch;
    default: return 5950.0 + 5.0 * ch;
    }
}

} // namespace

IcapSource::IcapSource(QObject *parent) : SpectrumSource(parent)
{
    connect(&m_proc, &QProcess::readyReadStandardOutput, this, &IcapSource::onReadyRead);
    connect(&m_proc, &QProcess::readyReadStandardError, this, &IcapSource::onErrorOutput);
    connect(&m_proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &IcapSource::onFinished);
    connect(this, &SpectrumSource::planChanged, this, [this] {
        if (m_running)
            planChunks();
    });
    setStatus(tr("ICAP ready — starting switches Wi-Fi off"));
}

IcapSource::~IcapSource()
{
    // Helfer räumt bei stdin-EOF selbst auf; kurz warten, damit das passiert.
    if (m_proc.state() != QProcess::NotRunning) {
        m_proc.write("quit\n");
        m_proc.closeWriteChannel();
        if (!m_proc.waitForFinished(3000))
            m_proc.terminate();
        m_proc.waitForFinished(2000);
    }
}

void IcapSource::start()
{
    if (m_proc.state() != QProcess::NotRunning)
        return;
    QString helper = QString::fromLocal8Bit(qgetenv("WFSCOPE_HELPER"));
    if (helper.isEmpty())
        helper = QString::fromLatin1(kHelperPath);
    if (!QFile::exists(helper)) {
        setStatus(tr("Helper missing: %1").arg(helper));
        return;
    }
    m_buf.clear();
    m_errors = 0;
    m_stopping = false;
    planChunks();
    setStatus(tr("ICAP starting (Wi-Fi off) …"));
    if (geteuid() == 0)
        m_proc.start(helper, QStringList());
    else
        m_proc.start(QStringLiteral("pkexec"), QStringList() << helper);
    setRunning(true);
}

void IcapSource::stop()
{
    if (m_proc.state() == QProcess::NotRunning) {
        setRunning(false);
        return;
    }
    m_stopping = true;
    setStatus(tr("Stopping, restoring Wi-Fi …"));
    m_proc.write("quit\n");
    m_proc.closeWriteChannel();
}

void IcapSource::planChunks()
{
    m_chunks.clear();
    int ch, band;
    channelFor(m_centerMHz, &ch, &band);
    const int bw = band == 0 ? 1 : 2;              // 2,4 GHz: 40 MHz, sonst 80 MHz
    const double width = band == 0 ? 40.0 : 80.0;
    const double usable = width * 0.9;             // Filterflanken abschneiden
    const double lo = m_centerMHz - m_spanMHz / 2.0, hi = m_centerMHz + m_spanMHz / 2.0;
    const int n = qMax(1, int(qCeil(m_spanMHz / usable)));
    for (int i = 0; i < n; ++i) {
        const double want = lo + usable * (i + 0.5) * (m_spanMHz / (usable * n));
        const double f = qBound(lo, want, hi);
        // nur WLAN-Bänder: dazwischen gibt es keine gültigen Kanäle
        const bool inWifi = (f >= 2400.0 && f <= 2500.0) || (f >= 5150.0 && f <= 7125.0);
        if (!inWifi)
            continue;
        channelFor(f, &ch, &band);
        Chunk c;
        c.channel = ch;
        c.bw = bw;
        c.band = band;
        c.centerMHz = centerFor(ch, band);
        c.usableMHz = usable;
        if (!m_chunks.isEmpty() && m_chunks.last().channel == ch)
            continue;
        m_chunks.append(c);
    }
    m_next = 0;
    m_sweepStartMs = QDateTime::currentMSecsSinceEpoch();
    if (m_chunks.isEmpty())
        setStatus(tr("The range contains no Wi-Fi band (2.4 / 5 / 6 GHz)"));
}

void IcapSource::requestNext()
{
    if (m_stopping || m_chunks.isEmpty())
        return;
    if (m_next >= m_chunks.size())
        m_next = 0;
    const Chunk &c = m_chunks.at(m_next);
    m_proc.write(QStringLiteral("cap %1 %2 %3\n").arg(c.channel).arg(c.bw).arg(c.band).toLatin1());
}

void IcapSource::onErrorOutput()
{
    const QByteArray err = m_proc.readAllStandardError();
    if (err.contains("bereit")) {
        setStatus(tr("ICAP active (Wi-Fi off)"));
        requestNext();
    } else if (!err.trimmed().isEmpty() && !err.contains("wiederhergestellt")) {
        setStatus(QString::fromLocal8Bit(err.trimmed()));
    }
}

void IcapSource::onReadyRead()
{
    m_buf.append(m_proc.readAllStandardOutput());
    while (m_buf.size() >= int(sizeof(FrameHdr))) {
        FrameHdr h;
        memcpy(&h, m_buf.constData(), sizeof(h));
        if (memcmp(h.magic, "WFIQ", 4) != 0) {
            setStatus(tr("Protocol error from the helper"));
            m_buf.clear();
            stop();
            return;
        }
        const int need = int(sizeof(FrameHdr)) + int(h.nsamples) * 4;
        if (m_buf.size() < need)
            return;
        const QByteArray iq = m_buf.mid(sizeof(FrameHdr), int(h.nsamples) * 4);
        m_buf.remove(0, need);
        handleFrame(h.channel, h.status, iq);
    }
}

void IcapSource::handleFrame(quint32 channel, quint16 status, const QByteArray &iq)
{
    if (m_chunks.isEmpty())
        return;
    if (m_next >= m_chunks.size() || m_chunks.at(m_next).channel != int(channel)) {
        // Plan hat sich geändert, während der Capture lief: verwerfen, neu anfordern.
        m_next = 0;
        requestNext();
        return;
    }
    if (status != 0 || iq.isEmpty()) {
        ++m_errors;
        setStatus(tr("Capture channel %1: %2")
                      .arg(m_chunks.at(m_next).channel).arg(QLatin1String(statusText(status))));
    } else {
        m_chunks[m_next].psd = psdFromIq(iq);
    }
    // nach jedem Stück eine Wasserfallzeile; Status nur nach vollem Durchlauf
    const bool full = ++m_next >= m_chunks.size();
    emitSweep(full);
    if (full)
        m_next = 0;
    requestNext();
}

QVector<float> IcapSource::psdFromIq(const QByteArray &iq) const
{
    const int n = iq.size() / 4;
    const qint16 *s = reinterpret_cast<const qint16 *>(iq.constData());
    const int segs = n / kFft;
    QVector<double> acc(kFft, 0.0);
    QVector<std::complex<float>> buf(kFft);
    double winPow = 0.0;
    const QVector<float> win = wf::hann(kFft, &winPow);
    for (int sgm = 0; sgm < segs; ++sgm) {
        const qint16 *p = s + 2 * sgm * kFft;
        for (int i = 0; i < kFft; ++i)
            buf[i] = std::complex<float>(p[2 * i] * win[i], p[2 * i + 1] * win[i]);
        wf::fft(buf.data(), kFft);
        // Spitzenwert-Detektor: kurze Bursts innerhalb der 102 µs bleiben sichtbar
        for (int i = 0; i < kFft; ++i)
            acc[(i + kFft / 2) % kFft] = qMax(acc[(i + kFft / 2) % kFft], double(std::norm(buf[i])));
    }
    QVector<float> db(kFft);
    const double norm = double(kFft) * winPow;
    for (int i = 0; i < kFft; ++i)
        db[i] = 10.0f * std::log10(float(acc[i] / norm) + 1e-12f) + kCalDb;
    // DC-Spitze (LO-Durchgriff) mit den Nachbarn glätten
    db[kFft / 2] = 0.5f * (db[kFft / 2 - 2] + db[kFft / 2 + 2]);
    db[kFft / 2 - 1] = db[kFft / 2 - 2];
    db[kFft / 2 + 1] = db[kFft / 2 + 2];
    return db;
}

void IcapSource::emitSweep(bool fullCycle)
{
    QVector<float> out(kOutBins, -120.0f);
    const double lo = m_centerMHz - m_spanMHz / 2.0;
    const double binMHz = kFsMHz / kFft;
    for (int i = 0; i < kOutBins; ++i) {
        const double f = lo + m_spanMHz * i / double(kOutBins - 1);
        float best = -120.0f;
        double bestDist = 1e9;
        for (const Chunk &c : m_chunks) {
            const double off = f - c.centerMHz;
            if (c.psd.isEmpty() || qAbs(off) > c.usableMHz / 2.0 || qAbs(off) >= bestDist)
                continue;
            const double k = kFft / 2 + off / binMHz;
            const int k0 = qBound(0, int(std::floor(k)), kFft - 2);
            const float t = float(k - k0);
            best = c.psd[k0] * (1.0f - t) + c.psd[k0 + 1] * t;
            bestDist = qAbs(off);
        }
        out[i] = best;
    }
    if (!fullCycle) {
        emit sweep(out, m_centerMHz, m_spanMHz);
        return;
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const double secs = qMax<qint64>(1, now - m_sweepStartMs) / 1000.0;
    m_sweepStartMs = now;
    setStatus(tr("ICAP active (Wi-Fi off) · %1 chunks · %2 s/sweep%3")
                  .arg(m_chunks.size()).arg(secs, 0, 'f', 1)
                  .arg(m_errors ? tr(" · %1 errors").arg(m_errors) : QString()));
    emit sweep(out, m_centerMHz, m_spanMHz);
}

void IcapSource::onFinished(int code, QProcess::ExitStatus st)
{
    setRunning(false);
    if (m_stopping && code == 0)
        setStatus(tr("Stopped, Wi-Fi restored"));
    else if (code == 126 || code == 127)
        setStatus(tr("pkexec: not authorized (polkit rule installed?)"));
    else if (st == QProcess::CrashExit)
        setStatus(tr("Helper crashed — the watchdog restores Wi-Fi"));
    else
        setStatus(tr("Helper exited (code %1)").arg(code));
    m_stopping = false;
}
