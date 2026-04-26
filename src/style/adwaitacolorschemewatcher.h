#ifndef ADWAITA_COLORSCHEMEWATCHER_H
#define ADWAITA_COLORSCHEMEWATCHER_H

#include <QObject>
#include <QtDBus/QDBusVariant>

namespace Adwaita
{

class ColorSchemeWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ColorSchemeWatcher(QObject *parent = nullptr);
    ~ColorSchemeWatcher() override = default;

    bool prefersDark() const { return m_prefersDark; }

Q_SIGNALS:
    void colorSchemeChanged(bool dark);

private Q_SLOTS:
    void onSettingChanged(const QString &nspace, const QString &key, const QDBusVariant &value);

private:
    void readInitial();
    bool readPortalScheme(uint &out) const;

    bool m_prefersDark { false };
};

} // namespace Adwaita

#endif // ADWAITA_COLORSCHEMEWATCHER_H
