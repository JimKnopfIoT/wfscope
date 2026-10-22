#ifndef SDRPLAYSOURCE_H
#define SDRPLAYSOURCE_H

#include "spectrumsource.h"
#include <QMutex>
#include <atomic>
#include <complex>
#include <sdrplay_api.h>

// SDRplay RSPdx (und andere RSPs) über die proprietäre SDRplay API v3.
// Die Bibliothek wird zur Laufzeit per dlopen geladen, damit wfscope auch ohne
// installierte API startet; der Dienst sdrplay_apiService muss laufen.
//
// Panadapter-Modus (FT-991A mit TSP IFace 2): der Tuner sitzt fest auf der
// ersten ZF (69,450 MHz + Feinkorrektur), die Achse zeigt die VFO-Frequenz.
// Ohne Panadapter-Modus wird das gewählte Band direkt (ggf. in Schritten) empfangen.
class SdrplaySource : public SpectrumSource
{
    Q_OBJECT
    Q_PROPERTY(bool panadapter READ panadapter WRITE setPanadapter NOTIFY settingsChanged)
    Q_PROPERTY(double vfoMHz READ vfoMHz WRITE setVfoMHz NOTIFY settingsChanged)
    Q_PROPERTY(double ifMHz READ ifMHz WRITE setIfMHz NOTIFY settingsChanged)
    Q_PROPERTY(double offsetKHz READ offsetKHz WRITE setOffsetKHz NOTIFY settingsChanged)
    Q_PROPERTY(bool mirror READ mirror WRITE setMirror NOTIFY settingsChanged)
    Q_PROPERTY(int antenna READ antenna WRITE setAntenna NOTIFY settingsChanged)
    Q_PROPERTY(int lnaState READ lnaState WRITE setLnaState NOTIFY settingsChanged)
    Q_PROPERTY(bool agc READ agc WRITE setAgc NOTIFY settingsChanged)
public:
    explicit SdrplaySource(QObject *parent = nullptr);
    ~SdrplaySource() override;

    QString name() const override { return QStringLiteral("SDRplay RSPdx"); }
    QString kind() const override { return QStringLiteral("sdrplay"); }
    QVariantList bands() const override
    {
        return QVariantList()
            << band(QStringLiteral("UKW 87,5–108"), 97.75, 20.5)
            << band(QStringLiteral("Flugfunk 118–137"), 127.5, 19)
            << band(QStringLiteral("2 m (144–146)"), 145, 2)
            << band(QStringLiteral("70 cm (430–440)"), 435, 10)
            << band(QStringLiteral("40 m (7,0–7,2)"), 7.1, 0.2)
            << band(QStringLiteral("20 m (14,0–14,35)"), 14.175, 0.35);
    }

    bool panadapter() const { return m_panadapter; }
    double vfoMHz() const { return m_vfoMHz; }
    double ifMHz() const { return m_ifMHz; }
    double offsetKHz() const { return m_offsetKHz; }
    bool mirror() const { return m_mirror; }
    int antenna() const { return m_antenna; }
    int lnaState() const { return m_lnaState; }
    bool agc() const { return m_agc; }
    void setPanadapter(bool v);
    void setVfoMHz(double v);
    void setIfMHz(double v);
    void setOffsetKHz(double v);
    void setMirror(bool v);
    void setAntenna(int v);
    void setLnaState(int v);
    void setAgc(bool v);

    void start() override;
    void stop() override;

signals:
    void settingsChanged();
    // aus dem API-Thread, queued in den Hauptthread
    void frameReady(const QVector<float> &psd);
    void deviceEvent(int what);

private slots:
    void onFrame(const QVector<float> &psd);
    void onDeviceEvent(int what);

private:
    static void streamCb(short *xi, short *xq, sdrplay_api_StreamCbParamsT *params,
                         unsigned int numSamples, unsigned int reset, void *ctx);
    static void eventCb(sdrplay_api_EventT id, sdrplay_api_TunerSelectT tuner,
                        sdrplay_api_EventParamsT *params, void *ctx);
    void onSamples(const short *xi, const short *xq, unsigned int n, bool rfChanged);
    bool loadApi();
    void planChunks();
    void tuneTo(double rfMHz);
    void applyGain();
    void applyAntenna();
    void emitStitched();
    double usableMHz() const;

    // aufgelöste API-Funktionen
    struct Api;
    Api *m_api = nullptr;
    void *m_lib = nullptr;

    sdrplay_api_DeviceT m_device;
    bool m_haveDevice = false;
    sdrplay_api_DeviceParamsT *m_params = nullptr;

    bool m_panadapter = true;
    double m_vfoMHz = 14.200;
    double m_ifMHz = 69.450;
    double m_offsetKHz = 0.0;
    bool m_mirror = false;
    int m_antenna = 0;
    int m_lnaState = 4;
    bool m_agc = true;

    // Schrittplan für Bänder breiter als die Empfangsbandbreite
    QVector<double> m_chunkMHz;
    QVector<QVector<float>> m_chunkPsd;
    int m_chunk = 0;

    // Verarbeitung im API-Thread
    QVector<std::complex<float>> m_buf;
    int m_fill = 0;
    int m_frameSkip = 0;
    int m_avgCount = 0;
    QVector<double> m_avg;
    std::atomic<int> m_discard { 0 };
    QVector<float> m_window;
    double m_winPow = 1.0;

    static const int kFft = 2048;
};

#endif
