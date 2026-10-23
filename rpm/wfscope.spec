Name:       wfscope

# Neutralise the RPM BuildHost header so the build machine's hostname (and home
# network) never leak in a published package.
%define _buildhost reproducible-builder

Summary:    Spectrum analyser and waterfall with FM radio, Wi-Fi bands and external SDRs
Version:    0.2.0
Release:    1
License:    GPLv3+
Group:      Qt/Qt
URL:        https://github.com/JimKnopfIoT/wfscope
Vendor:     wfscope contributors
Packager:   wfscope contributors
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires: pkgconfig(sailfishapp) >= 1.0.2
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(libusb-1.0)
BuildRequires: pkgconfig(libresourceqt5)
BuildRequires: pkgconfig(libpulse-simple)
BuildRequires: desktop-file-utils

%description
Receive-only spectrum and waterfall display for Sailfish OS. Sources: the
phone's internal FM radio (band scan, audio, RDS station names), the
phone's own MediaTek connac2x Wi-Fi chip (ICAP IQ snapshots, root helper),
a HackRF One / PortaPack in sweep mode, an SDRplay RSPdx (optional, needs the
separately installed SDRplay API; panadapter mode for a transceiver IF tap),
and a synthetic demo source.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
%make_build
gcc %{optflags} -o helper/wfscope-icap helper/wfscope-icap.c

%install
%qmake5_install
install -D -m 0755 helper/wfscope-icap %{buildroot}%{_libexecdir}/%{name}/wfscope-icap

%files
%defattr(-,root,root,-)
%license LICENSE LICENSE-EXCEPTION
%{_bindir}/%{name}
%{_libexecdir}/%{name}/wfscope-icap
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%changelog
* Fri Oct 23 2026 wfscope contributors - 0.2.0-1
- Internal FM radio source: band scan, clean audio, RDS station names checked
  by PI code, manual names, last frequency restored on start.
- Gqrx-style tuning: red centre line, slide or tap a peak (magnetic snap),
  frequency display with digit stepping, channel raster.
- New waterfall palette and intensity slider, labels for frequencies and
  names, landscape layout, About page, 32 UI languages, new icon.
* Fri Oct 23 2026 wfscope contributors - 0.1.0-1
- First release: Wi-Fi ICAP source, HackRF sweep source, optional SDRplay
  source with panadapter mode, spectrum with frequency axis and waterfall.
