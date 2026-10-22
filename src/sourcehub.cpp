#include "sourcehub.h"

SourceHub::SourceHub(QObject *parent) : QObject(parent) {}

void SourceHub::add(SpectrumSource *src)
{
    src->setParent(this);
    m_sources.append(src);
}

QStringList SourceHub::names() const
{
    QStringList n;
    for (SpectrumSource *s : m_sources)
        n << s->name();
    return n;
}

void SourceHub::select(int index)
{
    if (index < 0 || index >= m_sources.size() || index == m_index)
        return;
    if (SpectrumSource *old = currentSource())
        if (old->running())
            old->stop();
    m_index = index;
    emit currentChanged();
}

bool SourceHub::selectKind(const QString &kind)
{
    for (int i = 0; i < m_sources.size(); ++i)
        if (m_sources.at(i)->kind() == kind) {
            select(i);
            return true;
        }
    return false;
}
