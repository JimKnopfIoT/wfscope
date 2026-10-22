#ifndef SPECTRUMVIEW_H
#define SPECTRUMVIEW_H

#include <QQuickPaintedItem>
#include <QImage>
#include <QVector>

class SpectrumView : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(float floorDbm READ floorDbm WRITE setFloorDbm NOTIFY rangeChanged)
    Q_PROPERTY(float ceilDbm READ ceilDbm WRITE setCeilDbm NOTIFY rangeChanged)
    Q_PROPERTY(float peakHoldDbm READ peakHoldDbm NOTIFY readoutChanged)
    Q_PROPERTY(float peakHoldMHz READ peakHoldMHz NOTIFY readoutChanged)
    Q_PROPERTY(bool autoRange READ autoRange WRITE setAutoRange NOTIFY rangeChanged)
public:
    explicit SpectrumView(QQuickItem *parent = nullptr);

    float floorDbm() const { return m_floor; }
    float ceilDbm() const { return m_ceil; }
    float peakHoldDbm() const { return m_peakDbm; }
    float peakHoldMHz() const { return m_peakMHz; }
    bool autoRange() const { return m_auto; }
    void setAutoRange(bool v);
    void setFloorDbm(float v);
    void setCeilDbm(float v);

    void paint(QPainter *painter) override;

public slots:
    void pushSweep(const QVector<float> &bins, double centerMHz, double spanMHz);

signals:
    void rangeChanged();
    void readoutChanged();

private:
    void ensureFall(int w);
    void updateAutoRange();
    int colorFor(float dbm) const;

    QVector<float> m_bins;
    double m_center = 0.0;
    double m_span = 0.0;
    QImage m_fall;
    int m_row = 0;
    float m_floor = -100.0f;
    float m_ceil = -30.0f;
    float m_peakDbm = -120.0f;
    float m_peakMHz = 0.0f;
    bool m_auto = true;
    float m_noise = -95.0f;
    bool m_noiseValid = false;
    QVector<QRgb> m_lut;
};

#endif
