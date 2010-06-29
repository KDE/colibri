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
#ifndef LABEL_H
#define LABEL_H

// Qt
#include <QWidget>

class QPaintEvent;
class QTextDocument;

namespace Colibri
{

/**
 * A label with Plasma-like halo behind its text
 */
class Label : public QWidget
{
    Q_OBJECT
public:
    // Must match the margin defined in HaloPainter
    // (kdelibs/plasma/private/effects/halopainter.cpp)
    enum { HaloMargin = 8 };

    Label(QWidget* parent=0);

    void setText(const QString& text);

    virtual QSize minimumSizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent*);

private:
    QTextDocument* mDocument;
    QList<QRectF> mHaloRects;
};

} // namespace

#endif /* LABEL_H */
