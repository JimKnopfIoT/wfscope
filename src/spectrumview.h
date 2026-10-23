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
    // Intensität der Farbskala 0…100 (gespeichert): höher = schwache Signale kräftiger
    Q_PROPERTY(int intensity READ intensity WRITE setIntensity NOTIFY rangeChanged)
    // Magnetisches Einrasten auf Spitzen (gespeichert)
    Q_PROPERTY(bool snap READ snap WRITE setSnap NOTIFY snapChanged)
    // Gesten-Vorschau: Verschiebung (Anteil der Breite, + = Inhalt nach rechts) und Zoom
    Q_PROPERTY(qreal viewOffset READ viewOffset WRITE setViewOffset NOTIFY viewChanged)
    Q_PROPERTY(qreal viewScale READ viewScale WRITE setViewScale NOTIFY viewChanged)
public:
    explicit SpectrumView(QQuickItem *parent = nullptr);

    float floorDbm() const { return m_floor; }
    float ceilDbm() const { return m_ceil; }
    float peakHoldDbm() const { return m_peakDbm; }
    float peakHoldMHz() const { return m_peakMHz; }
    bool autoRange() const { return m_auto; }
    void setAutoRange(bool v);
    int intensity() const { return m_intensity; }
    void setIntensity(int v);
    bool snap() const { return m_snap; }
    void setSnap(bool v);
    qreal viewOffset() const { return m_viewOffset; }
    qreal viewScale() const { return m_viewScale; }
    void setViewOffset(qreal v);
    void setViewScale(qreal v);
    void setFloorDbm(float v);
    void setCeilDbm(float v);

    void paint(QPainter *painter) override;

    // Magnetisches Abstimmen: Frequenz der stärksten Spitze im Umkreis (deutlich über dem
    // Rauschboden), sonst NaN
    Q_INVOKABLE double peakNear(double mhz, double radiusMHz) const;

public slots:
    void pushSweep(const QVector<float> &bins, double centerMHz, double spanMHz);
    // Geste beendet: Vorschau stehen lassen, bis Daten im neuen Bereich ankommen
    void commitView();

signals:
    void rangeChanged();
    void readoutChanged();
    void viewChanged();
    void snapChanged();

private:
    void ensureFall(int w);
    void remapFall(int newWidth, double newCenter, double newSpan);
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
    int m_intensity = 50;
    bool m_snap = true;
    qreal m_viewOffset = 0.0;
    qreal m_viewScale = 1.0;
    bool m_commitPending = false;
    double m_commitFromCenter = 0.0, m_commitFromSpan = 0.0;
    float m_noise = -95.0f;
    bool m_noiseValid = false;
    QVector<QRgb> m_lut;
};

#endif
