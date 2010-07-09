// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Colibri: Light notification system for KDE4
Copyright 2010 Aurélien Gâteau <agateau@kde.org>

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
#ifndef VERSION_H
#define VERSION_H

static const char* DESCRIPTION = I18N_NOOP("Light notification system for KDE4");
static const char* VERSION = "0.2.0";

KAboutData* createAboutData()
{
    KAboutData* about = new KAboutData(
        "colibri", 0, ki18n("Colibri"),
        VERSION, ki18n(DESCRIPTION),
        KAboutData::License_GPL,
        ki18n("(C) 2009-2010 Aurélien Gâteau"),
        KLocalizedString(), 0, "agateau@kde.org");
    about->addAuthor(ki18n("Aurélien Gâteau"), KLocalizedString(), "agateau@kde.org");
    return about;
}

#endif /* VERSION_H */
