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

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    qmlRegisterType<SpectrumView>("wfscope", 1, 0, "SpectrumView");
    qRegisterMetaType<QVector<float>>("QVector<float>");

    auto *hub = new SourceHub(app.data());
    hub->add(new SyntheticSource);
    hub->add(new IcapSource);
    hub->add(new HackRfSource);
#ifdef WFSCOPE_HAVE_SDRPLAY
    hub->add(new SdrplaySource);
#endif
    // Startquelle: WFSCOPE_SOURCE=synthetic|icap|hackrf|sdrplay (alt: WFSCOPE_ICAP=1)
    const QString startKind = QString::fromLocal8Bit(qgetenv("WFSCOPE_SOURCE"));
    if (!startKind.isEmpty())
        hub->selectKind(startKind);
    else if (qEnvironmentVariableIntValue("WFSCOPE_ICAP") != 0)
        hub->selectKind(QStringLiteral("icap"));

    view->rootContext()->setContextProperty("hub", hub);
    // Entwicklerhilfe: Startband (Index in der Bandliste der Quelle)
    view->rootContext()->setContextProperty("initialBand", qEnvironmentVariableIntValue("WFSCOPE_BAND"));
    view->setSource(SailfishApp::pathTo("qml/wfscope.qml"));
    view->show();
    // Entwicklerhilfe: Quelle ohne Antippen starten (Tests per SSH)
    if (qEnvironmentVariableIntValue("WFSCOPE_AUTOSTART") != 0)
        QTimer::singleShot(1000, hub, [hub] { hub->currentSource()->start(); });
    return app->exec();
}
