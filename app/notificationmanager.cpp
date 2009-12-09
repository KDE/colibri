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
#include <notificationwidget.h>

namespace Colibri
{

NotificationManager::NotificationManager()
: mNextId(1)
{
    new NotificationsAdaptor(this);
    bool ok;
    QDBusConnection connection = QDBusConnection::sessionBus();
    ok = connection.registerObject("/org/freedesktop/Notifications", this);
    if (!ok) {
        kWarning() << "Could not register object /org/freedesktop/Notifications";
        return;
    }
    ok = connection.registerService("org.freedesktop.Notifications");
    if (!ok) {
        kWarning() << "Could not register service org.freedesktop.Notifications";
        return;
    }
    kDebug() << "Registered";
}

NotificationManager::~NotificationManager()
{
}

uint NotificationManager::Notify(const QString& appName, uint replacesId, const QString& appIcon, const QString& summary, const QString& body, const QStringList& actions, const QVariantMap& hints, int timeout)
{
    Q_UNUSED(appName);
    Q_UNUSED(replacesId);
    Q_UNUSED(actions);
    kDebug() << appName << summary << body;
    QPixmap pix;
    Q_UNUSED(appIcon);
    Q_UNUSED(hints);
    /*
    QImage img = notification->image();
    if (!img.isNull()) {
        if (qMax(img.width(), img.height()) > ICON_SIZE) {
            img = img.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        mIconLabel->setPixmap(QPixmap::fromImage(img));
        mIconLabel->show();
    } else if (!notification->applicationIcon().isNull()) {
        QPixmap pix = notification->applicationIcon().pixmap(ICON_SIZE);
    }
    */

    uint id = mNextId++;
    NotificationWidget* widget = new NotificationWidget(id, pix, summary, body, timeout);
    connect(widget, SIGNAL(fadedOut()), SLOT(showNext()));
    mWidgets << widget;
    if (mWidgets.size() == 1) {
        widget->fadeIn();
    }
    return id;
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

void NotificationManager::showNext()
{
    if (mWidgets.isEmpty()) {
        kWarning() << "mWidgets should not be empty!";
        return;
    }
    NotificationWidget* widget = mWidgets.takeFirst();
    widget->deleteLater();

    if (!mWidgets.isEmpty()) {
        mWidgets.first()->fadeIn();
    }
}

} // namespace
