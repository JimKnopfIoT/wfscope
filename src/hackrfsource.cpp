#include "hackrfsource.h"
#include "fft.h"

#include <QtMath>

namespace {
const int kSampleRateHz = 20000000;     // wie hackrf_sweep
const int kTuneStepMHz = kSampleRateHz / 1000000;
const int kOffsetHz = 7500000;
const int kBasebandHz = 15000000;
} // namespace

HackRfSource::HackRfSource(QObject *parent) : SpectrumSource(parent)
{
    m_window = wf::hann(kFft);
    connect(this, &HackRfSource::sweepReady, this,
            [this](const QVector<float> &b, double c, double s) { emit sweep(b, c, s); },
            Qt::QueuedConnection);
    connect(this, &SpectrumSource::planChanged, this, [this] {
        if (m_dev) {
            hackrf_stop_rx(m_dev);
            startSweep();
        }
    });
    setStatus(tr("HackRF ready — connect the device via USB OTG"));
}

HackRfSource::~HackRfSource()
{
    stop();
}

void HackRfSource::setLnaGain(int v)
{
    v = qBound(0, v / 8 * 8, 40);
    if (v == m_lna) return;
    m_lna = v;
    applyGains();
    emit gainChanged();
}

void HackRfSource::setVgaGain(int v)
{
    v = qBound(0, v / 2 * 2, 62);
    if (v == m_vga) return;
    m_vga = v;
    applyGains();
    emit gainChanged();
}

void HackRfSource::setAmpEnable(bool v)
{
    if (v == m_amp) return;
    m_amp = v;
    applyGains();
    emit gainChanged();
}

void HackRfSource::setSerial(const QString &s)
{
    if (s == m_serial) return;
    m_serial = s;
    emit serialChanged();
}

void HackRfSource::applyGains()
{
    if (!m_dev) return;
    hackrf_set_lna_gain(m_dev, uint32_t(m_lna));
    hackrf_set_vga_gain(m_dev, uint32_t(m_vga));
    hackrf_set_amp_enable(m_dev, m_amp ? 1 : 0);
}

void HackRfSource::start()
{
    if (m_dev) return;
    if (!m_libInit) {
        const int r = hackrf_init();
        if (r != HACKRF_SUCCESS) {
            setStatus(QStringLiteral("libhackrf: %1").arg(QLatin1String(hackrf_error_name(hackrf_error(r)))));
            return;
        }
        m_libInit = true;
    }
    const QByteArray ser = m_serial.toLatin1();
    int r = hackrf_open_by_serial(ser.isEmpty() ? nullptr : ser.constData(), &m_dev);
    if (r != HACKRF_SUCCESS) {
        m_dev = nullptr;
        if (r == HACKRF_ERROR_NOT_FOUND)
            setStatus(tr("No HackRF found (OTG cable, power?)"));
        else if (r == HACKRF_ERROR_LIBUSB)
            setStatus(tr("HackRF: no USB access (udev rule missing? try as root)"));
        else
            setStatus(tr("Opening HackRF: %1").arg(QLatin1String(hackrf_error_name(hackrf_error(r)))));
        return;
    }
    hackrf_set_sample_rate_manual(m_dev, kSampleRateHz, 1);
    hackrf_set_baseband_filter_bandwidth(m_dev, kBasebandHz);
    applyGains();
    if (!startSweep()) {
        hackrf_close(m_dev);
        m_dev = nullptr;
        return;
    }
    setRunning(true);
}

void HackRfSource::stop()
{
    if (m_dev) {
        hackrf_stop_rx(m_dev);
        hackrf_close(m_dev);
        m_dev = nullptr;
        setStatus(tr("HackRF stopped"));
    }
    if (m_libInit) {
        hackrf_exit();
        m_libInit = false;
    }
    setRunning(false);
}

bool HackRfSource::startSweep()
{
    const double lo = qMax(1.0, m_centerMHz - m_spanMHz / 2.0);
    const double hi = qMin(6000.0, m_centerMHz + m_spanMHz / 2.0);
    quint16 fMin = quint16(qFloor(lo));
    quint16 fMax = quint16(qCeil(hi));
    // wie hackrf_sweep: Bereich auf ganze 20-MHz-Schritte aufrunden
    const int steps = 1 + (fMax - fMin - 1) / kTuneStepMHz;
    fMax = quint16(fMin + steps * kTuneStepMHz);
    {
        QMutexLocker lock(&m_mutex);
        m_fMinMHz = fMin;
        m_fMaxMHz = fMax;
        m_loMHz = lo;
        m_spanMHz = hi - lo;
        m_acc = QVector<float>(kOutBins, -200.0f);
        m_started = false;
        m_emitTimer.start();
    }
    const uint16_t freqs[2] = { fMin, fMax };
    int r = hackrf_init_sweep(m_dev, freqs, 1, BYTES_PER_BLOCK, uint32_t(kTuneStepMHz) * 1000000u,
                              uint32_t(kOffsetHz), INTERLEAVED);
    if (r == HACKRF_SUCCESS)
        r = hackrf_start_rx_sweep(m_dev, &HackRfSource::rxCallback, this);
    if (r != HACKRF_SUCCESS) {
        setStatus(tr("HackRF sweep: %1").arg(QLatin1String(hackrf_error_name(hackrf_error(r)))));
        return false;
    }
    setStatus(tr("HackRF sweeping %1–%2 MHz · LNA %3 · VGA %4%5")
                  .arg(fMin).arg(fMax).arg(m_lna).arg(m_vga)
                  .arg(m_amp ? QStringLiteral(" · Amp") : QString()));
    return true;
}

int HackRfSource::rxCallback(hackrf_transfer *transfer)
{
    auto *self = static_cast<HackRfSource *>(transfer->rx_ctx);
    self->processTransfer(transfer->buffer, transfer->valid_length);
    return 0;
}

void HackRfSource::processTransfer(const quint8 *buf, int len)
{
    for (int off = 0; off + BYTES_PER_BLOCK <= len; off += BYTES_PER_BLOCK) {
        const quint8 *b = buf + off;
        if (b[0] != 0x7F || b[1] != 0x7F)
            continue;
        quint64 f = 0;
        for (int i = 9; i >= 2; --i)
            f = (f << 8) | b[i];
        processBlock(reinterpret_cast<const qint8 *>(b), f);
    }
}

void HackRfSource::processBlock(const qint8 *block, quint64 freqHz)
{
    QMutexLocker lock(&m_mutex);
    if (freqHz == quint64(m_fMinMHz) * 1000000ull) {
        // Sweep vollständig: höchstens ~15 Zeilen/s ausgeben, sonst weiter Max-Hold
        if (m_started && m_emitTimer.elapsed() >= 66) {
            QVector<float> out = m_acc;
            for (float &v : out)
                if (v < -190.0f) v = -120.0f;
            emit sweepReady(out, m_loMHz + m_spanMHz / 2.0, m_spanMHz);
            m_acc.fill(-200.0f);
            m_emitTimer.restart();
        }
        m_started = true;
    }
    if (!m_started)
        return;

    // wie hackrf_sweep: die letzten kFft Samples des Blocks auswerten
    const qint8 *s = block + BYTES_PER_BLOCK - kFft * 2;
    std::complex<float> x[kFft];
    for (int i = 0; i < kFft; ++i)
        x[i] = std::complex<float>(s[2 * i] / 128.0f * m_window[i], s[2 * i + 1] / 128.0f * m_window[i]);
    wf::fft(x, kFft);
    float pwr[kFft];
    const float scale = 1.0f / kFft;
    for (int i = 0; i < kFft; ++i)
        pwr[i] = 10.0f * std::log10(std::norm(x[i]) * scale * scale + 1e-20f);

    const double binMHz = kSampleRateHz / 1e6 / kFft;
    const double f0 = freqHz / 1e6;
    auto put = [&](double fMHz, float db) {
        const int k = int((fMHz - m_loMHz) / m_spanMHz * kOutBins);
        if (k >= 0 && k < kOutBins && db > m_acc[k])
            m_acc[k] = db;
    };
    // Segment [f, f + fs/4] aus Bins 1+5N/8 …, Segment [f + fs/2, f + 3fs/4] aus Bins 1+N/8 …
    for (int i = 0; i < kFft / 4; ++i) {
        put(f0 + i * binMHz, pwr[1 + (kFft * 5) / 8 + i]);
        put(f0 + kSampleRateHz / 2e6 + i * binMHz, pwr[1 + kFft / 8 + i]);
    }
}
