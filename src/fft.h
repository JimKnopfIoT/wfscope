#ifndef WFSCOPE_FFT_H
#define WFSCOPE_FFT_H

// Kleine, abhängigkeitsfreie Radix-2-FFT und Hann-Fenster für alle Quellen.

#include <QVector>
#include <cmath>
#include <complex>
#include <utility>

namespace wf {

inline void fft(std::complex<float> *x, int n)
{
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(x[i], x[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        const float ang = -2.0f * float(M_PI) / len;
        const std::complex<float> wl(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (int k = 0; k < len / 2; ++k) {
                const std::complex<float> u = x[i + k], v = x[i + k + len / 2] * w;
                x[i + k] = u + v;
                x[i + k + len / 2] = u - v;
                w *= wl;
            }
        }
    }
}

// Hann-Fenster der Länge n; winPow erhält die Summe der Quadrate.
inline QVector<float> hann(int n, double *winPow = nullptr)
{
    QVector<float> w(n);
    double p = 0.0;
    for (int i = 0; i < n; ++i) {
        w[i] = 0.5f - 0.5f * std::cos(2.0f * float(M_PI) * i / (n - 1));
        p += double(w[i]) * w[i];
    }
    if (winPow)
        *winPow = p;
    return w;
}

} // namespace wf

#endif
