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

#include "SelectionMimeData.h"

#include "QProps/JsonInterfaceHelper.h"

#include "core/NoteStream.h"
#include "core/Notes.h"
#include "core/Bar.h"

namespace Sonot {

SelectionMimeData::SelectionMimeData()
    : QMimeData()
{

}


SelectionMimeData* SelectionMimeData::fromSelection(
        const Score::Selection &s)
{
    QProps::JsonInterfaceHelper json("SelectionMimeData");

    QJsonObject o;

    if (s.isSingleBar())
    {
        o.insert("bar", s.from().getBar().toJson());
        if (!s.isCompleteRow())
        {
            o.insert("col-start", (int)s.from().column());
            o.insert("col-end", (int)s.to().column());
        }
        if (!s.isCompleteColumn())
        {
            o.insert("row-start", (int)s.from().row());
            o.insert("row-end", (int)s.to().row());
        }
    }
    else if (s.isSingleStream())
    {
        if (s.isCompleteStream())
            o.insert("stream", s.from().getStream().toJson());
        else
        {
            NoteStream n = s.from().getStream();
            n.removeBars(0, s.from().bar());
            size_t ridx = s.to().bar() - s.from().bar() + 1;
            if (ridx < n.numBars())
                n.removeBars(ridx);
            o.insert("bars", n.toJson());
        }
    }
    else
    {
        if (s.isCompleteScore())
            o.insert("score", s.score()->toJson());
        else
        {
            auto sc = *s.score();
            sc.removeNoteStreams(0, s.from().stream());
            if (s.from().bar() > 0)
            {
                auto n = sc.noteStream(0);
                n.removeBars(0, s.from().bar());
                sc.setNoteStream(0, n);
            }
            size_t ridx = s.to().stream() - s.from().stream();
            if (ridx < sc.numNoteStreams())
                sc.removeNoteStreams(ridx);
            if (!sc.isEmpty())
            {
                auto& n = sc.noteStream(sc.numNoteStreams()-1);
                if (s.to().bar() + 1 != n.numBars())
                {
                    auto n = sc.noteStream(sc.numNoteStreams()-1);
                    ridx = s.to().bar() + 1;
                    if (ridx < n.numBars())
                        n.removeBars(ridx);
                    sc.setNoteStream(sc.numNoteStreams()-1, n);
                }
            }
            o.insert("score", sc.toJson());
        }
    }


    auto d = new SelectionMimeData();
    d->setData("text/plain", QJsonDocument(o).toJson());

    return d;
}


} // namespace Sonot
