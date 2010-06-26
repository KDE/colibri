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
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QDBusReply>
#include <QVBoxLayout>

// KDE
#include <KAboutData>
#include <KPluginFactory>

// Local
#include "alignmentselector.h"
#include "config.h"
#include "ui_controlmodule.h"

static const char* DESCRIPTION = I18N_NOOP("Light notification system for KDE4");
static const char* VERSION = "0.1.0";

static const char* DBUS_INTERFACE = "org.freedesktop.Notifications";
static const char* DBUS_SERVICE = "org.freedesktop.Notifications";
static const char* DBUS_PATH = "/org/freedesktop/Notifications";

K_PLUGIN_FACTORY(ColibriModuleFactory, registerPlugin<Colibri::ControlModule>();)
K_EXPORT_PLUGIN(ColibriModuleFactory("kcmcolibri"))

namespace Colibri
{

ControlModule::ControlModule(QWidget* parent, const QVariantList&)
: KCModule(ColibriModuleFactory::componentData(), parent)
, mConfig(new Config)
, mUi(new Ui::ControlModule)
{
    KAboutData* about = new KAboutData(
        "colibri", 0, ki18n("Colibri"),
        VERSION, ki18n(DESCRIPTION),
        KAboutData::License_GPL,
        ki18n("(C) 2009 Aurélien Gâteau"),
        KLocalizedString(), 0, "agateau@kde.org");
    about->addAuthor(ki18n("Aurélien Gâteau"), KLocalizedString(), "agateau@kde.org");
    setAboutData(about);

    mUi->setupUi(this);

    addConfig(mConfig, this);

    connect(mUi->alignmentSelector, SIGNAL(changed(Qt::Alignment)),
        SLOT(updateUnmanagedWidgetChangeState()));

    QDBusServiceWatcher* watcher = new QDBusServiceWatcher(DBUS_SERVICE, QDBusConnection::sessionBus());
    connect(watcher, SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString&)),
        SLOT(updateStateInformation()));

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
    KCModule::load();
}

void ControlModule::save()
{
    mConfig->setAlignment(int(mUi->alignmentSelector->alignment()));
    mConfig->writeConfig();
    KCModule::save();
}

void ControlModule::defaults()
{
    KCModule::defaults();
#if KDE_IS_VERSION(4, 4, 0)
    mUi->alignmentSelector->setAlignment(Qt::Alignment(mConfig->defaultAlignmentValue()));
#else
    bool useDefaults = mConfig->useDefaults(true);
    mUi->alignmentSelector->setAlignment(Qt::Alignment(mConfig->alignment()));
    mConfig->useDefaults(useDefaults);
#endif
    updateUnmanagedWidgetChangeState();
}

void ControlModule::updateUnmanagedWidgetChangeState()
{
    int alignment = int(mUi->alignmentSelector->alignment());
    unmanagedWidgetChangeState(alignment != mConfig->alignment());
}

static QString getCurrentService()
{
    QDBusInterface iface(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE);
    if (!iface.isValid()) {
        return QString();
    }

    QDBusReply<QString> reply = iface.call("GetServerInformation");
    if (!reply.isValid()) {
        return QString();
    }
    return reply.value();
}

void ControlModule::updateStateInformation()
{
    QString service = getCurrentService();
    QString icon;
    QString text;
    if (service == "colibri") {
        icon = "dialog-ok";
        text = i18n("Colibri is currently running.");
    } else if (service.isEmpty()) {
        icon = "dialog-warning";
        text = i18n("No notification system is currently running.");
    } else {
        icon = "dialog-error";
        text = i18n("The current notification system is %1.", service);
    }
    mUi->stateIconLabel->setPixmap(KIcon(icon).pixmap(16, 16));
    mUi->stateTextLabel->setText(text);
}

} // namespace
