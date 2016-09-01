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

#include <QApplication>
#include "gui/MainWindow.h"

#if 1
#include <QJsonObject>
#include "core/NoteStream.h"
#include <iostream>
void testNoteStream()
{
    using namespace Sonot;

    NoteStream s;

    for (int i=0; i<4; ++i)
    {
        Bar b(4, 3+rand()%2);
        b.setNote(0, 0, Note(Note::C+i*2));
        b.setNote(1, 0, Note(Note::D));
        b.setNote(2, 0, Note(Note::Space));
        b.setNote(3, 0, Note(Note::F).setAnnotation("3,0"));
        b.setNote(i, rand()%b.numRows(), Note(Note::C+rand()%12));
        b.setNote(i, rand()%b.numRows(), Note(Note::C+rand()%12));

        s.appendBar(b);
    }

    NoteStream s2;
    s2.fromJson(s.toJson());

    std::cout
              << s.toJsonString().toStdString()
              << "\n" << s.toString().toStdString()
              << "\n" << s2.toString().toStdString()
              << std::endl;


}
#endif

int main(int argc, char *argv[])
{
    //testNoteStream(); return 0;

    QApplication a(argc, argv);
    Sonot::MainWindow w;
    w.show();

    return a.exec();
}
