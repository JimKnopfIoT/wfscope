#ifndef HACKRFSOURCE_H
#define HACKRFSOURCE_H

#include "spectrumsource.h"
#include <QElapsedTimer>
#include <QMutex>
#include <hackrf.h>

// HackRF One / PortaPack (im HackRF-Modus) über libhackrf im Sweep-Modus:
// die Firmware stimmt in 20-MHz-Schritten durch den Bereich, pro Schritt
// eine 256er-FFT (78 kHz/Bin), Auswertung wie hackrf_sweep (INTERLEAVED).
// Das WLAN des Telefons bleibt dabei an. Braucht USB-Zugriff auf das Gerät.
class HackRfSource : public SpectrumSource
{
    Q_OBJECT
    Q_PROPERTY(int lnaGain READ lnaGain WRITE setLnaGain NOTIFY gainChanged)
    Q_PROPERTY(int vgaGain READ vgaGain WRITE setVgaGain NOTIFY gainChanged)
    Q_PROPERTY(bool ampEnable READ ampEnable WRITE setAmpEnable NOTIFY gainChanged)
    Q_PROPERTY(QString serial READ serial WRITE setSerial NOTIFY serialChanged)
public:
    explicit HackRfSource(QObject *parent = nullptr);
    ~HackRfSource() override;

    QString name() const override { return QStringLiteral("HackRF / PortaPack"); }
    QString kind() const override { return QStringLiteral("hackrf"); }
    QVariantList bands() const override
    {
        return QVariantList()
            << band(QStringLiteral("2,4 GHz WLAN"), 2450, 100)
            << band(QStringLiteral("5 GHz WLAN"), 5525, 750)
            << band(QStringLiteral("2,4 + 5 GHz (2,4–5,9)"), 4150, 3500)
            << band(QStringLiteral("UKW 87,5–108"), 97.75, 20.5)
            << band(QStringLiteral("2 m / 70 cm (140–440)"), 290, 300)
            << band(QStringLiteral("Übersicht 1–6000"), 3000.5, 5999);
    }

    int lnaGain() const { return m_lna; }
    int vgaGain() const { return m_vga; }
    bool ampEnable() const { return m_amp; }
    QString serial() const { return m_serial; }
    void setLnaGain(int v);
    void setVgaGain(int v);
    void setAmpEnable(bool v);
    void setSerial(const QString &s);

    void start() override;
    void stop() override;

signals:
    void gainChanged();
    void serialChanged();
    void sweepReady(const QVector<float> &bins, double centerMHz, double spanMHz);

private:
    static int rxCallback(hackrf_transfer *transfer);
    void processTransfer(const quint8 *buf, int len);
    void processBlock(const qint8 *block, quint64 freqHz);
    bool startSweep();
    void applyGains();

    hackrf_device *m_dev = nullptr;
    bool m_libInit = false;
    int m_lna = 16;
    int m_vga = 20;
    bool m_amp = false;
    QString m_serial;

    // Sweep-Plan (vom Hauptthread gesetzt, im USB-Thread gelesen unter m_mutex)
    QMutex m_mutex;
    quint16 m_fMinMHz = 0, m_fMaxMHz = 0;
    double m_loMHz = 0.0, m_spanMHz = 0.0;
    QVector<float> m_acc;
    bool m_started = false;
    QElapsedTimer m_emitTimer;

    static const int kFft = 256;
    static const int kOutBins = 1024;
    QVector<float> m_window;
};

#endif
