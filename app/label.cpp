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
#include <label.moc>

// Qt
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextLayout>

// KDE
#include <Plasma/PaintUtils>

namespace Colibri
{

Label::Label(QWidget* parent)
: QWidget(parent)
, mDocument(new QTextDocument(this))
{
    mDocument->setDocumentMargin(0);

    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    setFont(theme->font(Plasma::Theme::DefaultFont));
    QColor textColor = theme->color(Plasma::Theme::TextColor);
    mDocument->setDefaultStyleSheet(QString("body { color: %1; }").arg(textColor.name()));
}

void Label::setText(const QString& text)
{
    mDocument->setHtml("<body>" + text + "</body>");
    mDocument->adjustSize();
    mHaloRects.clear();
    for (QTextBlock block = mDocument->begin(); block.isValid(); block = block.next()) {
        QTextLayout* layout = block.layout();
        for (int i = 0; i < layout->lineCount(); ++i) {
            QTextLine line = layout->lineAt(i);
            if (line.textLength() > 0) {
                mHaloRects << line.naturalTextRect()
                    .translated(layout->position().toPoint())
                    .translated(HaloMargin, HaloMargin);
            }
        }
    }
    updateGeometry();
    update();
}

QSize Label::minimumSizeHint() const
{
    return mDocument->size().toSize() + QSize(2 * HaloMargin, 2 * HaloMargin);
}

void Label::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    if (Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).value() < 128) {
        Q_FOREACH(const QRectF &rect, mHaloRects) {
            Plasma::PaintUtils::drawHalo(&painter, rect);
        }
    }
    painter.translate(HaloMargin, HaloMargin);
    mDocument->drawContents(&painter, event->rect());
}

} // namespace
