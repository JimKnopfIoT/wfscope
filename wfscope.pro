TARGET = wfscope

CONFIG += sailfishapp sailfishapp_i18n c++14

# Versionsnummer für die Über-Seite; synchron mit rpm/wfscope.spec halten
DEFINES += WFSCOPE_VERSION=\\\"0.2.2\\\"

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

QT += quick

# libusb direkt statt über PKGCONFIG (stört sonst die sailfishapp-Einbindung)
INCLUDEPATH += /usr/include/libusb-1.0
LIBS += -lusb-1.0 -ldl

# libhackrf und tinyalsa (beide BSD) werden mitkompiliert
INCLUDEPATH += 3rdparty/libhackrf 3rdparty/tinyalsa/include

# UKW-Ton: Sailfish-Audio-Ressource und PulseAudio
INCLUDEPATH += /usr/include/resource/qt5
LIBS += -lresourceqt5 -lpulse-simple -lpulse

# SDRplay-Unterstützung nur, wenn die (proprietären) API-Header vorhanden sind:
# 3rdparty/sdrplay_api/inc/ oder /usr/local/include. Die Bibliothek wird zur
# Laufzeit per dlopen geladen und ist nicht Teil des Pakets.
exists($$PWD/3rdparty/sdrplay_api/inc/sdrplay_api.h) {
    INCLUDEPATH += $$PWD/3rdparty/sdrplay_api/inc
    CONFIG += wfscope_sdrplay
} else:exists(/usr/local/include/sdrplay_api.h) {
    INCLUDEPATH += /usr/local/include
    CONFIG += wfscope_sdrplay
}
wfscope_sdrplay {
    DEFINES += WFSCOPE_HAVE_SDRPLAY
    SOURCES += src/sdrplaysource.cpp
    HEADERS += src/sdrplaysource.h
    message("SDRplay-Unterstützung: an")
} else {
    message("SDRplay-Unterstützung: aus (keine sdrplay_api.h gefunden)")
}

SOURCES += \
    src/main.cpp \
    src/spectrumsource.cpp \
    src/syntheticsource.cpp \
    src/icapsource.cpp \
    src/spectrumview.cpp \
    src/sourcehub.cpp \
    src/hackrfsource.cpp \
    src/fmsource.cpp \
    src/fmaudio.cpp \
    src/languagesettings.cpp \
    3rdparty/tinyalsa/src/mixer.c \
    3rdparty/tinyalsa/src/mixer_hw.c \
    3rdparty/tinyalsa/src/mixer_plugin.c \
    3rdparty/tinyalsa/src/pcm.c \
    3rdparty/tinyalsa/src/pcm_hw.c \
    3rdparty/tinyalsa/src/pcm_plugin.c \
    3rdparty/tinyalsa/src/snd_card_plugin.c \
    3rdparty/tinyalsa/src/limits.c \
    3rdparty/libhackrf/hackrf.c

HEADERS += \
    src/spectrumsource.h \
    src/syntheticsource.h \
    src/icapsource.h \
    src/spectrumview.h \
    src/sourcehub.h \
    src/hackrfsource.h \
    src/fmsource.h \
    src/fmaudio.h \
    src/languagesettings.h \
    src/fft.h \
    3rdparty/libhackrf/hackrf.h

DISTFILES += \
    helper/wfscope-icap.c \
    qml/wfscope.qml \
    qml/pages/ScopePage.qml \
    qml/components/FrequencyDisplay.qml \
    qml/components/ToggleDot.qml \
    qml/pages/AboutPage.qml \
    qml/cover/CoverPage.qml \
    rpm/wfscope.spec \
    wfscope.desktop

OTHER_FILES += README.md

# Oberflächensprachen (wie xmatic); Quelltexte sind englisch
TRANSLATIONS += \
    translations/wfscope-ar.ts \
    translations/wfscope-bg.ts \
    translations/wfscope-cs.ts \
    translations/wfscope-da.ts \
    translations/wfscope-de.ts \
    translations/wfscope-el.ts \
    translations/wfscope-en.ts \
    translations/wfscope-es.ts \
    translations/wfscope-et.ts \
    translations/wfscope-fa.ts \
    translations/wfscope-fi.ts \
    translations/wfscope-fr.ts \
    translations/wfscope-ga.ts \
    translations/wfscope-hi.ts \
    translations/wfscope-hr.ts \
    translations/wfscope-hu.ts \
    translations/wfscope-is.ts \
    translations/wfscope-it.ts \
    translations/wfscope-ja.ts \
    translations/wfscope-lt.ts \
    translations/wfscope-lv.ts \
    translations/wfscope-mt.ts \
    translations/wfscope-nb.ts \
    translations/wfscope-nl.ts \
    translations/wfscope-pl.ts \
    translations/wfscope-pt.ts \
    translations/wfscope-ro.ts \
    translations/wfscope-ru.ts \
    translations/wfscope-sk.ts \
    translations/wfscope-sl.ts \
    translations/wfscope-sv.ts \
    translations/wfscope-zh_CN.ts
