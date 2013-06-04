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
#include <KIconLoader>
#include <KUrl>

// Local
#include <config.h>
#include <notificationsadaptor.h>
#include <notificationwidget.h>

namespace Colibri
{

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

NotificationManager::NotificationManager()
: mNextId(1)
, mConfig(new Config)
{
    new NotificationsAdaptor(this);
}

bool NotificationManager::connectOnDBus()
{
    bool ok;
    QDBusConnection connection = QDBusConnection::sessionBus();
    ok = connection.registerObject("/org/freedesktop/Notifications", this);
    if (!ok) {
        kWarning() << "Could not register object /org/freedesktop/Notifications";
        return false;
    }
    ok = connection.registerService("org.freedesktop.Notifications");
    if (!ok) {
        kWarning() << "Could not register service org.freedesktop.Notifications";
        return false;
    }
    kDebug() << "Registered";
    return true;
}

NotificationManager::~NotificationManager()
{
    delete mConfig;
}

inline void copyLineRGB32(QRgb* dst, const char* src, int width)
{
    const char* end = src + width * 3;
    for (; src != end; ++dst, src+=3) {
        *dst = qRgb(src[0], src[1], src[2]);
    }
}

inline void copyLineARGB32(QRgb* dst, const char* src, int width)
{
    const char* end = src + width * 4;
    for (; src != end; ++dst, src+=4) {
        *dst = qRgba(src[0], src[1], src[2], src[3]);
    }
}

static QImage decodeNotificationSpecImageHint(const QDBusArgument& arg)
{
    int width, height, rowStride, hasAlpha, bitsPerSample, channels;
    QByteArray pixels;
    char* ptr;
    char* end;

    arg.beginStructure();
    arg >> width >> height >> rowStride >> hasAlpha >> bitsPerSample >> channels >> pixels;
    arg.endStructure();
    //kDebug() << width << height << rowStride << hasAlpha << bitsPerSample << channels;

    #define SANITY_CHECK(condition) \
    if (!(condition)) { \
        kWarning() << "Sanity check failed on" << #condition; \
        return QImage(); \
    }

    SANITY_CHECK(width > 0);
    SANITY_CHECK(width < 2048);
    SANITY_CHECK(height > 0);
    SANITY_CHECK(height < 2048);
    SANITY_CHECK(rowStride > 0);

    #undef SANITY_CHECK

    QImage::Format format = QImage::Format_Invalid;
    void (*fcn)(QRgb*, const char*, int) = 0;
    if (bitsPerSample == 8) {
        if (channels == 4) {
            format = QImage::Format_ARGB32;
            fcn = copyLineARGB32;
        } else if (channels == 3) {
            format = QImage::Format_RGB32;
            fcn = copyLineRGB32;
        }
    }
    if (format == QImage::Format_Invalid) {
        kWarning() << "Unsupported image format (hasAlpha:" << hasAlpha << "bitsPerSample:" << bitsPerSample << "channels:" << channels << ")";
        return QImage();
    }

    QImage image(width, height, format);
    ptr = pixels.data();
    end = ptr + pixels.length();
    for (int y=0; y<height; ++y, ptr += rowStride) {
        if (ptr + channels * width > end) {
            kWarning() << "Image data is incomplete. y:" << y << "height:" << height;
            break;
        }
        fcn((QRgb*)image.scanLine(y), ptr, width);
    }

    return image;
}

static QString findImageForSpecImagePath(const QString &_path)
{
    QString path = _path;
    if (path.startsWith("file:")) {
        KUrl url(path);
        path = url.toLocalFile();
    }
    return KIconLoader::global()->iconPath(path, -KIconLoader::SizeHuge,
                                           true /* canReturnNull */);
}

static int timeoutForText(const QString& text)
{
    const int AVERAGE_WORD_LENGTH = 6;
    const int WORD_PER_MINUTE = 250;
    return 1000 + 60000 * text.length() / AVERAGE_WORD_LENGTH / WORD_PER_MINUTE;
}

uint NotificationManager::Notify(const QString& appName, uint replacesId, const QString& appIcon, const QString& summary, const QString& body, const QStringList& /*actions*/, const QVariantMap& hints, int /*timeout*/)
{
    NotificationWidget* widget = findWidget(appName, summary);
    QString cBody = cleanBody(body);
     // Block already existing notifications
    if (widget && widget->summary() == summary && widget->body() == cBody) {
        return replacesId;
    }

    if (replacesId > 0) {
        CloseNotification(replacesId);
    }

    // Can we append to an existing notification?
    widget = findWidget(appName, summary);
    if (widget && widget->summary() == summary && !body.isEmpty()) {
        int timeout = timeoutForText(body);
        widget->appendToBody(cBody, timeout);
        return widget->id();
    }

    QPixmap pix;

    // image
    QImage image;
    if (hints.contains("image_data")) {
        QDBusArgument arg = hints["image_data"].value<QDBusArgument>();
        image = decodeNotificationSpecImageHint(arg);
    } else if (hints.contains("image_path")) {
        QString path = findImageForSpecImagePath(hints["image_path"].toString());
        if (!path.isEmpty()) {
            image.load(path);
        }
    } else if (hints.contains("icon_data")) {
        // This hint was in use in version 1.0 of the spec but has been
        // replaced by "image_data" in version 1.1. We need to support it for
        // users of the 1.0 version of the spec.
        QDBusArgument arg = hints["icon_data"].value<QDBusArgument>();
        image = decodeNotificationSpecImageHint(arg);
    }

    int timeout = qBound(2000, timeoutForText(summary + body), 20000);

    // Id
    uint id = mNextId++;

    // Create widget
    widget = new NotificationWidget(appName, id, image, appIcon, summary, cBody, timeout);

    // Update config, KCM may have changed it
    mConfig->readConfig();
    widget->setAlignment(Qt::Alignment(mConfig->alignment()));
    widget->setScreen(mConfig->screen());
    connect(widget, SIGNAL(closed(uint, uint)), SLOT(slotNotificationWidgetClosed(uint, uint)));
    mWidgets << widget;
    if (mWidgets.size() == 1) {
        widget->start();
    }
    kDebug() << "id:" << id << "app:" << appName << "summary:" << summary << "timeout:" << timeout;
    kDebug() << "body:" << body;;
    return id;
}

void NotificationManager::CloseNotification(uint id)
{
    Q_FOREACH(NotificationWidget* widget, mWidgets) {
        if (widget->id() == id) {
            widget->closeWidget();
            return;
        }
    }
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

void NotificationManager::slotNotificationWidgetClosed(uint id, uint reason)
{
    NotificationClosed(id, reason);

    if (mWidgets.isEmpty()) {
        kWarning() << "mWidgets should not be empty!";
        return;
    }
    NotificationWidget* widget = mWidgets.takeFirst();

    // Hack to workaround blinking when the notification is fading out
    // See https://bugs.kde.org/show_bug.cgi?id=314427
    widget->move(-widget->width(), 0);
    widget->deleteLater();

    if (!mWidgets.isEmpty()) {
        mWidgets.first()->start();
    }
}

NotificationWidget* NotificationManager::findWidget(const QString& appName, const QString& summary)
{
    Q_FOREACH(NotificationWidget* widget, mWidgets) {
        if (widget->appName() == appName && widget->summary() == summary) {
            return widget;
        }
    }
    return 0;
}

} // namespace
