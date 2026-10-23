#ifndef SPECTRUMSOURCE_H
#define SPECTRUMSOURCE_H

#include <QObject>
#include <QVector>
#include <QVariantList>

class SpectrumSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double centerMHz READ centerMHz WRITE setCenterMHz NOTIFY planChanged)
    Q_PROPERTY(double spanMHz READ spanMHz WRITE setSpanMHz NOTIFY planChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString kind READ kind CONSTANT)
    Q_PROPERTY(QVariantList bands READ bands CONSTANT)
    Q_PROPERTY(double minMHz READ minMHz CONSTANT)
    Q_PROPERTY(double maxMHz READ maxMHz CONSTANT)
public:
    explicit SpectrumSource(QObject *parent = nullptr);

    double centerMHz() const { return m_centerMHz; }
    double spanMHz() const { return m_spanMHz; }
    bool running() const { return m_running; }
    QString status() const { return m_status; }
    virtual QString name() const = 0;
    virtual QString kind() const = 0;
    // Liste von {name, center, span} in MHz für die Bandauswahl
    virtual QVariantList bands() const = 0;
    // gültiger Frequenzbereich der Quelle (MHz), für die manuelle Eingabe
    virtual double minMHz() const { return 0.0; }
    virtual double maxMHz() const { return 100000.0; }

    void setCenterMHz(double v);
    void setSpanMHz(double v);

    Q_INVOKABLE virtual void start() = 0;
    Q_INVOKABLE virtual void stop() = 0;

signals:
    void sweep(const QVector<float> &bins, double centerMHz, double spanMHz);
    void planChanged();
    void runningChanged();
    void statusChanged();

protected:
    static QVariantMap band(const QString &name, double centerMHz, double spanMHz);
    void setRunning(bool r);
    void setStatus(const QString &s);

    double m_centerMHz = 2442.0;
    double m_spanMHz = 100.0;
    bool m_running = false;
    QString m_status;
};

#endif
