#ifndef SOURCEHUB_H
#define SOURCEHUB_H

#include "spectrumsource.h"
#include <QStringList>

// Hält alle Quellen und schaltet zwischen ihnen um. Es läuft immer höchstens
// eine Quelle; beim Umschalten wird die alte gestoppt.
class SourceHub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *current READ current NOTIFY currentChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentChanged)
    Q_PROPERTY(QStringList names READ names CONSTANT)
public:
    explicit SourceHub(QObject *parent = nullptr);

    void add(SpectrumSource *src);
    QObject *current() const { return m_sources.value(m_index); }
    SpectrumSource *currentSource() const { return m_sources.value(m_index); }
    int currentIndex() const { return m_index; }
    QStringList names() const;

    Q_INVOKABLE void select(int index);
    // Wählt die Quelle mit dieser Kennung (kind), z. B. "icap"; false, wenn unbekannt
    bool selectKind(const QString &kind);

signals:
    void currentChanged();

private:
    QVector<SpectrumSource *> m_sources;
    int m_index = 0;
};

#endif
