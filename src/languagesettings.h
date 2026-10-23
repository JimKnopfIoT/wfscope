#ifndef LANGUAGESETTINGS_H
#define LANGUAGESETTINGS_H

#include <QObject>
#include <QString>
#include <QVariantList>

class QCoreApplication;

// Sprache der Oberfläche (wie in xmatic): leer = wie das Gerät, sonst fest gewählt.
// Wirkt ab dem nächsten Start; Qt 5.6 kann eine geladene QML-Oberfläche nicht neu übersetzen.
class LanguageSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString code READ code WRITE setCode NOTIFY changed)
    // [{code, name}], Name jeweils in der eigenen Sprache; "Wie das Gerät" zuerst
    Q_PROPERTY(QVariantList available READ available CONSTANT)
public:
    explicit LanguageSettings(QObject *parent = nullptr);

    QString code() const { return m_code; }
    void setCode(const QString &code);
    QVariantList available() const;

    // Gewählten Katalog installieren; nach SailfishApp::createView() aufrufen,
    // denn der zuletzt installierte Übersetzer wird zuerst gefragt
    static void applyTo(QCoreApplication *app);

signals:
    void changed();

private:
    QString m_code;
};

#endif
