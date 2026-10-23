#include <QtQuick>
#include <QGuiApplication>
#include <QQuickView>
#include <sailfishapp.h>

#include "spectrumsource.h"
#include "syntheticsource.h"
#include "icapsource.h"
#include "spectrumview.h"
#include "hackrfsource.h"
#ifdef WFSCOPE_HAVE_SDRPLAY
#include "sdrplaysource.h"
#endif
#include "sourcehub.h"
#include "fmsource.h"
#include "languagesettings.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());
    // Nach createView() (installiert den Katalog der Gerätesprache) und vor den Quellen,
    // deren Konstruktoren schon Statustexte setzen
    LanguageSettings::applyTo(app.data());
    auto *language = new LanguageSettings(app.data());

    qmlRegisterType<SpectrumView>("wfscope", 1, 0, "SpectrumView");
    qRegisterMetaType<QVector<float>>("QVector<float>");

    auto *hub = new SourceHub(app.data());
    hub->add(new SyntheticSource);
    hub->add(new FmSource);
    hub->add(new IcapSource);
    hub->add(new HackRfSource);
#ifdef WFSCOPE_HAVE_SDRPLAY
    hub->add(new SdrplaySource);
#endif
    // Startquelle: WFSCOPE_SOURCE=synthetic|fm|icap|hackrf|sdrplay (alt: WFSCOPE_ICAP=1)
    const QString startKind = QString::fromLocal8Bit(qgetenv("WFSCOPE_SOURCE"));
    if (!startKind.isEmpty())
        hub->selectKind(startKind);
    else if (qEnvironmentVariableIntValue("WFSCOPE_ICAP") != 0)
        hub->selectKind(QStringLiteral("icap"));

    view->rootContext()->setContextProperty("hub", hub);
    view->rootContext()->setContextProperty("language", language);
    view->rootContext()->setContextProperty("appVersion", QStringLiteral(WFSCOPE_VERSION));
    // Entwicklerhilfe: Startband (Index in der Bandliste der Quelle)
    view->rootContext()->setContextProperty("initialBand", qEnvironmentVariableIntValue("WFSCOPE_BAND"));
    view->setSource(SailfishApp::pathTo("qml/wfscope.qml"));
    view->show();
    // Entwicklerhilfe: Quelle ohne Antippen starten (Tests per SSH)
    if (qEnvironmentVariableIntValue("WFSCOPE_AUTOSTART") != 0)
        QTimer::singleShot(1000, hub, [hub] {
            hub->currentSource()->start();
            // Entwicklerhilfe: UKW intern direkt im Hörmodus starten (WFSCOPE_LISTEN=89.5)
            bool ok = false;
            const double mhz = qgetenv("WFSCOPE_LISTEN").toDouble(&ok);
            if (ok)
                QMetaObject::invokeMethod(hub->currentSource(), "listenTo", Q_ARG(double, mhz));
        });
    return app->exec();
}
