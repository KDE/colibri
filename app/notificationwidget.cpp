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
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <fixx11h.h>

// Qt
#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QLabel>
#include <QTimeLine>
#include <QTimer>
#include <QX11Info>

// KDE
#include <KDebug>
#include <KIconLoader>
#include <KWindowSystem>

#include <Plasma/FrameSvg>
#include <Plasma/Label>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

// Local

// Enable this to quit colibri after one notification. Useful for memchecking
// with Valgrind.
//#define QUIT_AFTER_ONE

namespace Colibri
{

static const uint CLOSE_REASON_EXPIRED        = 1;
static const uint CLOSE_REASON_CLOSED_BY_USER = 2;
static const uint CLOSE_REASON_CLOSED_BY_APP  = 3;

static const int DEFAULT_BUBBLE_MIN_HEIGHT = 50;
static const int DEFAULT_FADE_IN_TIMEOUT   = 250;
static const int DEFAULT_FADE_OUT_TIMEOUT  = 1000;

static const int GROW_ANIMATION_DURATION = 200;

static const int ICON_SIZE = KIconLoader::SizeMedium;

// 60 FPS to ensure a smooth animation
static const int MOUSE_POLL_INTERVAL = 1000 / 60;

static const int   MOUSE_OVER_MARGIN      = 48;
static const qreal MOUSE_OVER_OPACITY_MIN = .4;

// When running on a non composited desktop, if opacity is less than this value
// the widget will be hidden (should not be less than MOUSE_OVER_OPACITY_MIN!)
static const qreal NON_COMPOSITED_OPACITY_THRESHOLD = .4;

////////////////////////////////////////////////////:
// State
////////////////////////////////////////////////////:
State::State(NotificationWidget* widget)
: QObject(widget)
, mNotificationWidget(widget)
{}

void State::switchToState(State* state)
{
    mNotificationWidget->mState = state;
    deleteLater();
}

////////////////////////////////////////////////////:
// HiddenState
////////////////////////////////////////////////////:
class HiddenState : public State
{
public:
    HiddenState(NotificationWidget* widget) : State(widget) {}

    void onStarted()
    {
        switchToState(new FadeInState(mNotificationWidget));
    }
};


////////////////////////////////////////////////////:
// FadeInState
////////////////////////////////////////////////////:
FadeInState::FadeInState(NotificationWidget* widget)
: State(widget)
{
    QPropertyAnimation* anim = new QPropertyAnimation(widget, "fadeOpacity", this);
    anim->setDuration(DEFAULT_FADE_IN_TIMEOUT);
    // FIXME: Adjust duration according to current opacity
    anim->setStartValue(mNotificationWidget->fadeOpacity());
    anim->setEndValue(1.);
    connect(anim, SIGNAL(finished()), SLOT(slotFinished()));
    anim->start();
}

void FadeInState::slotFinished()
{
    switchToState(new VisibleState(mNotificationWidget));
}


////////////////////////////////////////////////////:
// VisibleState
////////////////////////////////////////////////////:
VisibleState::VisibleState(NotificationWidget* widget)
: State(widget)
{
    connect(widget->visibleTimeLine(), SIGNAL(finished()), SLOT(slotFinished()));
    if (widget->visibleTimeLine()->state() == QTimeLine::NotRunning) {
        widget->visibleTimeLine()->start();
    }
}

void VisibleState::slotFinished()
{
    switchToState(new FadeOutState(mNotificationWidget));
}

void VisibleState::onMouseOver()
{
    mNotificationWidget->visibleTimeLine()->setPaused(true);
}

void VisibleState::onMouseLeave()
{
    mNotificationWidget->visibleTimeLine()->setPaused(false);
}

////////////////////////////////////////////////////:
// FadeOutState
////////////////////////////////////////////////////:
FadeOutState::FadeOutState(NotificationWidget* widget)
: State(widget)
{
    QPropertyAnimation* anim = new QPropertyAnimation(widget, "fadeOpacity", this);
    anim->setDuration(DEFAULT_FADE_OUT_TIMEOUT);
    anim->setStartValue(1.);
    anim->setEndValue(0.);
    connect(anim, SIGNAL(finished()), SLOT(slotFinished()));
    anim->start();
}

void FadeOutState::onAppended()
{
    switchToState(new FadeInState(mNotificationWidget));
}

void FadeOutState::slotFinished()
{
    mNotificationWidget->emitClosed();
}

static QString cleanBody(const QString& _body)
{
    QString body = _body;
    if (body.startsWith("<qt>", Qt::CaseInsensitive)) {
        body = body.mid(4);
    } else if (body.startsWith("<html>", Qt::CaseInsensitive)) {
        body = body.mid(6);
    }
    if (body.endsWith("</qt>", Qt::CaseInsensitive)) {
        body.chop(5);
    } else if (body.endsWith("</html>", Qt::CaseInsensitive)) {
        body.chop(6);
    }
    if (body.isEmpty()) {
        return QString();
    }
    return "<div>" + body + "</div>";
}

static QPixmap pixmapFromImage(const QImage& image_)
{
    if (image_.isNull()) {
        return QPixmap();
    }
    QImage image = image_;
    if (qMax(image.width(), image.height()) > ICON_SIZE) {
        image = image.scaled(ICON_SIZE, ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return QPixmap::fromImage(image);
}

static QPixmap pixmapFromAppIcon(const QString& appIcon)
{
    if (appIcon.isEmpty()) {
        return QPixmap();
    }
    return KIconLoader::global()->loadIcon(appIcon, KIconLoader::Panel,
        ICON_SIZE,
        KIconLoader::DefaultState,
        QStringList() /* overlays */,
        0L /* path_store */,
        true /* canReturnNull */);
}

////////////////////////////////////////////////////:
// NotificationWidget
////////////////////////////////////////////////////:
NotificationWidget::NotificationWidget(const QString& appName, uint id, const QImage& image, const QString& appIcon, const QString& summary, const QString& body, int timeout)
: Plasma::Dialog(0, Qt::ToolTip)
, mAppName(appName)
, mId(id)
, mSummary(summary)
, mBody(cleanBody(body))
, mVisibleTimeLine(new QTimeLine(timeout, this))
, mScene(new QGraphicsScene(this))
, mContainer(new QGraphicsWidget)
, mTextLabel(new Plasma::Label)
, mBackgroundSvg(new Plasma::FrameSvg(this))
, mCloseReason(CLOSE_REASON_EXPIRED)
, mAlignment(Qt::AlignRight | Qt::AlignTop)
, mScreen(-1)
, mState(new HiddenState(this))
, mMousePollTimer(new QTimer(this))
, mFadeOpacity(1.)
, mMouseOverOpacity(1.)
{
    //Setup the window properties
    KWindowSystem::setState(winId(), NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Tooltip);
    setAttribute(Qt::WA_X11NetWmWindowTypeToolTip, true);

    // Background. This is only used to get the dialog margins
    mBackgroundSvg->setImagePath("dialogs/background");
    mBackgroundSvg->setEnabledBorders(Plasma::FrameSvg::AllBorders);

    // Icon
    QPixmap pix = pixmapFromImage(image);
    if (pix.isNull()) {
        pix = pixmapFromAppIcon(appIcon);
    }

    // UI
    setMinimumHeight(DEFAULT_BUBBLE_MIN_HEIGHT);

    Plasma::Label* iconLabel;
    if (pix.isNull()) {
        iconLabel = 0;
    } else {
        QSize size = pix.size();
        iconLabel = new Plasma::Label;
        iconLabel->nativeWidget()->setPixmap(pix);
        iconLabel->nativeWidget()->setFixedSize(size);
        iconLabel->setMinimumSize(size);
        iconLabel->setMaximumSize(size);
    }

    mTextLabel->setWordWrap(true);

    QFontMetrics fm = mTextLabel->nativeWidget()->fontMetrics();
    mTextLabel->setMinimumWidth(27 * fm.averageCharWidth());
    mTextLabel->setMinimumHeight(fm.height());
    updateTextLabel();

    // Layout
    mLayout = new QGraphicsLinearLayout(mContainer);
    mLayout->setContentsMargins(0, 0, 0, 0);
    if (iconLabel) {
        mLayout->addItem(iconLabel);
    }
    mLayout->addItem(mTextLabel);

    mScene->addItem(mContainer);
    setGraphicsWidget(mContainer);

    syncToGraphicsWidget();

    // Behavior
    setWindowOpacity(0);
    hide();

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
        text += mBody;
    }
    mTextLabel->setText(text);
}

void NotificationWidget::appendToBody(const QString& body, int timeout)
{
    mBody += cleanBody(body);
    mVisibleTimeLine->setDuration(mVisibleTimeLine->duration() + timeout);
    updateTextLabel();
    if (isVisible()) {
        mGrowAnimation.reset(new QPropertyAnimation(this, "geometry"));
        mGrowAnimation->setEasingCurve(QEasingCurve::OutQuad);
        mGrowAnimation->setDuration(GROW_ANIMATION_DURATION);
        mGrowAnimation->setStartValue(geometry());
        mGrowAnimation->setEndValue(idealGeometry());
        mGrowAnimation->start();
    }
    mState->onAppended();
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

void NotificationWidget::setScreen(int screen)
{
    mScreen = screen;
}

void NotificationWidget::closeWidget()
{
    mCloseReason = CLOSE_REASON_CLOSED_BY_APP;
    emitClosed();
}

void NotificationWidget::start()
{
    setInputMask();
    if (mScreen == -1) {
        mScreen = QApplication::desktop()->screenNumber(QCursor::pos());
    }
    setGeometry(idealGeometry());
    show();
    mMousePollTimer->start();
    mState->onStarted();
}

static bool getShadowMargins(WId id, int* left, int* top, int* right, int* bottom)
{
    static Atom shadowAtom = XInternAtom( QX11Info::display(), "_KDE_NET_WM_SHADOW", False);
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    status = XGetWindowProperty(QX11Info::display(), id, shadowAtom, 0, 12, false, XA_CARDINAL, &type, &format, &nitems, &extra, &data);
    bool ok = false;
    if (status == Success && type == XA_CARDINAL && format == 32 && nitems == 12) {
        long* shadow = reinterpret_cast< long* >(data);
        *top = shadow[8];
        *right = shadow[9];
        *bottom = shadow[10];
        *left = shadow[11];
        ok = true;
    }
    if (data) {
        XFree(data);
    }
    return ok;
}

QRect NotificationWidget::idealGeometry() const
{
    QRect rect = QApplication::desktop()->availableGeometry(mScreen);
    QSize sh = mLayout->sizeHint(Qt::PreferredSize, QSizeF()).toSize();
    {
        qreal left, top, right, bottom;
        mBackgroundSvg->getMargins(left, top, right, bottom);
        sh.rwidth() += int(left + right);
        sh.rheight() += int(top + bottom);
    }
    {
        int left, top, right, bottom;
        if (getShadowMargins(winId(), &left, &top, &right, &bottom)) {
            rect.adjust(left, top, -right, -bottom);
        }
    }
    int left, top;
    if (mAlignment & Qt::AlignTop) {
        top = rect.top();
    } else if (mAlignment & Qt::AlignVCenter) {
        top = rect.top() + (rect.height() - sh.height()) / 2;
    } else {
        top = rect.bottom() - sh.height();
    }
    if (mAlignment & Qt::AlignLeft) {
        left = rect.left();
    } else if (mAlignment & Qt::AlignHCenter) {
        left = rect.left() + (rect.width() - sh.width()) / 2;
    } else {
        left = rect.right() - sh.width();
    }
    return QRect(QPoint(left, top), sh);
}

void NotificationWidget::updateOpacity()
{
    const qreal opacity = mFadeOpacity * mMouseOverOpacity;
    if (KWindowSystem::compositingActive()) {
        setWindowOpacity(opacity);
        setVisible(true);
    } else {
        setWindowOpacity(1.);
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
        mState->onMouseOver();
    } else if (wasOver && !isOver) {
        mState->onMouseLeave();
    }

    if (!qFuzzyCompare(mMouseOverOpacity, oldOpacity)) {
        updateOpacity();
    }
}

void NotificationWidget::emitClosed()
{
    emit closed(mId, mCloseReason);
#ifdef QUIT_AFTER_ONE
    QCoreApplication::exit();
#endif
}

} // namespace

#include "notificationwidget.moc"
