#include "spectrumview.h"
#include <QPainter>
#include <QSettings>
#include <QFontMetricsF>
#include <QTransform>
#include <cmath>
#include <QtMath>
#include <algorithm>

SpectrumView::SpectrumView(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setFlag(ItemHasContents, true);
    m_intensity = qBound(0, QSettings().value(QStringLiteral("view/intensity"), 50).toInt(), 100);
    m_snap = QSettings().value(QStringLiteral("view/snap"), true).toBool();
    // Palette ähnlich Gqrx: Rauschen fast schwarz/dunkelblau, schwache Signale blau → cyan,
    // starke gelb, sehr starke rot
    struct Stop { float t; int r, g, b; };
    static const Stop stops[] = {
        { 0.00f,   0,   0,  10 }, { 0.20f,   0,  10,  70 }, { 0.40f,  20,  60, 200 },
        { 0.60f,   0, 200, 230 }, { 0.80f, 250, 240,  40 }, { 1.00f, 255,  40,  20 },
    };
    m_lut.resize(256);
    for (int i = 0; i < 256; ++i) {
        const float t = i / 255.0f;
        int k = 0;
        while (k < 4 && t > stops[k + 1].t)
            ++k;
        const Stop &a = stops[k], &b = stops[k + 1];
        const float u = (t - a.t) / (b.t - a.t);
        m_lut[i] = qRgb(int(a.r + (b.r - a.r) * u), int(a.g + (b.g - a.g) * u), int(a.b + (b.b - a.b) * u));
    }
}

void SpectrumView::setFloorDbm(float v) { if (!qFuzzyCompare(m_floor, v)) { m_floor = v; emit rangeChanged(); update(); } }
void SpectrumView::setCeilDbm(float v) { if (!qFuzzyCompare(m_ceil, v)) { m_ceil = v; emit rangeChanged(); update(); } }

void SpectrumView::setSnap(bool v)
{
    if (v == m_snap) return;
    m_snap = v;
    QSettings().setValue(QStringLiteral("view/snap"), v);
    emit snapChanged();
}

void SpectrumView::setIntensity(int v)
{
    v = qBound(0, v, 100);
    if (v == m_intensity) return;
    m_intensity = v;
    QSettings().setValue(QStringLiteral("view/intensity"), v);
    if (m_auto && !m_bins.isEmpty())
        updateAutoRange();
    emit rangeChanged();
    update();
}

void SpectrumView::setAutoRange(bool v) { if (m_auto != v) { m_auto = v; emit rangeChanged(); update(); } }
void SpectrumView::setViewOffset(qreal v) { if (!qFuzzyCompare(1.0 + m_viewOffset, 1.0 + v)) { m_viewOffset = v; emit viewChanged(); update(); } }
void SpectrumView::setViewScale(qreal v) { v = qBound(qreal(0.02), v, qreal(50.0)); if (!qFuzzyCompare(m_viewScale, v)) { m_viewScale = v; emit viewChanged(); update(); } }

// Farbskala am Rauschboden ausrichten: 20-%-Perzentil der Bins (robust auch,
// wenn das halbe Band belegt ist), geglättet.
void SpectrumView::updateAutoRange()
{
    // Leere Bins (außerhalb des Empfangsbereichs, ≤ −129 dBm) nicht mitzählen
    QVector<float> v;
    v.reserve(m_bins.size());
    for (float b : m_bins)
        if (b > -129.0f)
            v.append(b);
    if (v.isEmpty())
        return;
    const int k = v.size() / 5;
    std::nth_element(v.begin(), v.begin() + k, v.end());
    const float med = v[k];
    m_noise = m_noiseValid ? 0.8f * m_noise + 0.2f * med : med;
    m_noiseValid = true;
    // Intensität 0…100: Skalenbreite 60 → 15 dB über dem Rauschboden (50 = 35 dB wie bisher)
    const float width = 60.0f - 0.45f * m_intensity;
    const float fl = m_noise - 5.0f, ce = m_noise - 5.0f + width;
    if (!qFuzzyCompare(fl, m_floor) || !qFuzzyCompare(ce, m_ceil)) {
        m_floor = fl;
        m_ceil = ce;
        emit rangeChanged();
    }
}

double SpectrumView::peakNear(double mhz, double radiusMHz) const
{
    const int n = m_bins.size();
    if (n < 2 || m_span <= 0)
        return qQNaN();
    const double lo = m_center - m_span / 2, step = m_span / (n - 1);   // Bin i liegt bei lo + i·step
    const int a = qMax(0, int(std::floor((mhz - radiusMHz - lo) / step)));
    const int b = qMin(n - 1, int(std::ceil((mhz + radiusMHz - lo) / step)));
    int best = -1;
    for (int i = a; i <= b; ++i)
        if (best < 0 || m_bins[i] > m_bins[best])
            best = i;
    if (best < 0 || m_bins[best] < m_noise + 6.0f)
        return qQNaN();
    return lo + best * step;
}

void SpectrumView::commitView()
{
    if (qFuzzyCompare(m_viewScale, 1.0) && qFuzzyCompare(1.0 + m_viewOffset, 1.0))
        return;
    m_commitPending = true;
    m_commitFromCenter = m_center;
    m_commitFromSpan = m_span;
}

void SpectrumView::remapFall(int newWidth, double newCenter, double newSpan)
{
    QImage out(newWidth, m_fall.height(), QImage::Format_RGB32);
    out.fill(Qt::black);
    const int ow = m_fall.width();
    const double oLo = m_center - m_span / 2, nLo = newCenter - newSpan / 2;
    QVector<int> src(newWidth);
    for (int x = 0; x < newWidth; ++x) {
        const double f = nLo + newSpan * (x + 0.5) / newWidth;
        const int ox = int((f - oLo) / m_span * ow);
        src[x] = (ox >= 0 && ox < ow) ? ox : -1;
    }
    for (int y = 0; y < m_fall.height(); ++y) {
        const QRgb *in = reinterpret_cast<const QRgb *>(m_fall.constScanLine(y));
        QRgb *o = reinterpret_cast<QRgb *>(out.scanLine(y));
        for (int x = 0; x < newWidth; ++x)
            if (src[x] >= 0)
                o[x] = in[src[x]];
    }
    m_fall = out;
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
    const bool planChanged = !qFuzzyCompare(m_center, centerMHz) || !qFuzzyCompare(m_span, spanMHz);
    if (planChanged) {
        // Neuer Bereich: Wasserfall-Historie auf die neue Achse umrechnen;
        // bei großen Sprüngen (kaum Überlappung) neu beginnen
        const double ovLo = qMax(m_center - m_span / 2, centerMHz - spanMHz / 2);
        const double ovHi = qMin(m_center + m_span / 2, centerMHz + spanMHz / 2);
        if (!m_fall.isNull() && m_span > 0 && ovHi > ovLo)
            remapFall(bins.size(), centerMHz, spanMHz);
        else
            m_fall = QImage();
        if (!m_commitPending)
            m_noiseValid = false;
    }
    // Vorschau der Geste auflösen, sobald Daten im neuen Bereich da sind
    if (m_commitPending && (!qFuzzyCompare(centerMHz, m_commitFromCenter)
                            || !qFuzzyCompare(spanMHz, m_commitFromSpan))) {
        m_commitPending = false;
        m_viewOffset = 0.0;
        m_viewScale = 1.0;
        emit viewChanged();
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
    static const double steps[] = { 0.001, 0.002, 0.005, 0.01, 0.02, 0.025, 0.05, 0.1, 0.2, 0.25,
                                    0.5, 1, 2, 5, 10, 20, 25, 50, 100, 200, 500 };
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
    // im Querformat nicht mit der Breite wachsen: höchstens so groß wie im Hochformat
    font.setPixelSize(qMax(12, int(qMin(w, h * 1.6) / 36)));
    p->setFont(font);
    const qreal fontH = QFontMetricsF(font).height();

    const qreal traceH = h * 0.30;
    const qreal axisH = fontH * 1.5;
    const qreal fallTop = traceH + axisH;
    const qreal fallH = h - fallTop;
    // Während einer Geste: angezeigter Bereich = verschobene/gezoomte Vorschau
    const qreal vs = m_viewScale, vo = m_viewOffset;
    const double dispSpan = m_span / vs;
    const double dispCenter = m_center - m_span * vo / vs;
    const double lo = dispCenter - dispSpan / 2.0;
    auto xOf = [&](double mhz) { return (mhz - lo) / dispSpan * w; };
    // Kurve und Wasserfall sind in Original-Koordinaten gezeichnet; diese
    // Transformation bildet sie auf die Vorschau ab
    QTransform view;
    view.translate(w / 2 + vo * w, 0);
    view.scale(vs, 1.0);
    view.translate(-w / 2, 0);

    // Gitter: dB horizontal, MHz vertikal
    const QColor grid(30, 40, 52);
    p->setPen(grid);
    for (int g = 1; g < 5; ++g)
        p->drawLine(QPointF(0, traceH * g / 5.0), QPointF(w, traceH * g / 5.0));
    const double step = tickStep(dispSpan);
    const int decimals = qMax(0, int(std::ceil(-std::log10(step) - 1e-9)));
    const double first = std::ceil(lo / step - 1e-9) * step;
    for (double f = first; f <= lo + dispSpan + 1e-9; f += step)
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
    p->save();
    p->setClipRect(QRectF(0, 0, w, traceH));
    p->setTransform(view, true);
    p->fillPath(fill, QColor(60, 200, 120, 40));
    p->setPen(QPen(QColor(90, 230, 150), 1.4 / vs));
    p->drawPath(path);
    p->restore();

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
    for (double f = first; f <= lo + dispSpan + 1e-9; f += step) {
        const qreal x = xOf(f);
        p->drawLine(QPointF(x, traceH), QPointF(x, traceH + axisH * 0.25));
        const QString label = QString::number(f, 'f', decimals);
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
        p->save();
        p->setClipRect(QRectF(0, fallTop, w, fallH));
        p->setTransform(view, true);
        p->drawImage(QRectF(0, fallTop, w, firstRows * scale), m_fall,
                     QRect(0, m_row, m_fall.width(), firstRows));
        if (m_row > 0)
            p->drawImage(QRectF(0, fallTop + firstRows * scale, w, m_row * scale), m_fall,
                         QRect(0, 0, m_fall.width(), m_row));
        p->restore();
        // dezente MHz-Linien auch im Wasserfall
        p->setPen(QColor(255, 255, 255, 40));
        for (double f = first; f <= lo + dispSpan + 1e-9; f += step)
            p->drawLine(QPointF(xOf(f), fallTop), QPointF(xOf(f), h));
    }
}
