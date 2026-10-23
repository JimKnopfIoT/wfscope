#ifndef FMSOURCE_H
#define FMSOURCE_H

#include "spectrumsource.h"
#include <QHash>
#include <QMutex>
#include <atomic>
#include <thread>

class FmAudio;

// Per RDS gelernter Sendername; gilt erst als geprüft, wenn der PI-Code des Senders passt
struct FmStationName {
    QString name;
    quint16 pi = 0;          // RDS-Senderkennung (0 = unbekannt)
    bool verified = false;   // in dieser Sitzung per PI bestätigt
};

// Was RDS beim Hören eines Senders geliefert hat (für die Erklärung, warum ein Name fehlt)
struct FmRdsStat {
    qint64 listenMs = 0;     // gesamte Hördauer
    int reads = 0;           // RDS-Lesungen mit neuen Daten
    quint16 pi = 0;          // gesicherter PI-Code (0 = keiner)
    int dbm = -130;          // zuletzt gemessener Pegel
};

// Eingebauter UKW-Empfänger (MediaTek connac2x FM, /dev/fm). Kein IQ: pro
// 100-kHz-Kanal ein Pegel über den Soft-Mute-Tune des Treibers (dBm = roh × 3/8),
// ~50 ms je Kanal. Läuft ohne root (Gruppe media) und neben dem WLAN.
class FmSource : public SpectrumSource
{
    Q_OBJECT
    Q_PROPERTY(bool shortAntenna READ shortAntenna WRITE setShortAntenna NOTIFY settingsChanged)
    // Hörmodus: Chip fest abstimmen, Ton über FmAudio; Durchlauf pausiert
    Q_PROPERTY(bool listening READ listening WRITE setListening NOTIFY listenChanged)
    Q_PROPERTY(double listenMHz READ listenMHz WRITE setListenMHz NOTIFY listenChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY listenChanged)
    Q_PROPERTY(int listenDbm READ listenDbm NOTIFY listenLevelChanged)
    // RDS des gehörten Senders: Programmname (PS) und Radiotext
    Q_PROPERTY(QString rdsName READ rdsName NOTIFY rdsChanged)
    Q_PROPERTY(QString rdsText READ rdsText NOTIFY rdsChanged)
    // Beschriftung im Spektrum (gespeichert)
    Q_PROPERTY(bool labelFreqs READ labelFreqs WRITE setLabelFreqs NOTIFY settingsChanged)
    Q_PROPERTY(bool labelNames READ labelNames WRITE setLabelNames NOTIFY settingsChanged)
    // Beim Hören das Band einmal neu abtasten (Ton pausiert so lange)
    Q_PROPERTY(bool rescanning READ rescanning NOTIFY rescanningChanged)
    // Senderliste aus dem letzten vollen Durchlauf: [{mhz, dbm, valid, name}]
    Q_PROPERTY(QVariantList stations READ stations NOTIFY stationsChanged)
public:
    explicit FmSource(QObject *parent = nullptr);
    ~FmSource() override;

    QString name() const override { return tr("FM radio (internal)"); }
    QString kind() const override { return QStringLiteral("fm"); }
    double minMHz() const override { return 87.5; }
    double maxMHz() const override { return 108.0; }
    QVariantList bands() const override
    {
        // nur das ganze Band; Ausschnitte per Zoom und Schieben
        return QVariantList() << band(tr("FM 87.5–108"), 97.75, 20.5);
    }

    bool shortAntenna() const { return m_shortAnt; }
    void setShortAntenna(bool v);
    bool listening() const { return m_listening; }
    void setListening(bool v);
    double listenMHz() const { return m_listenCh / 100.0; }
    void setListenMHz(double mhz);
    int volume() const { return m_volume; }
    void setVolume(int v);
    int listenDbm() const { return m_listenDbm; }
    QVariantList stations() const { return m_stations; }
    QString rdsName() const { return m_rdsName; }
    QString rdsText() const { return m_rdsText; }
    bool labelFreqs() const { return m_labelFreqs; }
    void setLabelFreqs(bool v);
    bool labelNames() const { return m_labelNames; }
    void setLabelNames(bool v);
    // Warum fehlt der Name? {listenedSec, reads, pi, dbm}
    Q_INVOKABLE QVariantMap rdsInfo(double mhz) const;
    // Namen von Hand setzen (mit dem PI, falls bekannt)
    Q_INVOKABLE void setManualName(double mhz, const QString &name);
    Q_INVOKABLE void rescan();
    bool rescanning() const { return m_rescanning; }
    // Sender antippen: abstimmen und hören
    Q_INVOKABLE void listenTo(double mhz);

    void start() override;
    void stop() override;

signals:
    void settingsChanged();
    void listenChanged();
    void listenLevelChanged();
    void stationsChanged();
    void stationsReady(const QVariantList &list);
    void levelReady(int dbm);
    void rdsChanged();
    void rdsReady(const QString &name, const QString &text);
    void nameLearned(int ch, const QString &name, int pi);
    void nameForgotten(int ch);
    void rescanningChanged();
    void rescanState(bool on);
    void bandScanned(const QVector<float> &dbm);
    // aus dem Abtast-Thread, queued in den Hauptthread
    void rowReady(const QVector<float> &bins, double centerMHz, double spanMHz);
    void workerStatus(const QString &s);
    void workerEnded();

private:
    void run();
    void runDemo();
    void emitStations(const QVector<float> &dbm, const QVector<bool> &valid, int firstCh);
    bool ioc(unsigned long cmd, void *arg);
    bool readRds(QString *name, QString *text, bool *confirmed, quint16 *pi);

    int m_fd = -1;
    std::thread m_thread;
    std::atomic<bool> m_stop { false };
    std::atomic<bool> m_antDirty { false };
    bool m_shortAnt = true;
    FmAudio *m_audio = nullptr;
    bool m_listening = false;
    std::atomic<int> m_listenCh { 8750 };       // 10 kHz; beim Start die zuletzt genutzte
    std::atomic<bool> m_listenReq { false };    // Thread soll in den Hörmodus
    int m_volume = 50;                          // 50 % = Verstärkung 1,0
    int m_listenDbm = -120;
    QVariantList m_stations;
    QString m_rdsName, m_rdsText;
    bool m_labelFreqs = true, m_labelNames = true;
    // Entwicklerhilfe: WFSCOPE_FM_DEMO=1 zeigt ohne Chip ein Demo-Band (Standort Brandenburger Tor,
    // Berliner Sender) für Screenshots; nichts davon wird gespeichert
    const bool m_demo = qEnvironmentVariableIsSet("WFSCOPE_FM_DEMO");
    bool m_autoListenAfterScan = false;   // erster Start: erst ein Durchlauf, dann Ton
    QVector<float> m_lastBand;      // letztes vollständiges Bandbild (gespeichert), Startwert fürs Spektrum
    // Namen und RDS-Statistik je Kanal (10 kHz); Abtast-Thread und Oberfläche, daher m_namesMutex
    mutable QMutex m_namesMutex { QMutex::Recursive };
    QHash<int, FmStationName> m_names;
    QHash<int, FmRdsStat> m_rdsStats;
    std::atomic<bool> m_namesDirty { false };  // Senderliste neu melden (manueller Name)
    std::atomic<bool> m_rescanReq { false };
    bool m_rescanning = false;
    QHash<int, FmStationName> m_savedNames;  // dauerhaft gespeichert (Hauptthread)

    // Abtastplan (Kanäle in 10 kHz), vom Hauptthread gesetzt
    QMutex m_mutex;
    int m_viewFirstCh = 8750, m_viewLastCh = 10800;  // angezeigtes Fenster, darf übers Band ragen
    std::atomic<int> m_planGen { 0 };
};

#endif
