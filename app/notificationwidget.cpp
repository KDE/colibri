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
#include "notificationwidget.h"

// libc
#include <math.h>

// X11
#include <X11/extensions/shape.h>
#include <fixx11h.h>

// Qt
#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QX11Info>

// KDE
#include <KDebug>
#include <KDialog>
#include <KWindowSystem>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>

// Local

namespace Colibri
{

static const uint CLOSE_REASON_EXPIRED        = 1;
static const uint CLOSE_REASON_CLOSED_BY_USER = 2;
static const uint CLOSE_REASON_CLOSED_BY_APP  = 3;

static const int DEFAULT_BUBBLE_MIN_HEIGHT = 50;
static const int DEFAULT_FADE_IN_TIMEOUT   = 250;
static const int DEFAULT_FADE_OUT_TIMEOUT  = 1000;
static const int DEFAULT_ON_SCREEN_TIMEOUT = 3000; //10000;

static const int ICON_SIZE = KIconLoader::SizeMedium;

// 60 FPS to ensure a smooth animation
static const int MOUSE_POLL_INTERVAL = 1000 / 60;

static const int   MOUSE_OVER_MARGIN      = 48;
static const qreal MOUSE_OVER_OPACITY_MIN = .4;

// When running on a non composited desktop, if opacity is less than this value
// the widget will be hidden (should not be less than MOUSE_OVER_OPACITY_MIN!)
static const qreal NON_COMPOSITED_OPACITY_THRESHOLD = .4;

NotificationWidget::NotificationWidget(const QString& appName, uint id, const QImage& image_, const QString& appIcon, const QString& summary, const QString& body, int timeout)
: mAppName(appName)
, mId(id)
, mSummary(summary)
, mBody(body)
, mTextLabel(new QLabel(this))
, mCloseReason(CLOSE_REASON_EXPIRED)
, mAlignment(Qt::AlignRight | Qt::AlignTop)
, mBackground(new Plasma::FrameSvg(this))
, mAnimation(new QSequentialAnimationGroup(this))
, mMousePollTimer(new QTimer(this))
, mFadeOpacity(1.)
, mMouseOverOpacity(1.)
{
    QPropertyAnimation* fadeInAnim = new QPropertyAnimation(this, "fadeOpacity");
    fadeInAnim->setDuration(DEFAULT_FADE_IN_TIMEOUT);
    fadeInAnim->setStartValue(0.);
    fadeInAnim->setEndValue(1.);

    QPropertyAnimation* fadeOutAnim = new QPropertyAnimation(this, "fadeOpacity");
    fadeOutAnim->setDuration(DEFAULT_FADE_OUT_TIMEOUT);
    fadeOutAnim->setStartValue(1.);
    fadeOutAnim->setEndValue(0.);

    mAnimation->addAnimation(fadeInAnim);
    mAnimation->addPause(timeout == 0 ? DEFAULT_ON_SCREEN_TIMEOUT : timeout);
    mAnimation->addAnimation(fadeOutAnim);

    // Icon
    QPixmap pix;
    if (!image_.isNull()) {
        QImage image = image_;
        if (qMax(image.width(), image.height()) > ICON_SIZE) {
            image = image.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        pix = QPixmap::fromImage(image);
    } else if (!appIcon.isEmpty()) {
        pix = KIconLoader::global()->loadIcon(appIcon, KIconLoader::Panel,
            ICON_SIZE,
            KIconLoader::DefaultState,
            QStringList() /* overlays */,
            0L /* path_store */,
            true /* canReturnNull */);
    }

    // UI
    setWindowFlags(Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    setMinimumHeight(DEFAULT_BUBBLE_MIN_HEIGHT);

    mBackground->setImagePath("widgets/tooltip");
    const int topHeight = mBackground->marginSize(Plasma::TopMargin);
    const int leftWidth = mBackground->marginSize(Plasma::LeftMargin);
    const int rightWidth = mBackground->marginSize(Plasma::RightMargin);
    const int bottomHeight = mBackground->marginSize(Plasma::BottomMargin);
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    QLabel* iconLabel;
    if (pix.isNull()) {
        iconLabel = 0;
    } else {
        iconLabel = new QLabel(this);
        iconLabel->setAlignment(Qt::AlignTop | Qt::AlignCenter);
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        iconLabel->setPixmap(pix);
    }

    mTextLabel->setWordWrap(true);
    mTextLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    mTextLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    mTextLabel->setFont(theme->font(Plasma::Theme::DefaultFont));

    int averageCharWidth = mTextLabel->fontMetrics().averageCharWidth();
    mTextLabel->setFixedWidth(30 * averageCharWidth);
    updateTextLabel();
    adjustSize();

    QPalette palette = mTextLabel->palette();
    palette.setColor(QPalette::WindowText, theme->color(Plasma::Theme::TextColor));
    mTextLabel->setPalette(palette);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(KDialog::spacingHint());
    if (iconLabel) {
        layout->addWidget(iconLabel, 0 /* stretch */, Qt::AlignTop | Qt::AlignLeft);
    }
    layout->addWidget(mTextLabel);

    // Behavior
    connect(mAnimation, SIGNAL(finished()),
        SLOT(slotAnimationFinished()));
    setWindowOpacity(0);

    mMousePollTimer->setInterval(MOUSE_POLL_INTERVAL);
    connect(mMousePollTimer, SIGNAL(timeout()),
        SLOT(updateMouseOverOpacity()));
}

void NotificationWidget::updateTextLabel()
{
    QString text;
    if (!mSummary.isEmpty()) {
        text = "<b>" + mSummary + "</b>";
    }
    if (!mBody.isEmpty()) {
        if (!mSummary.isEmpty()) {
            text += "<br/>";
        }
        text += mBody;
    }
    mTextLabel->setText(text);
}

void NotificationWidget::setBody(const QString& body)
{
    mBody = body;
    updateTextLabel();
    fadeIn();
}

void NotificationWidget::setInputMask()
{
    // Create an empty input mask to achieve click-through effect
    // Thanks to MacSlow for this!
    Pixmap mask;

    mask = XCreatePixmap(QX11Info::display(),
            winId(),
            1, /* width  */
            1, /* height */
            1  /* depth  */);
    XShapeCombineMask(QX11Info::display(),
            winId(),
            ShapeInput,
            0, /* x-offset */
            0, /* y-offset */
            mask,
            ShapeSet);
    XFreePixmap(QX11Info::display(), mask);
}

void NotificationWidget::setAlignment(Qt::Alignment alignment)
{
    mAlignment = alignment;
}

void NotificationWidget::closeWidget()
{
    mCloseReason = CLOSE_REASON_CLOSED_BY_APP;
    fadeOut();
}

void NotificationWidget::fadeIn()
{
    adjustSize();
    setInputMask();

    QRect rect = QApplication::desktop()->availableGeometry(QCursor::pos());
    int left, top;
    if (mAlignment & Qt::AlignTop) {
        top = rect.top();
    } else if (mAlignment & Qt::AlignVCenter) {
        top = rect.top() + (rect.height() - height()) / 2;
    } else {
        top = rect.bottom() - height();
    }
    if (mAlignment & Qt::AlignLeft) {
        left = rect.left();
    } else if (mAlignment & Qt::AlignHCenter) {
        left = rect.left() + (rect.width() - width()) / 2;
    } else {
        left = rect.right() - width();
    }
    move(left, top);
    show();
    mAnimation->start();
    mMousePollTimer->start();
}

void NotificationWidget::fadeOut()
{
    // Go to fadeout directly if we were not already there
    mAnimation->removeAnimation(mAnimation->animationAt(0));
    mAnimation->removeAnimation(mAnimation->animationAt(1));
}

void NotificationWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    mBackground->paintFrame(&painter, QPointF(0, 0));
}

void NotificationWidget::resizeEvent(QResizeEvent*)
{
    mBackground->resizeFrame(size());
    setMask(mBackground->mask());
}

void NotificationWidget::updateOpacity()
{
    const qreal opacity = mFadeOpacity * mMouseOverOpacity;
    if (KWindowSystem::compositingActive()) {
        setWindowOpacity(opacity);
    } else {
        setVisible(opacity > NON_COMPOSITED_OPACITY_THRESHOLD);
    }
}

qreal NotificationWidget::fadeOpacity() const
{
    return mFadeOpacity;
}

void NotificationWidget::setFadeOpacity(qreal value)
{
    mFadeOpacity = value;
    updateOpacity();
}

static inline int distance(int value, int min, int max)
{
    if (value <= min) {
        return min - value;
    }
    if (value >= max) {
        return value - max;
    }
    return 0;
}

static inline qreal mouseOverOpacityFromPos(const QPoint& pos, const QRect& rect)
{
    #define returnIfOut(x) if (x >= MOUSE_OVER_MARGIN) return 1;

    int distanceX = distance(pos.x(), rect.left(), rect.right());
    returnIfOut(distanceX);
    int distanceY = distance(pos.y(), rect.top(), rect.bottom());
    returnIfOut(distanceY);
    int distance = sqrt(distanceX * distanceX + distanceY * distanceY);
    returnIfOut(distance);

    return MOUSE_OVER_OPACITY_MIN + (1. - MOUSE_OVER_OPACITY_MIN) * qreal(distance) / MOUSE_OVER_MARGIN;
    #undef returnIfOut
}

void NotificationWidget::updateMouseOverOpacity()
{
    qreal oldOpacity = mMouseOverOpacity;
    mMouseOverOpacity = mouseOverOpacityFromPos(QCursor::pos(), geometry());

    bool wasOver = oldOpacity < 1.;
    bool isOver = mMouseOverOpacity < 1.;
    if (!wasOver && isOver) {
        mAnimation->pause();
    } else if (wasOver && !isOver) {
        mAnimation->resume();
    }

    if (!qFuzzyCompare(mMouseOverOpacity, oldOpacity)) {
        updateOpacity();
    }
}

void NotificationWidget::slotAnimationFinished()
{
    emit closed(mId, mCloseReason);
}

} // namespace

#include "notificationwidget.moc"
