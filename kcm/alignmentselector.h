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
#ifndef ALIGNMENTSELECTOR_H
#define ALIGNMENTSELECTOR_H

// Qt
#include <QWidget>

namespace Plasma {
class FrameSvg;
}

class QButtonGroup;

namespace Colibri
{

class AlignmentSelector : public QWidget
{
    Q_OBJECT

public:
    AlignmentSelector(QWidget* parent = 0);

    void setAlignment(Qt::Alignment);
    Qt::Alignment alignment() const;

protected:
    virtual void resizeEvent(QResizeEvent*);
    virtual void paintEvent(QPaintEvent*);

private:
    QWidget* mContent;
    QButtonGroup* mButtonGroup;
    Plasma::FrameSvg* mScreenSvg;
};

} // namespace

#endif /* ALIGNMENTSELECTOR_H */
