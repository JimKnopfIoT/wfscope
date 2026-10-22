# wfscope

**A receive-only spectrum analyser and waterfall for Sailfish OS.**

wfscope shows a live spectrum with frequency axis and a scrolling waterfall. It
only **listens** — it never transmits, never connects to anything it observes
and never interferes. Everything runs on the device; there is no telemetry and
no cloud component.

> ⚠️ **Scope & legality.** This is a passive *receiver*. It does not jam, spoof
> or interfere with anything. Whether and how you may use radio-analysis tools
> can depend on your jurisdiction — you are responsible for using it lawfully.

## Sources

| Source | What it gives | Needs |
|---|---|---|
| **Synthetic** | demo signals | nothing |
| **Wi-Fi chip (ICAP)** | 2.4/5 GHz Wi-Fi bands from the phone's own MediaTek connac2x Wi-Fi chip: 32 768 IQ samples per capture at 320 MS/s (102.4 µs snapshots), stepped in 40/80 MHz pieces | root (helper `wfscope-icap`); Wi-Fi is **off** while it runs |
| **HackRF One / PortaPack** (HackRF mode) | 1 MHz – 6 GHz in sweep mode (20 MHz steps, 78 kHz bins); 2.4 + 5 GHz in one view | USB-OTG, USB access to the device |
| **SDRplay RSPdx** (optional) | 1 kHz – 2 GHz, 8 MHz per tuning; **panadapter mode** for a transceiver's first IF (e.g. an IF tap board, default 69.450 MHz), axis shown in VFO frequency, mirror switch, fine offset | USB-OTG, the separately installed SDRplay API (`sdrplay_apiService` running) |

### Wi-Fi chip (ICAP) — read this first

- The capture runs the Wi-Fi chip in **RF test mode**; normal Wi-Fi is down for
  the whole session. The root helper restores it on exit, on stdin EOF, on
  SIGTERM, and through a detached watchdog even if the helper is killed
  (abort test mode, then a connman Wi-Fi power cycle). Never write `0` to
  `/dev/wmtWifi` yourself — that powers the chip off.
- The app starts the helper through `pkexec`. No polkit rule is installed by
  the package; either run wfscope as root or add a polkit rule for
  `/usr/libexec/wfscope/wfscope-icap` yourself, knowing that it lets the
  permitted user switch Wi-Fi into test mode.
- The vendor Wi-Fi driver has been seen to crash in its own error-recovery
  path after a TX stall some time after test-mode sessions. Use this source as
  an experiment, not as something left running unattended.
- Levels are **uncalibrated** (dB relative, not dBm).

### HackRF / SDRplay

- The app opens the HackRF through libusb; a normal user needs a udev rule
  granting access to the HackRF's USB IDs, or run as root.
- SDRplay support is compiled only if `sdrplay_api.h` is found (in
  `3rdparty/sdrplay_api/inc/` or `/usr/local/include`). The SDRplay API is
  proprietary and is **not** part of this repository or the packages; the
  library is loaded with `dlopen` at run time. Its headers must never be
  committed here.

## Building

Requires the [Sailfish OS SDK](https://sailfishos.org/develop/). From the
project root:

```sh
mb2 -t SailfishOS-<version>-aarch64 build    # produces an RPM under RPMS/
```

Install on the device:

```sh
scp RPMS/wfscope-*.aarch64.rpm <device>:/tmp/
ssh <device> 'pkcon install-local -y /tmp/wfscope-*.aarch64.rpm'
```

## Status & responsible use

**Proof of concept / work in progress**, shared **as is** with **no warranty**
of any kind (see the GPLv3). The ICAP path relies on undocumented vendor test
interfaces and may stop working, or misbehave, with other firmware or driver
versions. How you use it is your responsibility.

---

AI assisted development

## License

Licensed under the **GNU General Public License v3.0 or later** (see
[`LICENSE`](LICENSE)), with an additional permission under section 7 for
combining with the SDRplay API (see [`LICENSE-EXCEPTION`](LICENSE-EXCEPTION)).
Anyone distributing wfscope or a modified version must provide the source code
under the same terms. The bundled libhackrf keeps its BSD-3-Clause licence, see
[`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md).
