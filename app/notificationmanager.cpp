// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Colibri: Light notification system for KDE4
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "notificationmanager.moc"

// Qt
#include <QDBusConnection>

// KDE
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>

// Local
#include <notificationsadaptor.h>

namespace Colibri
{

struct NotificationManagerPrivate
{
};

NotificationManager::NotificationManager()
: d(new NotificationManagerPrivate)
{
    new NotificationsAdaptor(this);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerObject("/org/freedesktop/Notifications", this);
    connection.registerService("org.freedesktop.Notifications");
}

NotificationManager::~NotificationManager()
{
    delete d;
}

uint NotificationManager::Notify(const QString& appName, uint replacesId, const QString& appIcon, const QString& summary, const QString& body, const QStringList& actions, const QVariantMap& hints, int timeout)
{
    kDebug() << appName << summary << body;
}

void NotificationManager::CloseNotification(uint id)
{
    Q_UNUSED(id);
}

QStringList NotificationManager::GetCapabilities()
{
    return QStringList()
        << "body"
        << "body-hyperlinks"
        << "body-markup"
        << "icon-static"
        ;
}

QString NotificationManager::GetServerInformation(QString& vendor, QString& version, QString& specVersion)
{
    vendor = "Aurélien Gâteau";
    version = KCmdLineArgs::aboutData()->version();
    specVersion = "1.1";
    return KCmdLineArgs::aboutData()->appName();
}

} // namespace
