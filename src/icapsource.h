#ifndef ICAPSOURCE_H
#define ICAPSOURCE_H

#include "spectrumsource.h"
#include <QProcess>
#include <QByteArray>

// Echte Quelle: steuert den Root-Helfer wfscope-icap (per pkexec), der den
// ICAP-IQ-Capture des connac2x-WLAN-Chips fährt. Ein Band wird in 40/80-MHz-
// Stücke zerlegt; pro Stück ein Capture (32768 IQ-Samples bei 320 MHz),
// gemittelte 1024-FFT, die Stücke werden zum Gesamtspektrum zusammengesetzt.
// Solange die Quelle läuft, ist das normale WLAN aus (Testmodus).
class IcapSource : public SpectrumSource
{
    Q_OBJECT
public:
    explicit IcapSource(QObject *parent = nullptr);
    ~IcapSource() override;

    QString name() const override { return QStringLiteral("WLAN-Chip (ICAP)"); }
    QString kind() const override { return QStringLiteral("icap"); }
    QVariantList bands() const override
    {
        return QVariantList()
        << band(QStringLiteral("2,4 GHz"), 2442, 90)
        << band(QStringLiteral("5 GHz U-NII-1/2"), 5310, 280)
        << band(QStringLiteral("5 GHz U-NII-2C/3"), 5680, 420)
        << band(QStringLiteral("6 GHz low (ungetestet)"), 6025, 320);
    }

    void start() override;
    void stop() override;

private slots:
    void onReadyRead();
    void onFinished(int code, QProcess::ExitStatus st);
    void onErrorOutput();

private:
    struct Chunk {
        int channel;
        int bw;           // 0=20 1=40 2=80
        int band;         // 0=2,4 1=5 2=6 GHz
        double centerMHz;
        double usableMHz;
        QVector<float> psd; // dB, kFft Bins, fftshift-Reihenfolge
    };

    void planChunks();
    void requestNext();
    void handleFrame(quint32 channel, quint16 status, const QByteArray &iq);
    QVector<float> psdFromIq(const QByteArray &iq) const;
    void emitSweep(bool fullCycle);

    QProcess m_proc;
    QByteArray m_buf;
    QVector<Chunk> m_chunks;
    int m_next = 0;
    bool m_stopping = false;
    int m_errors = 0;
    qint64 m_sweepStartMs = 0;

    static const int kFft = 1024;
    static const int kOutBins = 512;
};

#endif
