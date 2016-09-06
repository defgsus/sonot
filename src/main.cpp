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
#include "core/Score.h"
#include <iostream>
void testNoteStream()
{
    using namespace Sonot;

    NoteStream s;

    for (int i=0; i<4; ++i)
    {
        Bar b(4+rand()%2, 3+rand()%2);
        b.setNote(0, 0, Note(Note::C+i*2));
        b.setNote(1, 0, Note(Note::D));
        b.setNote(2, 0, Note(Note::Space));
        b.setNote(3, 1, Note(Note::F).setAnnotation("3,1"));
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

    Score score, score2;
    score.setTitle("Amazing Haze");
    score.setAuthor("Convenieous Bar");
    score.setCopyright("(c) 1964");
    score.setProperty("version", 23);
    score.appendNoteStream(s);
    score.appendNoteStream(s2);
    score2.fromJson(score.toJson());

    std::cout << "\n" << score.toJsonString().toStdString()
              << "\n" << score2.noteStream(0).toString().toStdString()
              << std::endl;

    auto idx = score.index(0,0,0,0);
    do
    {
        std::cout << "," << idx.getNote().toNoteString().toStdString();
    } while (idx.nextNote());
    std::cout << std::endl;
    do
    {
        std::cout << "," << idx.getNote().toNoteString().toStdString();
    } while (idx.prevNote());
    std::cout << std::endl;
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
