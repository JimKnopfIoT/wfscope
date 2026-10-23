#include "fmaudio.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTime>
#include <cmath>
#include <cstdio>
#include <policy/audio-resource.h>
#include <policy/resource-set.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <tinyalsa/asoundlib.h>
#include <cstring>
#include <unistd.h>

namespace {
const unsigned kCard = 0;
const unsigned kCaptureDevice = 15;   // Capture_3 = VUL0 (Pfad CONNSYS_TO_CAPTURE3 der Hersteller-HAL)
const unsigned kChannels = 2;
const unsigned kRate = 48000;         // CONNSYS-ASRC ist für 44,1/48 kHz ausgelegt
const unsigned kPeriodFrames = 1024;
// 8 × 1024 Frames = 32 KB (170 ms). Der Puffer liegt im Audio-SRAM (64 KB gesamt), den sich die
// Aufnahme mit dem Lautsprecherpfad (DL_24CH + Echo-Referenz, 16 KB) teilt; 16 Perioden = 64 KB
// überschneiden sich damit und liefern wiederholte Blöcke („Dataphone“-Klang).
const unsigned kPeriodCount = 8;

// CONNSYS_I2S (FM-Chip) nur auf UL0 legen; UL1 u. a. gehören dem Mikrofonpfad
const char *const kRoutes[] = {
    "UL0_CH1 CONNSYS_I2S_CH1", "UL0_CH2 CONNSYS_I2S_CH2",
};
// Früher zusätzlich gesetzte Routen: beim Start sicherheitshalber lösen
const char *const kStaleRoutes[] = {
    "UL1_CH1 CONNSYS_I2S_CH1", "UL1_CH2 CONNSYS_I2S_CH2",
    "UL5_CH1 CONNSYS_I2S_CH1", "UL5_CH2 CONNSYS_I2S_CH2",
    "UL6_CH1 CONNSYS_I2S_CH1", "UL6_CH2 CONNSYS_I2S_CH2",
    "UL7_CH1 CONNSYS_I2S_CH1", "UL7_CH2 CONNSYS_I2S_CH2",
};
} // namespace

FmAudio::FmAudio(QObject *parent) : QObject(parent)
{
    m_resources = new ResourcePolicy::ResourceSet(QStringLiteral("player"), this);
    m_resources->setAlwaysReply();
    auto *audio = new ResourcePolicy::AudioResource(QStringLiteral("player"));
    audio->setProcessID(QCoreApplication::applicationPid());
    audio->setStreamTag(QStringLiteral("media.name"), QStringLiteral("*"));
    m_resources->addResourceObject(audio);
    connect(m_resources, &ResourcePolicy::ResourceSet::resourcesGranted, this, &FmAudio::onGranted);
    // Anruf, Wecker o. Ä.: Ton anhalten, der Wunsch zu hören bleibt bestehen
    auto pause = [this] {
        m_stop = true;
        if (m_thread.joinable())
            m_thread.join();
        m_active = false;
    };
    connect(m_resources, &ResourcePolicy::ResourceSet::lostResources, this, pause);
    connect(m_resources, &ResourcePolicy::ResourceSet::resourcesReleasedByManager, this, pause);
    // Danach (z. B. Anruf beendet) gibt die Policy die Ressource nur frei, teilt sie aber nicht
    // von selbst wieder zu: neu anfordern, onGranted() startet den Ton wieder
    connect(m_resources, &ResourcePolicy::ResourceSet::resourcesBecameAvailable, this, [this] {
        if (m_wanted && !m_active)
            m_resources->acquire();
    });
}

FmAudio::~FmAudio()
{
    stop();
}

void FmAudio::setVolume(int percent)
{
    m_gain = qBound(0, percent, 100) / 100.0f * 2.0f;
}

void FmAudio::start()
{
    if (m_wanted)
        return;
    m_wanted = true;
    m_resources->acquire();     // Ton startet in onGranted()
}

void FmAudio::onGranted()
{
    if (!m_wanted || m_active)
        return;
    setRoutes(true);
    m_stop = false;
    m_active = true;
    m_thread = std::thread([this] { run(); });
}

void FmAudio::stop()
{
    m_wanted = false;
    m_stop = true;
    if (m_thread.joinable())
        m_thread.join();
    if (m_active)
        setRoutes(false);
    m_active = false;
    m_resources->release();
}

void FmAudio::setRoutes(bool on)
{
    struct mixer *mx = mixer_open(kCard);
    if (!mx) {
        emit error(tr("ALSA mixer not accessible"));
        return;
    }
    for (const char *name : kStaleRoutes)
        if (struct mixer_ctl *ctl = mixer_get_ctl_by_name(mx, name))
            mixer_ctl_set_value(ctl, 0, 0);
    for (const char *name : kRoutes)
        if (struct mixer_ctl *ctl = mixer_get_ctl_by_name(mx, name))
            mixer_ctl_set_value(ctl, 0, on ? 1 : 0);
    mixer_close(mx);
}

// Audio-Thread: ALSA-Capture -> Lautstärke -> PulseAudio
void FmAudio::run()
{
    pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = kRate;
    ss.channels = kChannels;
    pa_buffer_attr attr;
    attr.maxlength = uint32_t(-1);
    attr.tlength = kRate / 4 * kChannels * sizeof(qint16);    // 250 ms; kürzer klingt zerhackt
    attr.prebuf = uint32_t(-1);
    attr.minreq = uint32_t(-1);
    attr.fragsize = uint32_t(-1);
    int err = 0;
    pa_simple *out = pa_simple_new(nullptr, "wfscope", PA_STREAM_PLAYBACK, nullptr, "UKW-Radio",
                                   &ss, nullptr, &attr, &err);
    if (!out) {
        emit error(QStringLiteral("PulseAudio: %1").arg(QString::fromLocal8Bit(pa_strerror(err))));
        return;
    }
    // Erst den Wiedergabepfad mit etwas Stille anlaufen lassen, dann die Aufnahme
    // öffnen: startet der Lautsprecherpfad (SRAM-Belegung) erst nach der Aufnahme,
    // liefert VUL0 immer wieder alte Pufferinhalte („Dataphone“-Klang).
    {
        const QVector<qint16> silence(int(kRate / 10 * kChannels), 0);    // 100 ms
        pa_simple_write(out, silence.constData(), size_t(silence.size()) * sizeof(qint16), &err);
        pa_simple_write(out, silence.constData(), size_t(silence.size()) * sizeof(qint16), &err);
        pa_simple_write(out, silence.constData(), size_t(silence.size()) * sizeof(qint16), &err);
        usleep(200 * 1000);
    }
    struct pcm_config cfg = {};
    cfg.channels = kChannels;
    cfg.rate = kRate;
    cfg.period_size = kPeriodFrames;
    cfg.period_count = kPeriodCount;
    cfg.format = PCM_FORMAT_S16_LE;
    struct pcm *in = pcm_open(kCard, kCaptureDevice, PCM_IN, &cfg);
    if (!in || !pcm_is_ready(in)) {
        emit error(tr("FM capture: %1").arg(QString::fromLocal8Bit(in ? pcm_get_error(in) : "pcm_open")));
        if (in) pcm_close(in);
        pa_simple_free(out);
        return;
    }
    QVector<qint16> buf(int(kPeriodFrames * kChannels));
    const size_t outBytes = size_t(buf.size()) * sizeof(qint16);
    // WFSCOPE_DEBUG: alle 5 s Lesestatistik, längste Blockade, PA-Latenz
    const bool debug = qEnvironmentVariableIsSet("WFSCOPE_DEBUG");
    QElapsedTimer statTimer, op;
    statTimer.start();
    int readErrors = 0, shortReads = 0, reads = 0, fastReads = 0, repeats = 0;
    QVector<qint16> prev(int(kPeriodFrames * kChannels));
    unsigned maxAvail = 0;
    double sumSq = 0;
    long sumN = 0;
    qint64 maxReadMs = 0, maxWriteMs = 0;
    while (!m_stop) {
        const float g = m_gain;
        op.start();
        const int r = pcm_readi(in, buf.data(), kPeriodFrames);
        maxReadMs = qMax(maxReadMs, op.elapsed());
        ++reads;
        if (op.elapsed() < 2)
            ++fastReads;
        {
            // Hardware-Zeiger abgleichen (hwsync). Ohne diesen Aufruf liefert der MTK-Treiber
            // zeitweise alte Puffer erneut aus („zerhackt“, „Dataphone“-Klang).
            unsigned avail = 0;
            struct timespec ts;
            if (pcm_get_htimestamp(in, &avail, &ts) == 0)
                maxAvail = qMax(maxAvail, avail);
        }
        if (r <= 0) {
            ++readErrors;
            continue;
        }
        if (unsigned(r) < kPeriodFrames)
            ++shortReads;
        if (debug) {
            for (qint16 v : buf)
                sumSq += double(v) * v;
            sumN += buf.size();
            if (memcmp(prev.constData(), buf.constData(), outBytes) == 0)
                ++repeats;
            memcpy(prev.data(), buf.constData(), outBytes);
        }
        // Blockierendes Schreiben; tlength (250 ms) begrenzt die Latenz, der Aufnahmepuffer
        // (170 ms) überbrückt die Wartezeit. Nicht verwerfen: nach einem Leerlauf meldet
        // PulseAudio die alte Latenz weiter, und es käme nie wieder Ton.
        for (qint16 &v : buf)
            v = qint16(qBound(-32768.0f, v * g, 32767.0f));
        op.start();
        if (pa_simple_write(out, buf.constData(), outBytes, &err) < 0)
            break;
        maxWriteMs = qMax(maxWriteMs, op.elapsed());
        if (debug && statTimer.elapsed() >= 5000) {
            const pa_usec_t lat = pa_simple_get_latency(out, nullptr);
            fprintf(stderr, "%s FmAudio: Pegel %.0f, %d Lesevorgänge (%d sofort, %d Wiederholungen), Lesefehler %d, kurz %d, max avail %u, max lesen %lld ms, max schreiben %lld ms, PA-Latenz %llu ms\n",
                    qPrintable(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))),
                    sumN ? std::sqrt(sumSq / sumN) : 0.0, reads, fastReads, repeats, readErrors, shortReads, maxAvail, (long long)maxReadMs,
                    (long long)maxWriteMs, (unsigned long long)(lat / 1000));
            readErrors = shortReads = reads = fastReads = repeats = 0;
            maxAvail = 0;
            sumSq = 0;
            sumN = 0;
            maxReadMs = maxWriteMs = 0;
            statTimer.restart();
        }
    }
    pa_simple_drain(out, nullptr);
    pa_simple_free(out);
    pcm_close(in);
}
