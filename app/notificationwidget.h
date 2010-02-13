// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Colibri: Light notification system for KDE4
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

Based on Ayatana Notifications for Plasma, Copyright 2009 Canonical Ltd.

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
#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>

class QLabel;
class QTimeLine;
class QTimer;
class QWidget;

namespace Plasma
{
class FrameSvg;
}

namespace Colibri
{

class Notification;

/**
 * A widget which shows an notification
 */
class NotificationWidget : public QWidget
{
    Q_OBJECT
public:
    NotificationWidget(uint id, const QImage& image, const QString& appIcon, const QString& summary, const QString& body, int timeout);

    void fadeIn();

    void setAlignment(Qt::Alignment);

Q_SIGNALS:
    void closed(uint id, uint reason);

protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void resizeEvent(QResizeEvent*);

private Q_SLOTS:
    void fadeOut();
    void updateOpacity();
    void slotFadeTimeLineFinished();
    void updateMouseOverOpacity();

private:
    uint mId;
    Qt::Alignment mAlignment;
    Plasma::FrameSvg* mBackground;

    // This timeline is used as a QTimer to handle the bubble appearance
    // duration.  It starts when initial fade in is done and ends when final
    // fade out starts.  We use a QTimeLine instead of QTimer because a
    // QTimeLine can be paused and resumed.
    QTimeLine* mLifeTimeLine;

    // This timeline handles the fade in/out animation
    QTimeLine* mFadeTimeLine;

    QTimer* mMousePollTimer;

    qreal mMouseOverOpacity;

    void setInputMask();
    void startLifeTimer();
};

} // namespace

#endif /* NOTIFICATIONWIDGET_H */
