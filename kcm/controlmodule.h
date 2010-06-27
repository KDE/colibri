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
#ifndef COLIBRIMODULE_H
#define COLIBRIMODULE_H

// KDE
#include <KCModule>

namespace Ui
{
class ControlModule;
}

namespace Colibri
{
class Config;
class AlignmentSelector;

class ControlModule : public KCModule
{
    Q_OBJECT
public:
    ControlModule(QWidget*, const QVariantList&);
    ~ControlModule();

public Q_SLOTS:
    virtual void load();
    virtual void save();
    virtual void defaults();

private Q_SLOTS:
    void updateUnmanagedWidgetChangeState();
    void updateStateInformation();
    void startColibri();
    void preview();
    void fillScreenComboBox();

private:
    Config* mConfig;
    Ui::ControlModule* mUi;
    uint mLastPreviewId;
};

} // namespace

#endif /* COLIBRIMODULE_H */
