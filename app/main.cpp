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
// KDE
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

// Locale
#include "notificationmanager.h"

static const char description[] =
    I18N_NOOP("Light notification system for KDE4");

static const char version[] = "0.1.0";

int main(int argc, char **argv)
{
    KAboutData about("colibri", 0, ki18n("Colibri"),
                     version, ki18n(description),
                     KAboutData::License_GPL,
                     ki18n("(C) 2009 Aurélien Gâteau"),
                     KLocalizedString(), 0, "agateau@kde.org");
    about.addAuthor(ki18n("Aurélien Gâteau"), KLocalizedString(), "agateau@kde.org");
    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;
    Colibri::NotificationManager manager;
    return app.exec();
}
