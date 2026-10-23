# Third-party notices

## libhackrf (bundled in `3rdparty/libhackrf/`)

From HackRF 2026.01.3 — https://github.com/greatscottgadgets/hackrf —
`host/libhackrf/src/hackrf.{c,h}`, unmodified. BSD-3-Clause; the full notice is
in `3rdparty/libhackrf/LICENSE`.

## tinyalsa (bundled in `3rdparty/tinyalsa/`)

tinyalsa 2.0.0 — https://github.com/tinyalsa/tinyalsa — `src/` and `include/`,
unmodified. BSD-3-Clause (Copyright 2011, The Android Open Source Project); the
full notice is in `3rdparty/tinyalsa/LICENSE`. Used for the FM radio audio
(ALSA mixer and capture).

## SDRplay API (not bundled)

Support for SDRplay receivers uses the proprietary SDRplay API v3 by SDRplay
Limited. Neither its headers nor its library are part of this repository or of
the packages. At build time the SDRplay source is compiled only if
`sdrplay_api.h` is available; at run time `libsdrplay_api.so.3` is loaded with
`dlopen` if installed. See `LICENSE-EXCEPTION`.
