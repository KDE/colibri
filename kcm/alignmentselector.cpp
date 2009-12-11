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
#include "alignmentselector.h"

// Qt
#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QPainter>
#include <QRadioButton>

// KDE
#include <KDebug>
#include <Plasma/FrameSvg>

namespace Colibri
{

static void createButton(QGridLayout* layout, QButtonGroup* group, Qt::Alignment alignment)
{
    QRadioButton* button = new QRadioButton;
    group->addButton(button, int(alignment));

    int row;
    int col;

    if (alignment & Qt::AlignTop) {
        row = 0;
    } else if (alignment & Qt::AlignVCenter) {
        row = 1;
    } else {
        row = 2;
    }
    if (alignment & Qt::AlignLeft) {
        col = 0;
    } else if (alignment & Qt::AlignHCenter) {
        col = 1;
    } else {
        col = 2;
    }

    layout->addWidget(button, row, col, alignment);
}

AlignmentSelector::AlignmentSelector(QWidget* parent)
: QWidget(parent)
, mButtonGroup(new QButtonGroup(this))
, mScreenSvg(new Plasma::FrameSvg(this))
{
    mScreenSvg->setImagePath("widgets/monitor");
    const int topHeight = mScreenSvg->marginSize(Plasma::TopMargin);
    const int leftWidth = mScreenSvg->marginSize(Plasma::LeftMargin);
    const int rightWidth = mScreenSvg->marginSize(Plasma::RightMargin);
    const int bottomHeight = mScreenSvg->marginSize(Plasma::BottomMargin);
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    /*
    QDesktopWidget* desktop = QApplication::desktop();
    QRect rect = desktop->availableGeometry(desktop->screenNumber(this));
    setRatio(qreal(rect.width()) / rect.height());
    */

    QGridLayout* layout = new QGridLayout(this);

    createButton(layout, mButtonGroup, Qt::AlignTop | Qt::AlignLeft);
    createButton(layout, mButtonGroup, Qt::AlignTop | Qt::AlignHCenter);
    createButton(layout, mButtonGroup, Qt::AlignTop | Qt::AlignRight);
    createButton(layout, mButtonGroup, Qt::AlignVCenter | Qt::AlignLeft);
    createButton(layout, mButtonGroup, Qt::AlignVCenter | Qt::AlignRight);
    createButton(layout, mButtonGroup, Qt::AlignBottom | Qt::AlignLeft);
    createButton(layout, mButtonGroup, Qt::AlignBottom | Qt::AlignHCenter);
    createButton(layout, mButtonGroup, Qt::AlignBottom | Qt::AlignRight);
}

void AlignmentSelector::setAlignment(Qt::Alignment alignment)
{
    QAbstractButton* button = mButtonGroup->button(int(alignment));
    if (!button) {
        kWarning() << "No button for alignment" << alignment;
        return;
    }
    button->setChecked(true);
}

Qt::Alignment AlignmentSelector::alignment() const
{
    return Qt::Alignment(mButtonGroup->checkedId());
}

void AlignmentSelector::resizeEvent(QResizeEvent*)
{
    mScreenSvg->resizeFrame(size());
}

void AlignmentSelector::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    mScreenSvg->paintFrame(&painter);
}

} // namespace

#include "alignmentselector.moc"
