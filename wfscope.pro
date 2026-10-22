TARGET = wfscope

CONFIG += sailfishapp c++14

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

QT += quick

# libusb direkt statt über PKGCONFIG (stört sonst die sailfishapp-Einbindung)
INCLUDEPATH += /usr/include/libusb-1.0
LIBS += -lusb-1.0 -ldl

# libhackrf (BSD) wird mitkompiliert
INCLUDEPATH += 3rdparty/libhackrf

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
    3rdparty/libhackrf/hackrf.c

HEADERS += \
    src/spectrumsource.h \
    src/syntheticsource.h \
    src/icapsource.h \
    src/spectrumview.h \
    src/sourcehub.h \
    src/hackrfsource.h \
    src/fft.h \
    3rdparty/libhackrf/hackrf.h

DISTFILES += \
    helper/wfscope-icap.c \
    qml/wfscope.qml \
    qml/pages/ScopePage.qml \
    qml/cover/CoverPage.qml \
    rpm/wfscope.spec \
    wfscope.desktop

OTHER_FILES += README.md
