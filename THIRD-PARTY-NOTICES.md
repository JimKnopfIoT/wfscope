# Third-party notices

## libhackrf (bundled in `3rdparty/libhackrf/`)

From HackRF 2026.01.3 — https://github.com/greatscottgadgets/hackrf —
`host/libhackrf/src/hackrf.{c,h}`, unmodified. BSD-3-Clause; the full notice is
in `3rdparty/libhackrf/LICENSE`.

## SDRplay API (not bundled)

Support for SDRplay receivers uses the proprietary SDRplay API v3 by SDRplay
Limited. Neither its headers nor its library are part of this repository or of
the packages. At build time the SDRplay source is compiled only if
`sdrplay_api.h` is available; at run time `libsdrplay_api.so.3` is loaded with
`dlopen` if installed. See `LICENSE-EXCEPTION`.
