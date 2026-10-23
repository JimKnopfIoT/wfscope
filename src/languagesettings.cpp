#include "languagesettings.h"

#include <QCoreApplication>
#include <QSettings>
#include <QTranslator>
#include <QVariantMap>

namespace {

const char kTranslations[] = "/usr/share/wfscope/translations";

QString storedCode()
{
    return QSettings().value(QStringLiteral("ui/language")).toString();
}

// Endonyme: jede Sprache in sich selbst benannt, damit die Liste auch lesbar ist,
// wenn man die aktuelle Oberflächensprache nicht versteht (Auswahl wie in xmatic)
const char *const kLanguages[][2] = {
    { "ar", "العربية" },    { "bg", "Български" }, { "cs", "Čeština" },
    { "da", "Dansk" },
    { "de", "Deutsch" },   { "el", "Ελληνικά" },  { "en", "English" },
    { "es", "Español" },   { "et", "Eesti" },     { "fa", "فارسی" },
    { "fi", "Suomi" },
    { "fr", "Français" },  { "ga", "Gaeilge" },   { "hr", "Hrvatski" },
    { "hi", "हिन्दी" },       { "hu", "Magyar" },    { "is", "Íslenska" },
    { "it", "Italiano" },  { "ja", "日本語" },
    { "lt", "Lietuvių" },  { "lv", "Latviešu" },  { "mt", "Malti" },
    { "nb", "Norsk bokmål" }, { "nl", "Nederlands" }, { "pl", "Polski" },
    { "pt", "Português" }, { "ro", "Română" },    { "ru", "Русский" },
    { "sk", "Slovenčina" }, { "sl", "Slovenščina" }, { "sv", "Svenska" },
    { "zh_CN", "简体中文" },
};

} // namespace

LanguageSettings::LanguageSettings(QObject *parent)
    : QObject(parent)
    , m_code(storedCode())
{
}

void LanguageSettings::setCode(const QString &code)
{
    if (code == m_code)
        return;
    m_code = code;
    QSettings().setValue(QStringLiteral("ui/language"), code);
    emit changed();
}

QVariantList LanguageSettings::available() const
{
    QVariantList list;
    QVariantMap automatic;
    automatic.insert(QStringLiteral("code"), QString());
    automatic.insert(QStringLiteral("name"), tr("Follow the device"));
    list.append(automatic);
    for (const auto &entry : kLanguages) {
        QVariantMap language;
        language.insert(QStringLiteral("code"), QString::fromLatin1(entry[0]));
        language.insert(QStringLiteral("name"), QString::fromUtf8(entry[1]));
        list.append(language);
    }
    return list;
}

void LanguageSettings::applyTo(QCoreApplication *app)
{
    const QString code = storedCode();
    if (code.isEmpty())
        return;
    auto *translator = new QTranslator(app);
    if (translator->load(QStringLiteral("wfscope-") + code, QLatin1String(kTranslations)))
        app->installTranslator(translator);
    else
        delete translator;          // kein Katalog: es bleibt bei der Gerätesprache
}
