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
#include "controlmodule.moc"

// Qt
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QDBusReply>
#include <QDesktopWidget>
#include <QTimer>
#include <QVBoxLayout>

// KDE
#include <KAboutData>
#include <KColorScheme>
#include <KPluginFactory>
#include <KProcess>

// Local
#include "alignmentselector.h"
#include "config.h"
#include "ui_controlmodule.h"
#include "about.h"

static const char* DBUS_INTERFACE = "org.freedesktop.Notifications";
static const char* DBUS_SERVICE = "org.freedesktop.Notifications";
static const char* DBUS_PATH = "/org/freedesktop/Notifications";

K_PLUGIN_FACTORY(ColibriModuleFactory, registerPlugin<Colibri::ControlModule>();)
K_EXPORT_PLUGIN(ColibriModuleFactory("kcmcolibri", "colibri"))

namespace Colibri
{

ControlModule::ControlModule(QWidget* parent, const QVariantList&)
: KCModule(ColibriModuleFactory::componentData(), parent)
, mConfig(new Config)
, mUi(new Ui::ControlModule)
, mLastPreviewId(0)
{
    KGlobal::locale()->insertCatalog("colibri");

    KAboutData* about = createAboutData();
    setAboutData(about);

    mUi->setupUi(this);
    // Fix height so that it does not grow or shrink depending on the visibility of the "Start" button
    mUi->stateContainer->setFixedHeight(mUi->stateContainer->sizeHint().height());

    addConfig(mConfig, this);

    connect(mUi->alignmentSelector, SIGNAL(changed(Qt::Alignment)),
        SLOT(updateUnmanagedWidgetChangeState()));

    connect(mUi->startButton, SIGNAL(clicked()),
        SLOT(startColibri()));

    connect(mUi->previewButton, SIGNAL(clicked()),
        SLOT(preview()));

    QDBusServiceWatcher* watcher = new QDBusServiceWatcher(DBUS_SERVICE, QDBusConnection::sessionBus());
    connect(watcher, SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString&)),
        SLOT(updateStateInformation()));

    fillScreenComboBox();
    updateStateInformation();
}

ControlModule::~ControlModule()
{
    delete mConfig;
    delete mUi;
}

void ControlModule::load()
{
    mUi->alignmentSelector->setAlignment(Qt::Alignment(mConfig->alignment()));
    mUi->screenComboBox->setCurrentIndex(
        mUi->screenComboBox->findData(mConfig->screen())
        );
    KCModule::load();
}

void ControlModule::save()
{
    mConfig->setAlignment(int(mUi->alignmentSelector->alignment()));
    int screen = mUi->screenComboBox->itemData(mUi->screenComboBox->currentIndex()).toInt();
    mConfig->setScreen(screen);
    mConfig->writeConfig();
    KCModule::save();
}

void ControlModule::defaults()
{
    KCModule::defaults();
    mUi->alignmentSelector->setAlignment(Qt::Alignment(mConfig->defaultAlignmentValue()));
    mUi->screenComboBox->setCurrentIndex(
        mUi->screenComboBox->findData(mConfig->defaultScreenValue())
        );
    updateUnmanagedWidgetChangeState();
}

void ControlModule::updateUnmanagedWidgetChangeState()
{
    int alignment = int(mUi->alignmentSelector->alignment());
    unmanagedWidgetChangeState(alignment != mConfig->alignment());
}

static QString getCurrentService()
{
    QDBusConnectionInterface* interface = QDBusConnection::sessionBus().interface();
    if (!interface->isServiceRegistered(DBUS_SERVICE)) {
        return QString();
    }
    QDBusInterface iface(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE);
    if (!iface.isValid()) {
        kWarning() << "Invalid registered interface!";
        return QString();
    }

    QDBusReply<QString> reply = iface.call("GetServerInformation");
    if (!reply.isValid()) {
        kWarning() << "Failed to get server information:" << reply.error().message();
        return QString();
    }
    return reply.value();
}

void ControlModule::updateStateInformation()
{
    QString service = getCurrentService();
    QString icon;
    QString text;
    bool showStartButton = false;
    bool colibriIsRunning = false;
    if (service == "colibri") {
        icon = "dialog-ok";
        text = i18n("Colibri is running.");
        colibriIsRunning = true;
    } else {
        icon = "dialog-warning";
        if (service.isEmpty()) {
            text = i18n("No notification system is running.");
            showStartButton = true;
        } else {
            text = i18n("The current notification system is %1. You must stop it to be able to start Colibri.", service);
        }
    }

    // Adjust palette
    KColorScheme scheme(QPalette::Active, KColorScheme::Window);
    QBrush bg = scheme.background(colibriIsRunning ? KColorScheme::PositiveBackground : KColorScheme::NegativeBackground);
    QBrush fg = scheme.foreground(colibriIsRunning ? KColorScheme::PositiveText : KColorScheme::NegativeText);
    mUi->stateContainer->setStyleSheet(
        QString(".QFrame {"
            "background-color: %1;"
            "border-radius: 3px;"
            "border: 1px solid %2;"
            "}"
            ".QLabel { color: %2; }"
            )
        .arg(bg.color().name())
        .arg(fg.color().name())
        );

    mUi->stateIconLabel->setPixmap(KIcon(icon).pixmap(22));
    mUi->stateTextLabel->setText(text);
    mUi->startButton->setVisible(showStartButton);
    mUi->previewButton->setEnabled(colibriIsRunning);
    mUi->previewImpossibleLabel->setVisible(!colibriIsRunning);

    // Hide the stateContainer if Colibri is running. If we come from a
    // slot (ie, we came because the dbus service changed), hide it after a
    // delay. If we come from the constructor, hide it immediatly.
    if (colibriIsRunning) {
        if (sender()) {
            QTimer::singleShot(1000, mUi->stateContainer, SLOT(hide()));
        } else {
            mUi->stateContainer->hide();
        }
    } else {
        mUi->stateContainer->show();
    }
}

void ControlModule::startColibri()
{
    KProcess::startDetached("colibri");
}

void ControlModule::preview()
{
    save();

    QDBusInterface iface(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE);
    QDBusReply<uint> reply = iface.call(
        "Notify",
        "kcmcolibri",                       // appname
        mLastPreviewId,                     // replacesId
        "preferences-desktop-notification", // appIcon
        i18n("Preview"),                    // summary
        i18n("This is a preview of a Colibri notification"),
        QStringList(),                      // actions
        QVariantMap(),                      // hints
        -1                                  // timeout
        );

    if (reply.isValid()) {
        mLastPreviewId = reply.value();
    }
}

void ControlModule::fillScreenComboBox()
{
    mUi->screenComboBox->clear();
    mUi->screenComboBox->addItem(i18n("Screen under mouse"), -1);
    int count = QApplication::desktop()->screenCount();
    if (count > 1) {
        for (int screen=0; screen < count; ++screen) {
            mUi->screenComboBox->addItem(i18n("Screen %1", screen + 1), screen);
        }
    } else {
        mUi->screenComboBox->setEnabled(false);
    }
}

} // namespace
