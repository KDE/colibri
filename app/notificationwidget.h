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

#include <QPropertyAnimation>
#include <QScopedPointer>
#include <QWidget>

class QLabel;
class QTimeLine;
class QTimer;

namespace Plasma
{
class FrameSvg;
}

namespace Colibri
{

class Label;

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
 * A widget which shows an notification
 */
class NotificationWidget : public QWidget
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

protected:
    virtual void paintEvent(QPaintEvent*);
    virtual void resizeEvent(QResizeEvent*);

private Q_SLOTS:
    void updateOpacity();
    void updateMouseOverOpacity();

private:
    QString mAppName;
    uint mId;
    QString mSummary;
    QString mBody;
    QTimeLine* mVisibleTimeLine;

    Label* mTextLabel;

    uint mCloseReason;
    Qt::Alignment mAlignment;
    int mScreen;
    Plasma::FrameSvg* mBackground;

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
