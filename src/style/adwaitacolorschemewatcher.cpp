#include "adwaitacolorschemewatcher.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusVariant>
#include <QVariant>

namespace Adwaita
{

namespace
{
constexpr const char *kPortalService = "org.freedesktop.portal.Desktop";
constexpr const char *kPortalPath    = "/org/freedesktop/portal/desktop";
constexpr const char *kPortalIface   = "org.freedesktop.portal.Settings";
constexpr const char *kAppearanceNs  = "org.freedesktop.appearance";
constexpr const char *kColorSchemeKey = "color-scheme";

// Portal returns: 0 = no preference, 1 = prefer dark, 2 = prefer light
bool schemeIsDark(uint v) { return v == 1; }
}

ColorSchemeWatcher::ColorSchemeWatcher(QObject *parent)
    : QObject(parent)
{
    readInitial();

    QDBusConnection::sessionBus().connect(
        QString::fromLatin1(kPortalService),
        QString::fromLatin1(kPortalPath),
        QString::fromLatin1(kPortalIface),
        QStringLiteral("SettingChanged"),
        this,
        SLOT(onSettingChanged(QString,QString,QDBusVariant)));
}

void ColorSchemeWatcher::readInitial()
{
    uint scheme = 0;
    if (readPortalScheme(scheme)) {
        m_prefersDark = schemeIsDark(scheme);
    } else {
        m_prefersDark = false;
    }
}

bool ColorSchemeWatcher::readPortalScheme(uint &out) const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QString::fromLatin1(kPortalService),
        QString::fromLatin1(kPortalPath),
        QString::fromLatin1(kPortalIface),
        QStringLiteral("Read"));
    msg << QString::fromLatin1(kAppearanceNs) << QString::fromLatin1(kColorSchemeKey);

    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }

    const QList<QVariant> args = reply.arguments();
    if (args.isEmpty()) {
        return false;
    }

    // Reply is variant<variant<uint>>; unwrap both layers.
    QVariant v = args.first();
    while (v.canConvert<QDBusVariant>()) {
        v = v.value<QDBusVariant>().variant();
    }

    bool ok = false;
    out = v.toUInt(&ok);
    return ok;
}

void ColorSchemeWatcher::onSettingChanged(const QString &nspace, const QString &key, const QDBusVariant &value)
{
    if (nspace != QLatin1String(kAppearanceNs) || key != QLatin1String(kColorSchemeKey)) {
        return;
    }

    QVariant v = value.variant();
    while (v.canConvert<QDBusVariant>()) {
        v = v.value<QDBusVariant>().variant();
    }

    bool ok = false;
    const uint scheme = v.toUInt(&ok);
    if (!ok) {
        return;
    }

    const bool dark = schemeIsDark(scheme);
    if (dark == m_prefersDark) {
        return;
    }
    m_prefersDark = dark;
    Q_EMIT colorSchemeChanged(dark);
}

} // namespace Adwaita
