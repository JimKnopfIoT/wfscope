#ifndef FMAUDIO_H
#define FMAUDIO_H

#include <QObject>
#include <atomic>
#include <thread>

namespace ResourcePolicy { class ResourceSet; }

// Ton des eingebauten UKW-Chips: der FM-Chip liefert sein Audio per I2S
// (CONNSYS_I2S, 32 kHz) an das Audio-Subsystem; dessen ASRC setzt es auf die
// Aufnahmerate um. Wir legen es per ALSA-Mixer auf UL0, lesen es vom
// Capture-PCM (Capture_3) und spielen es über PulseAudio ab.
// Die Audio-Ressource (Sailfish-Policy) wird angefordert, sonst setzt die
// Policy den Stream auf 0 %. Läuft ohne root (Gruppe audio).
class FmAudio : public QObject
{
    Q_OBJECT
public:
    explicit FmAudio(QObject *parent = nullptr);
    ~FmAudio() override;

    void start();
    void stop();
    bool active() const { return m_active; }
    void setVolume(int percent);    // 0…100 → Verstärkung 0…2×

signals:
    void error(const QString &message);

private:
    void setRoutes(bool on);
    void run();
    void onGranted();

    ResourcePolicy::ResourceSet *m_resources = nullptr;
    std::thread m_thread;
    std::atomic<bool> m_stop { false };
    std::atomic<float> m_gain { 1.0f };
    bool m_active = false;
    bool m_wanted = false;
};

#endif
