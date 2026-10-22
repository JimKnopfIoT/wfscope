#include "spectrumview.h"
#include <QPainter>
#include <QFontMetricsF>
#include <cmath>
#include <QtMath>
#include <algorithm>

SpectrumView::SpectrumView(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setFlag(ItemHasContents, true);
    m_lut.resize(256);
    for (int i = 0; i < 256; ++i) {
        const float t = i / 255.0f;
        int r, g, b;
        if (t < 0.5f) { r = 0; g = int(t * 2 * 255); b = int((1 - t * 2) * 180 + 40); }
        else { r = int((t - 0.5f) * 2 * 255); g = 255; b = 0; }
        m_lut[i] = qRgb(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255));
    }
}

void SpectrumView::setFloorDbm(float v) { if (!qFuzzyCompare(m_floor, v)) { m_floor = v; emit rangeChanged(); update(); } }
void SpectrumView::setCeilDbm(float v) { if (!qFuzzyCompare(m_ceil, v)) { m_ceil = v; emit rangeChanged(); update(); } }

void SpectrumView::setAutoRange(bool v) { if (m_auto != v) { m_auto = v; emit rangeChanged(); update(); } }

// Farbskala am Rauschboden ausrichten: 20-%-Perzentil der Bins (robust auch,
// wenn das halbe Band belegt ist), geglättet.
void SpectrumView::updateAutoRange()
{
    QVector<float> v = m_bins;
    const int k = v.size() / 5;
    std::nth_element(v.begin(), v.begin() + k, v.end());
    const float med = v[k];
    m_noise = m_noiseValid ? 0.8f * m_noise + 0.2f * med : med;
    m_noiseValid = true;
    const float fl = m_noise - 5.0f, ce = m_noise + 35.0f;
    if (!qFuzzyCompare(fl, m_floor) || !qFuzzyCompare(ce, m_ceil)) {
        m_floor = fl;
        m_ceil = ce;
        emit rangeChanged();
    }
}

void SpectrumView::ensureFall(int w)
{
    const int h = 200;
    if (m_fall.width() != w || m_fall.isNull()) {
        m_fall = QImage(w, h, QImage::Format_RGB32);
        m_fall.fill(Qt::black);
        m_row = 0;
    }
}

int SpectrumView::colorFor(float dbm) const
{
    const float t = (dbm - m_floor) / qMax(1.0f, m_ceil - m_floor);
    return qBound(0, int(t * 255.0f), 255);
}

void SpectrumView::pushSweep(const QVector<float> &bins, double centerMHz, double spanMHz)
{
    // Bandwechsel: alter Wasserfall passt nicht mehr zur Frequenzachse
    if (!qFuzzyCompare(m_center, centerMHz) || !qFuzzyCompare(m_span, spanMHz)) {
        m_fall = QImage();
        m_noiseValid = false;
    }
    m_bins = bins;
    m_center = centerMHz;
    m_span = spanMHz;

    const int w = bins.size();
    if (w > 0) {
        if (m_auto)
            updateAutoRange();
        ensureFall(w);
        // Ringpuffer rückwärts: m_row ist die jüngste Zeile
        m_row = (m_row + m_fall.height() - 1) % m_fall.height();
        QRgb *line = reinterpret_cast<QRgb *>(m_fall.scanLine(m_row));
        m_peakDbm = -200.0f;
        for (int i = 0; i < w; ++i) {
            line[i] = m_lut[colorFor(bins[i])];
            if (bins[i] > m_peakDbm) {
                m_peakDbm = bins[i];
                m_peakMHz = centerMHz - spanMHz / 2.0 + spanMHz * i / double(w - 1);
            }
        }
        emit readoutChanged();
    }
    update();
}

// Teilstrichabstand so wählen, dass etwa 4–9 Beschriftungen entstehen.
static double tickStep(double span)
{
    static const double steps[] = { 1, 2, 5, 10, 20, 25, 50, 100, 200, 500 };
    for (double st : steps)
        if (span / st <= 9.0)
            return st;
    return 1000.0;
}

void SpectrumView::paint(QPainter *p)
{
    const qreal w = width();
    const qreal h = height();
    p->fillRect(0, 0, w, h, QColor(8, 10, 14));
    if (m_bins.isEmpty()) return;

    QFont font = p->font();
    font.setPixelSize(qMax(12, int(w / 36)));
    p->setFont(font);
    const qreal fontH = QFontMetricsF(font).height();

    const qreal traceH = h * 0.30;
    const qreal axisH = fontH * 1.5;
    const qreal fallTop = traceH + axisH;
    const qreal fallH = h - fallTop;
    const double lo = m_center - m_span / 2.0;
    auto xOf = [&](double mhz) { return (mhz - lo) / m_span * w; };

    // Gitter: dB horizontal, MHz vertikal
    const QColor grid(30, 40, 52);
    p->setPen(grid);
    for (int g = 1; g < 5; ++g)
        p->drawLine(QPointF(0, traceH * g / 5.0), QPointF(w, traceH * g / 5.0));
    const double step = tickStep(m_span);
    const double first = std::ceil(lo / step) * step;
    for (double f = first; f <= lo + m_span + 1e-6; f += step)
        p->drawLine(QPointF(xOf(f), 0), QPointF(xOf(f), traceH));

    QPainterPath path;
    const int n = m_bins.size();
    for (int i = 0; i < n; ++i) {
        const qreal x = w * i / qreal(n - 1);
        const float t = (m_bins[i] - m_floor) / qMax(1.0f, m_ceil - m_floor);
        const qreal y = traceH * (1.0 - qBound(0.0f, t, 1.0f));
        if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
    }
    QPainterPath fill = path;
    fill.lineTo(w, traceH);
    fill.lineTo(0, traceH);
    fill.closeSubpath();
    p->fillPath(fill, QColor(60, 200, 120, 40));
    p->setPen(QPen(QColor(90, 230, 150), 1.4));
    p->drawPath(path);

    // dB-Beschriftung an den Gitterlinien
    p->setPen(QColor(140, 160, 180));
    for (int g = 1; g < 5; ++g) {
        const qreal y = traceH * g / 5.0;
        const float db = m_ceil - (m_ceil - m_floor) * g / 5.0f;
        p->drawText(QRectF(4, y - fontH, w / 3, fontH), Qt::AlignLeft | Qt::AlignBottom,
                    QString::number(qRound(db)) + QStringLiteral(" dBm"));
    }

    // Frequenzachse zwischen Spektrum und Wasserfall
    p->fillRect(QRectF(0, traceH, w, axisH), QColor(16, 20, 28));
    p->setPen(QColor(200, 215, 230));
    for (double f = first; f <= lo + m_span + 1e-6; f += step) {
        const qreal x = xOf(f);
        p->drawLine(QPointF(x, traceH), QPointF(x, traceH + axisH * 0.25));
        const QString label = QString::number(f, 'f', 0);
        const qreal tw = QFontMetricsF(font).width(label);
        const qreal tx = qBound(qreal(2), x - tw / 2, w - tw - 2);
        p->drawText(QRectF(tx, traceH + axisH * 0.2, tw + 2, axisH * 0.8),
                    Qt::AlignLeft | Qt::AlignVCenter, label);
    }

    if (!m_fall.isNull()) {
        // jüngste Zeile (m_row) oben, ältere darunter
        const int fh = m_fall.height();
        const qreal scale = fallH / fh;
        const int firstRows = fh - m_row;
        p->drawImage(QRectF(0, fallTop, w, firstRows * scale), m_fall,
                     QRect(0, m_row, m_fall.width(), firstRows));
        if (m_row > 0)
            p->drawImage(QRectF(0, fallTop + firstRows * scale, w, m_row * scale), m_fall,
                         QRect(0, 0, m_fall.width(), m_row));
        // dezente MHz-Linien auch im Wasserfall
        p->setPen(QColor(255, 255, 255, 40));
        for (double f = first; f <= lo + m_span + 1e-6; f += step)
            p->drawLine(QPointF(xOf(f), fallTop), QPointF(xOf(f), h));
    }
}
