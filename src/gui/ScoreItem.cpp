/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include <QPainter>

#include "ScoreItem.h"

namespace Sonot {

ScoreItem::ScoreItem(const Score::Index& i,
                     const ScoreDocument::Index& j,
                     const QRectF& rect, const Note& n)
    : p_index      (i)
    , p_docIndex   (j)
    , p_type       (T_NOTE)
    , p_rect       (rect)
    , p_note       (n)
{
    p_updateNoteLayout();
}

ScoreItem::ScoreItem(const Score::Index& i, const ScoreDocument::Index& j, const QLineF& line)
    : p_index      (i)
    , p_docIndex   (j)
    , p_type       (T_BAR_SLASH)
    , p_rect       (line.x1(), line.y1(),
                     line.x2()-line.x1(), line.y2()-line.y1())
{

}

ScoreItem::ScoreItem(const Score::Index& i, const ScoreDocument::Index& j,
                     const QRectF& rect, const QString& text)
    : p_index      (i)
    , p_docIndex   (j)
    , p_type       (T_TEXT)
    , p_rect       (rect)
    , p_text       (text)
{

}

void ScoreItem::updateNote(const Note& n)
{
    if (type() == T_NOTE)
    {
        p_note = n;
        p_updateNoteLayout();
    }
}

void ScoreItem::p_updateNoteLayout()
{
    p_font.clear();
    p_staticText.clear();
    p_textPos.clear();

    QString string = note().toSpanishString();

    QFont f;
    f.setPointSizeF(p_rect.height());
    f.setBold(true);
    f.setItalic(false);
    p_font << f;

    p_staticText << QStaticText( string.left(1) );

    QFontMetricsF m(p_font[0]);
    QRectF rect = m.boundingRect(p_rect,
                                   Qt::AlignCenter
                                 | Qt::TextDontClip,
                                 p_staticText[0].text() );

    p_textPos << rect.topLeft();

    if (note().accidental() != 0)
    {
        p_textPos[0].rx() -= rect.width()*.2;

        f.setPointSizeF(f.pointSizeF()*.8);
        f.setBold(false);
        p_font << f;
        p_staticText << QStaticText( note().accidentalString() );
        QRectF rect2 = rect;
        rect2.moveLeft(p_textPos[0].x() + m.width(p_staticText[0].text()));
        QFontMetricsF m2(f);
        rect2.moveTop(rect.top() + m2.height()*.1 );
        p_textPos << rect2.topLeft();
    }

    while (!string.isEmpty() && !(string[0] == '\'' || string[0] == ','))
        string.remove(0,1);
    if (!string.isEmpty())
    {
        f = p_font[0];
        f.setBold(false);
        f.setItalic(true);
        p_font << f;
        p_staticText << QStaticText( string );
        QPointF pos = rect.topRight();
        pos.rx() = p_textPos[0].x() + rect.width()*.8;
        double s = rect.height() * .1;
        if (string.startsWith(','))
            pos.ry() += s;
        else
            pos.ry() -= s, pos.rx() -= s;
        p_textPos << pos;

    }
}

void ScoreItem::paint(QPainter& p)
{
    if (p_type == T_NOTE)
    {
        //QString t = note().toSpanishString();

        for (int i=0; i<p_staticText.size(); ++i)
        {
            p.setFont(p_font[i]);
            p.drawStaticText(p_textPos[i], p_staticText[i]);
        }
        /*p.drawText(p_rect_,
                     Qt::AlignVCenter
                   | (note().isSpace() ? Qt::AlignHCenter : Qt::AlignLeft)
                   | Qt::TextDontClip, t);*/
        //p.drawRect(p_rect_);

        //p.drawLine(p_rect_.center().x(), p_rect_.top(),
        //           p_rect_.center().x(), p_rect_.bottom());
    }
    else if (p_type == T_BAR_SLASH)
    {
        p.drawLine(p_rect.x(), p_rect.y(),
                   p_rect.bottomRight().x(), p_rect.bottomRight().y());
    }
    else if (p_type == T_TEXT)
    {
        QFont f(p.font());
        f.setPixelSize(p_rect.height());
        f.setBold(false);
        f.setItalic(true);
        p.setFont(f);
        p.drawText(p_rect, Qt::AlignLeft | Qt::AlignVCenter
                            | Qt::TextDontClip, p_text);
    }

}

} // namespace Sonot
