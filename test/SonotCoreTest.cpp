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

#include <QString>
#include <QtTest>

#include "core/NoteStream.h"
#include "core/Score.h"
#include "QProps/Properties.h"

using namespace Sonot;

class SonotCoreTest : public QObject
{
    Q_OBJECT

public:
    SonotCoreTest() { }

    static Bar createRandomBar(size_t length);
    //static Score createScoreForIndexTest();

private slots:

    void testResize();
    void testKeepDataOnResize();
    void testJsonBar();
    void testJsonStream();
    void testJsonScore();
    //void testScoreIndexNextNote();
    //void testScoreIndexPrevNote();
};

Bar SonotCoreTest::createRandomBar(size_t length)
{
    QStringList anno;
    anno << "bladiblub" << "annotation" << "anno 1900"
         << "andante" << "piano forte";
    Bar b(length);
    for (size_t x = 0; x < length; ++x)
    {
        if (rand()%10 <= 7)
        {
            Note note = Note(rand()%128);
            if (rand()%10 < 2)
                note.setAnnotation(anno[rand()%anno.size()]);
            b.setNote(x, note);
        }
    }
    return b;
}

namespace QTest {

    template <>
    char* toString(const Score::Index& idx)
    {
        return toString(idx.toString());
    }

    template <>
    char* toString(const QVariant::Type& t)
    {
        return toString(QVariant::typeToName(t));
    }

    template <>
    char* toString(const NoteStream& s)
    {
        return toString(s.toJsonString());
    }

} // namespace QTest

void SonotCoreTest::testResize()
{
    Bar bar(2);

    QCOMPARE(bar.length(), size_t(2));

    bar.resize(3);

    QCOMPARE(bar.length(), size_t(3));
}

void SonotCoreTest::testKeepDataOnResize()
{
    Bar bar1(2), bar2(5);
    bar1.setNote(0, Note(Note::A));
    bar1.setNote(1, Note(Note::B));
    bar2.setNote(0, Note(Note::A));
    bar2.setNote(1, Note(Note::B));

    bar1.resize(5);

    QCOMPARE(bar1, bar2);
}



void SonotCoreTest::testJsonBar()
{
    Bar bar2, bar1 = createRandomBar(8);

    bar2.fromJsonString(bar1.toJsonString());
    QCOMPARE(bar1, bar2);
}

void SonotCoreTest::testJsonStream()
{
    NoteStream stream1, stream2;
    for (int i=0; i<5; ++i)
        stream1.appendBar( createRandomBar(rand()%4 + 4) );

    stream2.fromJsonString(stream1.toJsonString());
    QCOMPARE(stream1, stream2);
}

void SonotCoreTest::testJsonScore()
{
    Score score1, score2;
    for (int i=0; i<5; ++i)
    {
        NoteStream stream;
        for (int j=0; j<5; ++j)
            stream.appendBar( createRandomBar(rand()%4 + 4) );
        score1.appendNoteStream(stream);
    }
    score1.setTitle("Amazing Haze");
    score1.setAuthor("Convenieous Bar");
    score1.setCopyright("(c) 1964");
    score1.props().set("version", 23);

    score2.fromJsonString(score1.toJsonString());
    QCOMPARE(score1, score2);
    //qDebug() << score2.toJsonString();
}

#if 0
Score SonotCoreTest::createScoreForIndexTest()
{
    Score score;
    {
        NoteStream stream;
        stream.appendBar( createRandomBar(4,3) );
        stream.appendBar( createRandomBar(3,2) );
        stream.appendBar( createRandomBar(5,3) );
        score.appendNoteStream(stream);
    }
    {
        NoteStream stream;
        stream.appendBar( createRandomBar(2,1) );
        stream.appendBar( createRandomBar(5,2) );
        stream.appendBar( createRandomBar(8,3) );
        stream.appendBar( createRandomBar(3,2) );
        score.appendNoteStream(stream);
    }
    return score;
    // xxxx xxx xxxxx  xx xxxxx xxxxxxxx xxx
    // xxxx xxx xxxxx     xxxxx xxxxxxxx xxx
    // xxxx     xxxxx           xxxxxxxx
}

void SonotCoreTest::testScoreIndexPrevNote()
{
    Score score = createScoreForIndexTest();

    Score::Index idx = score.index(1,3,0,2);
    QVERIFY(idx.isValid());
    while (idx.prevNote());
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(0, 0, 0, 0));

    idx = score.index(1,3,1,2);
    QVERIFY(idx.isValid());
    while (idx.prevNote());
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(1, 1, 1, 0));

    idx = score.index(0,2,2,4);
    QVERIFY(idx.isValid());
    while (idx.prevNote());
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(0, 2, 2, 0));
}

void SonotCoreTest::testScoreIndexNextNote()
{
    Score score = createScoreForIndexTest();

    Score::Index idx = score.index(0,0,0,0);
    QVERIFY(idx.isValid());
    while (idx.nextNote());
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(1, 3, 0, 2));

    idx = score.index(0,0,2,0);
    QVERIFY(idx.isValid());
    while (idx.nextNote());
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(0, 0, 2, 3));

    idx = score.index(0,0,1,0);
    QVERIFY(idx.isValid());
    while (idx.nextNote());
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(0, 2, 1, 4));
}
#endif







QTEST_APPLESS_MAIN(SonotCoreTest)

#include "SonotCoreTest.moc"
