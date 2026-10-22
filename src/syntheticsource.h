#ifndef SYNTHETICSOURCE_H
#define SYNTHETICSOURCE_H

#include "spectrumsource.h"
#include <QTimer>

class SyntheticSource : public SpectrumSource
{
    Q_OBJECT
public:
    explicit SyntheticSource(QObject *parent = nullptr);

    QString name() const override { return QStringLiteral("Synthetisch (Demo)"); }
    QString kind() const override { return QStringLiteral("synthetic"); }
    QVariantList bands() const override
    {
        return QVariantList()
        << band(QStringLiteral("2,4 GHz"), 2442, 90)
        << band(QStringLiteral("5 GHz U-NII-1/2"), 5310, 280);
    }

    void start() override;
    void stop() override;

private slots:
    void tick();

private:
    QTimer m_timer;
    int m_frame = 0;
    static const int kBins = 256;
};

#endif
