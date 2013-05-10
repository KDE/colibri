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

// Qt
#include <QPropertyAnimation>
#include <QScopedPointer>
#include <QWidget>

// KDE
#include <Plasma/Dialog>

class QGraphicsScene;
class QTimeLine;
class QTimer;

class HLayout;

namespace Plasma
{
class FrameSvg;
class Label;
}

namespace Colibri
{

class NotificationWidget;

class State : public QObject
{
Q_OBJECT
public:
    State(NotificationWidget*);

    virtual void onStarted() {}
    virtual void onAppended() {}
    virtual void onMouseOver() {}
    virtual void onMouseLeave() {}

protected:
    void switchToState(State*);
    NotificationWidget* mNotificationWidget;
};

class FadeInState : public State
{
Q_OBJECT
public:
    FadeInState(NotificationWidget* widget);
private Q_SLOTS:
    void slotFinished();
};

class VisibleState : public State
{
Q_OBJECT
public:
    VisibleState(NotificationWidget* widget);
    virtual void onMouseOver();
    virtual void onMouseLeave();
private Q_SLOTS:
    void slotFinished();
private:
    QTimeLine* mTimeLine;
};

class FadeOutState : public State
{
Q_OBJECT
public:
    FadeOutState(NotificationWidget* widget);
    virtual void onAppended();
private Q_SLOTS:
    void slotFinished();
};

/**
 * A widget which shows a notification
 */
class NotificationWidget : public Plasma::Dialog
{
    Q_OBJECT
public:
    NotificationWidget(const QString& appName, uint id, const QImage& image, const QString& appIcon, const QString& summary, const QString& body, int timeout);

    Q_PROPERTY(qreal fadeOpacity READ fadeOpacity WRITE setFadeOpacity)

    void start();

    void setAlignment(Qt::Alignment);

    void setScreen(int);

    uint id() const { return mId; }

    QString appName() const { return mAppName; }

    QString summary() const { return mSummary; }

    QTimeLine* visibleTimeLine() const { return mVisibleTimeLine; }

    void appendToBody(const QString&, int timeout);

    // Not named close() to avoid confusion with QWidget::close()
    void closeWidget();

    qreal fadeOpacity() const;
    void setFadeOpacity(qreal);

    void emitClosed();

Q_SIGNALS:
    void closed(uint id, uint reason);

private Q_SLOTS:
    void updateOpacity();
    void updateMouseOverOpacity();

private:
    QString mAppName;
    uint mId;
    QString mSummary;
    QString mBody;
    QTimeLine* mVisibleTimeLine;

    QGraphicsScene* mScene;
    QGraphicsWidget* mContainer;
    QScopedPointer<HLayout> mHLayout;
    Plasma::Label* mIconLabel;
    Plasma::Label* mTextLabel;
    Plasma::FrameSvg* mBackgroundSvg;

    uint mCloseReason;
    Qt::Alignment mAlignment;
    int mScreen;

    State* mState;

    QTimer* mMousePollTimer;

    qreal mFadeOpacity;
    qreal mMouseOverOpacity;
    QScopedPointer<QPropertyAnimation> mGrowAnimation;

    void setInputMask();
    void updateTextLabel();
    void adjustSizeAndPosition();
    QRect idealGeometry() const;

    friend class State;
};

} // namespace

#endif /* NOTIFICATIONWIDGET_H */
