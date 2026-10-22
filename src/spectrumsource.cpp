#include "spectrumsource.h"

SpectrumSource::SpectrumSource(QObject *parent) : QObject(parent) {}

void SpectrumSource::setCenterMHz(double v)
{
    if (qFuzzyCompare(m_centerMHz, v)) return;
    m_centerMHz = v;
    emit planChanged();
}

void SpectrumSource::setSpanMHz(double v)
{
    if (qFuzzyCompare(m_spanMHz, v)) return;
    m_spanMHz = v;
    emit planChanged();
}

void SpectrumSource::setRunning(bool r)
{
    if (m_running == r) return;
    m_running = r;
    emit runningChanged();
}

void SpectrumSource::setStatus(const QString &s)
{
    if (m_status == s) return;
    m_status = s;
    // Entwicklerhilfe: Status bei Tests per SSH mitprotokollieren
    if (qEnvironmentVariableIsSet("WFSCOPE_DEBUG"))
        qInfo("[%s] %s", qPrintable(kind()), qPrintable(s));
    emit statusChanged();
}

QVariantMap SpectrumSource::band(const QString &name, double centerMHz, double spanMHz)
{
    QVariantMap m;
    m.insert(QStringLiteral("name"), name);
    m.insert(QStringLiteral("center"), centerMHz);
    m.insert(QStringLiteral("span"), spanMHz);
    return m;
}
