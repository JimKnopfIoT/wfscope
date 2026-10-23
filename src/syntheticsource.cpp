#include "syntheticsource.h"
#include <QtMath>

SyntheticSource::SyntheticSource(QObject *parent) : SpectrumSource(parent)
{
    m_timer.setInterval(60);
    connect(&m_timer, &QTimer::timeout, this, &SyntheticSource::tick);
    setStatus(tr("synthetic"));
}

void SyntheticSource::start()
{
    m_timer.start();
    setRunning(true);
}

void SyntheticSource::stop()
{
    m_timer.stop();
    setRunning(false);
}

static float peak(float x, float c, float w, float h)
{
    const float d = (x - c) / w;
    return h * qExp(-d * d);
}

void SyntheticSource::tick()
{
    QVector<float> bins(kBins);

    const float lo = m_centerMHz - m_spanMHz / 2.0f;
    for (int i = 0; i < kBins; ++i) {
        const float f = lo + m_spanMHz * i / float(kBins - 1);
        float p = -96.0f + 5.0f * qrand() / float(RAND_MAX) - 2.5f;

        if (m_centerMHz < 3000.0) {
            p = qMax(p, -96.0f + peak(f, 2412.0f, 9.0f, 38.0f));
            p = qMax(p, -96.0f + peak(f, 2437.0f, 9.0f, 30.0f));
            p = qMax(p, -96.0f + peak(f, 2462.0f, 9.0f, 44.0f));
            const float hop = 2402.0f + ((m_frame * 7) % 79);
            p = qMax(p, -96.0f + peak(f, hop, 0.6f, 34.0f));
        } else {
            p = qMax(p, -96.0f + peak(f, 5180.0f, 18.0f, 40.0f));
            p = qMax(p, -96.0f + peak(f, 5500.0f, 36.0f, 28.0f));
        }
        bins[i] = p;
    }

    ++m_frame;
    emit sweep(bins, m_centerMHz, m_spanMHz);
}
