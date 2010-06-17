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
class QSequentialAnimationGroup;
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
    NotificationWidget(const QString& appName, uint id, const QImage& image, const QString& appIcon, const QString& summary, const QString& body, int timeout);

    Q_PROPERTY(qreal fadeOpacity READ fadeOpacity WRITE setFadeOpacity)

    void fadeIn();

    void setAlignment(Qt::Alignment);

    uint id() const { return mId; }

    QString appName() const { return mAppName; }

    QString summary() const { return mSummary; }

    QString body() const { return mBody; }
    void setBody(const QString&);

    // Not named close() to avoid confusion with QWidget::close()
    void closeWidget();

    qreal fadeOpacity() const;
    void setFadeOpacity(qreal);

Q_SIGNALS:
    void closed(uint id, uint reason);

protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void resizeEvent(QResizeEvent*);

private Q_SLOTS:
    void fadeOut();
    void updateOpacity();
    void slotAnimationFinished();
    void updateMouseOverOpacity();

private:
    QString mAppName;
    uint mId;
    QString mSummary;
    QString mBody;

    QLabel* mTextLabel;

    uint mCloseReason;
    Qt::Alignment mAlignment;
    Plasma::FrameSvg* mBackground;

    QSequentialAnimationGroup* mAnimation;

    QTimer* mMousePollTimer;

    qreal mFadeOpacity;
    qreal mMouseOverOpacity;

    void setInputMask();
    void updateTextLabel();
};

} // namespace

#endif /* NOTIFICATIONWIDGET_H */
