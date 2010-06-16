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
#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

// Qt
#include <QObject>
#include <QVariant>

// KDE

// Local

namespace Colibri
{

class Config;

class NotificationWidget;
class NotificationManager : public QObject
{
    Q_OBJECT
public:
    NotificationManager();
    ~NotificationManager();

    uint Notify(const QString& appName, uint replacesId, const QString& appIcon, const QString& summary, const QString& body, const QStringList& actions, const QVariantMap& hints, int timeout);

    void CloseNotification(uint id);

    QStringList GetCapabilities();

    QString GetServerInformation(QString& vendor, QString& version, QString& specVersion);

Q_SIGNALS:
    void NotificationClosed(uint id, uint reason);
    void ActionInvoked(uint id, const QString& actionKey);

private Q_SLOTS:
    void slotNotificationWidgetClosed(uint id, uint reason);

private:
    QList<NotificationWidget*> mWidgets;
    uint mNextId;
    Config* mConfig;

    NotificationWidget* findWidget(const QString& appName, const QString& summary);
};

} // namespace

#endif /* NOTIFICATIONMANAGER_H */
