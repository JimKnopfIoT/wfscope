#include "sdrplaysource.h"
#include "fft.h"

#include <QtMath>
#include <algorithm>
#include <cstring>
#include <dlfcn.h>
#include <type_traits>

namespace {
const double kFsHz = 8000000.0;                 // Abtastrate
const sdrplay_api_Bw_MHzT kBw = sdrplay_api_BW_8_000;
const double kUsableFrac = 0.9;                 // Filterflanken abschneiden
const int kFftPerSecond = 150;                  // nicht jedes Fenster rechnen (CPU)
const int kAvg = 10;                            // ⇒ ~15 Zeilen/s
} // namespace

struct SdrplaySource::Api {
    decltype(&sdrplay_api_Open) Open;
    decltype(&sdrplay_api_Close) Close;
    decltype(&sdrplay_api_LockDeviceApi) LockDeviceApi;
    decltype(&sdrplay_api_UnlockDeviceApi) UnlockDeviceApi;
    decltype(&sdrplay_api_GetDevices) GetDevices;
    decltype(&sdrplay_api_SelectDevice) SelectDevice;
    decltype(&sdrplay_api_ReleaseDevice) ReleaseDevice;
    decltype(&sdrplay_api_GetDeviceParams) GetDeviceParams;
    decltype(&sdrplay_api_Init) Init;
    decltype(&sdrplay_api_Uninit) Uninit;
    decltype(&sdrplay_api_Update) Update;
    decltype(&sdrplay_api_GetErrorString) GetErrorString;
};

SdrplaySource::SdrplaySource(QObject *parent) : SpectrumSource(parent)
{
    memset(&m_device, 0, sizeof(m_device));
    m_window = wf::hann(kFft, &m_winPow);
    m_buf.resize(kFft);
    m_avg = QVector<double>(kFft, 0.0);
    connect(this, &SdrplaySource::frameReady, this, &SdrplaySource::onFrame, Qt::QueuedConnection);
    connect(this, &SdrplaySource::deviceEvent, this, &SdrplaySource::onDeviceEvent, Qt::QueuedConnection);
    connect(this, &SpectrumSource::planChanged, this, [this] {
        if (m_running && !m_panadapter)
            planChunks();
    });
    setStatus(tr("RSPdx ready — connect the device via USB OTG"));
}

SdrplaySource::~SdrplaySource()
{
    stop();
    if (m_lib)
        dlclose(m_lib);
    delete m_api;
}

bool SdrplaySource::loadApi()
{
    if (m_api)
        return true;
    const char *paths[] = { "libsdrplay_api.so.3", "/usr/local/lib/libsdrplay_api.so.3" };
    for (const char *p : paths)
        if ((m_lib = dlopen(p, RTLD_NOW | RTLD_LOCAL)))
            break;
    if (!m_lib) {
        setStatus(tr("SDRplay API not installed (libsdrplay_api.so.3)"));
        return false;
    }
    Api *a = new Api;
    bool ok = true;
    auto res = [&](auto &fn, const char *sym) {
        fn = reinterpret_cast<typename std::remove_reference<decltype(fn)>::type>(dlsym(m_lib, sym));
        ok = ok && fn;
    };
    res(a->Open, "sdrplay_api_Open");
    res(a->Close, "sdrplay_api_Close");
    res(a->LockDeviceApi, "sdrplay_api_LockDeviceApi");
    res(a->UnlockDeviceApi, "sdrplay_api_UnlockDeviceApi");
    res(a->GetDevices, "sdrplay_api_GetDevices");
    res(a->SelectDevice, "sdrplay_api_SelectDevice");
    res(a->ReleaseDevice, "sdrplay_api_ReleaseDevice");
    res(a->GetDeviceParams, "sdrplay_api_GetDeviceParams");
    res(a->Init, "sdrplay_api_Init");
    res(a->Uninit, "sdrplay_api_Uninit");
    res(a->Update, "sdrplay_api_Update");
    res(a->GetErrorString, "sdrplay_api_GetErrorString");
    if (!ok) {
        delete a;
        dlclose(m_lib);
        m_lib = nullptr;
        setStatus(tr("SDRplay API incomplete (wrong version?)"));
        return false;
    }
    m_api = a;
    return true;
}

void SdrplaySource::start()
{
    if (m_running || !loadApi())
        return;
    sdrplay_api_ErrT err = m_api->Open();
    if (err != sdrplay_api_Success) {
        setStatus(tr("SDRplay service unreachable (is sdrplay_apiService running?)"));
        return;
    }
    m_api->LockDeviceApi();
    sdrplay_api_DeviceT devs[SDRPLAY_MAX_DEVICES];
    unsigned int n = 0;
    m_api->GetDevices(devs, &n, SDRPLAY_MAX_DEVICES);
    int pick = -1;
    for (unsigned int i = 0; i < n; ++i)
        if (devs[i].hwVer == SDRPLAY_RSPdx_ID || devs[i].hwVer == SDRPLAY_RSPdxR2_ID) {
            pick = int(i);
            break;
        }
    if (pick < 0 && n > 0)
        pick = 0;
    if (pick < 0) {
        m_api->UnlockDeviceApi();
        m_api->Close();
        setStatus(tr("No SDRplay device found (OTG cable?)"));
        return;
    }
    m_device = devs[pick];
    err = m_api->SelectDevice(&m_device);
    m_api->UnlockDeviceApi();
    if (err != sdrplay_api_Success) {
        m_api->Close();
        setStatus(QStringLiteral("SDRplay SelectDevice: %1").arg(QLatin1String(m_api->GetErrorString(err))));
        return;
    }
    m_haveDevice = true;
    m_api->GetDeviceParams(m_device.dev, &m_params);
    m_params->devParams->fsFreq.fsHz = kFsHz;
    sdrplay_api_RxChannelParamsT *ch = m_params->rxChannelA;
    ch->tunerParams.bwType = kBw;
    ch->tunerParams.ifType = sdrplay_api_IF_Zero;
    ch->tunerParams.gain.gRdB = 40;
    ch->tunerParams.gain.LNAstate = (unsigned char)m_lnaState;
    ch->ctrlParams.agc.enable = m_agc ? sdrplay_api_AGC_50HZ : sdrplay_api_AGC_DISABLE;
    ch->ctrlParams.dcOffset.DCenable = 1;
    ch->ctrlParams.dcOffset.IQenable = 1;
    if (m_device.hwVer == SDRPLAY_RSPdx_ID || m_device.hwVer == SDRPLAY_RSPdxR2_ID)
        m_params->devParams->rspDxParams.antennaSel = sdrplay_api_RspDx_AntennaSelectT(m_antenna);

    m_fill = 0;
    m_frameSkip = 0;
    m_avgCount = 0;
    m_avg.fill(0.0);
    m_discard = 2;
    if (m_panadapter) {
        m_chunkMHz = QVector<double>() << (m_ifMHz + m_offsetKHz / 1000.0);
        m_chunkPsd = QVector<QVector<float>>(1);
        m_chunk = 0;
        ch->tunerParams.rfFreq.rfHz = m_chunkMHz.first() * 1e6;
    } else {
        planChunks();
        ch->tunerParams.rfFreq.rfHz = m_chunkMHz.value(0, m_centerMHz) * 1e6;
    }

    sdrplay_api_CallbackFnsT cbs;
    cbs.StreamACbFn = &SdrplaySource::streamCb;
    cbs.StreamBCbFn = nullptr;
    cbs.EventCbFn = &SdrplaySource::eventCb;
    err = m_api->Init(m_device.dev, &cbs, this);
    if (err != sdrplay_api_Success) {
        setStatus(QStringLiteral("SDRplay Init: %1").arg(QLatin1String(m_api->GetErrorString(err))));
        m_api->ReleaseDevice(&m_device);
        m_haveDevice = false;
        m_api->Close();
        return;
    }
    setRunning(true);
    setStatus(m_panadapter
              ? tr("RSPdx panadapter · IF %1 MHz · VFO %2 MHz")
                    .arg(m_ifMHz, 0, 'f', 3).arg(m_vfoMHz, 0, 'f', 3)
              : tr("RSPdx receiving · %1 step(s)").arg(m_chunkMHz.size()));
}

void SdrplaySource::stop()
{
    if (m_haveDevice) {
        m_api->Uninit(m_device.dev);
        m_api->ReleaseDevice(&m_device);
        m_api->Close();
        m_haveDevice = false;
        m_params = nullptr;
        setStatus(tr("RSPdx stopped"));
    }
    setRunning(false);
}

double SdrplaySource::usableMHz() const
{
    return kFsHz / 1e6 * kUsableFrac;
}

void SdrplaySource::planChunks()
{
    const double use = usableMHz();
    const int n = qMax(1, int(qCeil(m_spanMHz / use)));
    const double lo = m_centerMHz - m_spanMHz / 2.0;
    m_chunkMHz.clear();
    for (int i = 0; i < n; ++i)
        m_chunkMHz << lo + (i + 0.5) * (m_spanMHz / n);
    m_chunkPsd = QVector<QVector<float>>(n);
    m_chunk = 0;
    if (m_running)
        tuneTo(m_chunkMHz.first());
}

void SdrplaySource::tuneTo(double rfMHz)
{
    if (!m_haveDevice || !m_params)
        return;
    m_discard = 2;
    m_params->rxChannelA->tunerParams.rfFreq.rfHz = rfMHz * 1e6;
    m_api->Update(m_device.dev, sdrplay_api_Tuner_A, sdrplay_api_Update_Tuner_Frf,
                  sdrplay_api_Update_Ext1_None);
}

void SdrplaySource::applyGain()
{
    if (!m_haveDevice || !m_params)
        return;
    sdrplay_api_RxChannelParamsT *ch = m_params->rxChannelA;
    ch->tunerParams.gain.LNAstate = (unsigned char)m_lnaState;
    ch->ctrlParams.agc.enable = m_agc ? sdrplay_api_AGC_50HZ : sdrplay_api_AGC_DISABLE;
    m_api->Update(m_device.dev, sdrplay_api_Tuner_A,
                  sdrplay_api_ReasonForUpdateT(sdrplay_api_Update_Tuner_Gr | sdrplay_api_Update_Ctrl_Agc),
                  sdrplay_api_Update_Ext1_None);
}

void SdrplaySource::applyAntenna()
{
    if (!m_haveDevice || !m_params)
        return;
    if (m_device.hwVer != SDRPLAY_RSPdx_ID && m_device.hwVer != SDRPLAY_RSPdxR2_ID)
        return;
    m_params->devParams->rspDxParams.antennaSel = sdrplay_api_RspDx_AntennaSelectT(m_antenna);
    m_api->Update(m_device.dev, sdrplay_api_Tuner_A, sdrplay_api_Update_None,
                  sdrplay_api_Update_RspDx_AntennaControl);
}

void SdrplaySource::setPanadapter(bool v)
{
    if (v == m_panadapter) return;
    const bool wasRunning = m_running;
    if (wasRunning) stop();
    m_panadapter = v;
    emit settingsChanged();
    if (wasRunning) start();
}

void SdrplaySource::setVfoMHz(double v)
{
    if (qFuzzyCompare(v, m_vfoMHz)) return;
    m_vfoMHz = v;
    emit settingsChanged();
}

void SdrplaySource::setIfMHz(double v)
{
    if (qFuzzyCompare(v, m_ifMHz)) return;
    m_ifMHz = v;
    emit settingsChanged();
    if (m_running && m_panadapter) {
        m_chunkMHz = QVector<double>() << (m_ifMHz + m_offsetKHz / 1000.0);
        tuneTo(m_chunkMHz.first());
    }
}

void SdrplaySource::setOffsetKHz(double v)
{
    if (qFuzzyCompare(v + 1e9, m_offsetKHz + 1e9)) return;
    m_offsetKHz = v;
    emit settingsChanged();
    if (m_running && m_panadapter) {
        m_chunkMHz = QVector<double>() << (m_ifMHz + m_offsetKHz / 1000.0);
        tuneTo(m_chunkMHz.first());
    }
}

void SdrplaySource::setMirror(bool v)
{
    if (v == m_mirror) return;
    m_mirror = v;
    emit settingsChanged();
}

void SdrplaySource::setAntenna(int v)
{
    v = qBound(0, v, 2);
    if (v == m_antenna) return;
    m_antenna = v;
    emit settingsChanged();
    applyAntenna();
}

void SdrplaySource::setLnaState(int v)
{
    v = qBound(0, v, 27);
    if (v == m_lnaState) return;
    m_lnaState = v;
    emit settingsChanged();
    applyGain();
}

void SdrplaySource::setAgc(bool v)
{
    if (v == m_agc) return;
    m_agc = v;
    emit settingsChanged();
    applyGain();
}

void SdrplaySource::streamCb(short *xi, short *xq, sdrplay_api_StreamCbParamsT *params,
                             unsigned int numSamples, unsigned int, void *ctx)
{
    static_cast<SdrplaySource *>(ctx)->onSamples(xi, xq, numSamples, params && params->rfChanged);
}

void SdrplaySource::eventCb(sdrplay_api_EventT id, sdrplay_api_TunerSelectT,
                            sdrplay_api_EventParamsT *, void *ctx)
{
    emit static_cast<SdrplaySource *>(ctx)->deviceEvent(int(id));
}

void SdrplaySource::onDeviceEvent(int what)
{
    if (what == sdrplay_api_PowerOverloadChange && m_haveDevice) {
        // Überlastmeldung muss quittiert werden, sonst kommen keine weiteren
        m_api->Update(m_device.dev, sdrplay_api_Tuner_A, sdrplay_api_Update_Ctrl_OverloadMsgAck,
                      sdrplay_api_Update_Ext1_None);
    } else if (what == sdrplay_api_DeviceRemoved) {
        stop();
        setStatus(tr("RSPdx was unplugged"));
    }
}

// API-Thread: Samples sammeln, gelegentlich FFT, kAvg Fenster mitteln.
void SdrplaySource::onSamples(const short *xi, const short *xq, unsigned int n, bool rfChanged)
{
    if (rfChanged) {
        m_discard = 2;
        m_fill = 0;
        m_avgCount = 0;
        m_avg.fill(0.0);
    }
    const int skip = qMax(1, int(kFsHz / kFft / kFftPerSecond));
    for (unsigned int i = 0; i < n; ++i) {
        m_buf[m_fill++] = std::complex<float>(xi[i] / 32768.0f, xq[i] / 32768.0f);
        if (m_fill < kFft)
            continue;
        m_fill = 0;
        if (++m_frameSkip < skip)
            continue;
        m_frameSkip = 0;
        if (m_discard > 0) {
            --m_discard;
            continue;
        }
        QVector<std::complex<float>> x(kFft);
        for (int k = 0; k < kFft; ++k)
            x[k] = m_buf[k] * m_window[k];
        wf::fft(x.data(), kFft);
        for (int k = 0; k < kFft; ++k)
            m_avg[(k + kFft / 2) % kFft] += std::norm(x[k]);
        if (++m_avgCount < kAvg)
            continue;
        QVector<float> psd(kFft);
        const double norm = double(m_avgCount) * kFft * m_winPow;
        for (int k = 0; k < kFft; ++k)
            psd[k] = 10.0f * std::log10(float(m_avg[k] / norm) + 1e-20f);
        // DC-Rest der Zero-IF-Mischung glätten
        psd[kFft / 2] = 0.5f * (psd[kFft / 2 - 3] + psd[kFft / 2 + 3]);
        for (int d = 1; d < 3; ++d)
            psd[kFft / 2 - d] = psd[kFft / 2 + d] = psd[kFft / 2];
        m_avg.fill(0.0);
        m_avgCount = 0;
        emit frameReady(psd);
    }
}

// Hauptthread: Frame einsortieren, ggf. nächsten Schritt abstimmen, Zeile ausgeben.
void SdrplaySource::onFrame(const QVector<float> &psd)
{
    if (!m_running)
        return;
    const int keep = int(kFft * kUsableFrac);
    const int first = (kFft - keep) / 2;
    QVector<float> cut = psd.mid(first, keep);

    if (m_panadapter) {
        if (m_mirror)
            std::reverse(cut.begin(), cut.end());
        emit sweep(cut, m_vfoMHz, usableMHz());
        return;
    }
    if (m_chunkMHz.size() <= 1) {
        // gezoomt: auf die gewünschte Spanne um die Tunerfrequenz zuschneiden
        if (m_spanMHz < usableMHz()) {
            const int k = qMax(16, int(cut.size() * m_spanMHz / usableMHz()));
            cut = cut.mid((cut.size() - k) / 2, k);
            emit sweep(cut, m_chunkMHz.value(0, m_centerMHz), m_spanMHz);
        } else {
            emit sweep(cut, m_chunkMHz.value(0, m_centerMHz), usableMHz());
        }
        return;
    }
    m_chunkPsd[m_chunk] = cut;
    if (++m_chunk >= m_chunkMHz.size()) {
        m_chunk = 0;
        emitStitched();
    }
    tuneTo(m_chunkMHz.at(m_chunk));
}

void SdrplaySource::emitStitched()
{
    const int out = 1024;
    QVector<float> bins(out, -120.0f);
    const double lo = m_centerMHz - m_spanMHz / 2.0;
    const double use = usableMHz();
    for (int i = 0; i < out; ++i) {
        const double f = lo + m_spanMHz * (i + 0.5) / out;
        for (int c = 0; c < m_chunkMHz.size(); ++c) {
            const QVector<float> &p = m_chunkPsd.at(c);
            const double off = f - m_chunkMHz.at(c);
            if (p.isEmpty() || qAbs(off) > use / 2.0)
                continue;
            const int k = qBound(0, int((off / use + 0.5) * p.size()), p.size() - 1);
            bins[i] = p[k];
            break;
        }
    }
    emit sweep(bins, m_centerMHz, m_spanMHz);
}
