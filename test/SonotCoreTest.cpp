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
#include "QProps/error.h"

using namespace Sonot;

class SonotCoreTest : public QObject
{
    Q_OBJECT

public:
    SonotCoreTest() { }

    static Bar createRandomBar(size_t length);
    static QList<Bar> createRandomBar(size_t length, size_t rows);
    static Score createScoreForIndexTest();

private slots:

    void testNoteFromString();
    void testResize();
    void testRandomCursor();
    void testKeepDataOnResize();
    void testJsonBar();
    void testJsonStream();
    void testJsonScore();
    void testScoreIndexNextNote();
    void testScoreIndexPrevNote();
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

QList<Bar> SonotCoreTest::createRandomBar(size_t length, size_t rows)
{
    QList<Bar> l;
    for (size_t i=0; i<rows; ++i)
        l << createRandomBar(length);
    return l;
}

namespace QTest {

    template <>
    char* toString(const int8_t& i)
    {
        return toString(QString::number(i));
    }

    template <>
    char* toString(const Score::Index& idx)
    {
        return toString(idx.toString());
    }

    template <>
    char* toString(const Note& n)
    {
        return toString(n.to3String());
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



void SonotCoreTest::testNoteFromString()
{
#define SONOT__COMPARE(str__, N__, oct__) \
    QCOMPARE(Note(str__),       Note(Note::N__, 3+(oct__))); \
    QCOMPARE(Note(str__ "4"),   Note(Note::N__, 4        )); \
    QCOMPARE(Note(str__ "-5"),  Note(Note::N__, 5        )); \
    QCOMPARE(Note(str__ ",,"),  Note(Note::N__, 1+(oct__))); \
    QCOMPARE(Note(str__ "'''"), Note(Note::N__, 6+(oct__)));
    //QCOMPARE(Note(str__).value(), int8_t(Note::N__ + 3 * 12));
    //qDebug() << Note(str__).toNoteString() << Note(Note::N__, 3).toNoteString();

    SONOT__COMPARE("C",     C   , 0);

    SONOT__COMPARE("ces",   Ces , 0);
    SONOT__COMPARE("c",     C   , 0);
    SONOT__COMPARE("cis",   Cis , 0);
    SONOT__COMPARE("des",   Des , 0);
    SONOT__COMPARE("d",     D   , 0);
    SONOT__COMPARE("dis",   Dis , 0);
    SONOT__COMPARE("es",    Es  , 0);
    SONOT__COMPARE("e",     E   , 0);
    SONOT__COMPARE("eis",   Eis , 0);
    SONOT__COMPARE("fes",   Fes , 0);
    SONOT__COMPARE("f",     F   , 0);
    SONOT__COMPARE("fis",   Fis , 0);
    SONOT__COMPARE("ges",   Ges , 0);
    SONOT__COMPARE("g",     G   , 0);
    SONOT__COMPARE("gis",   Gis , 0);
    SONOT__COMPARE("as",    As  , 0);
    SONOT__COMPARE("a",     A   , 0);
    SONOT__COMPARE("ais",   Ais , 0);
    SONOT__COMPARE("bes",   Bes , 0);
    SONOT__COMPARE("b",     B   , 0);
    SONOT__COMPARE("bis",   Bis , 0);
    SONOT__COMPARE("h",     B   , 0);

    SONOT__COMPARE("1b",    E   , 0);
    SONOT__COMPARE("1",     F   , 0);
    SONOT__COMPARE("1x",    Fis , 0);
    SONOT__COMPARE("2b",    Fis , 0);
    SONOT__COMPARE("2",     G   , 0);
    SONOT__COMPARE("2x",    Gis , 0);
    SONOT__COMPARE("3b",    Gis , 0);
    SONOT__COMPARE("3",     A   , 0);
    SONOT__COMPARE("3x",    Ais , 0);
    SONOT__COMPARE("4b",    Ais , 0);
    SONOT__COMPARE("4",     B   , 0);
    SONOT__COMPARE("5",     C   , 1);
    SONOT__COMPARE("5x",    Cis , 1);
    SONOT__COMPARE("6b",    Cis , 1);
    SONOT__COMPARE("6",     D   , 1);
    SONOT__COMPARE("6x",    Dis , 1);
    SONOT__COMPARE("7b",    Dis , 1);
    SONOT__COMPARE("7",     E   , 1);
    SONOT__COMPARE("7x",    F   , 1);
}


void SonotCoreTest::testResize()
{
    Bar bar(2);

    QCOMPARE(bar.length(), size_t(2));

    bar.resize(3);

    QCOMPARE(bar.length(), size_t(3));
}

/** Generates random score (WITH EMPTY STREAMS/BARS!!)
    and verifies that Score::Index iterators always
    lead to valid positions. */
void SonotCoreTest::testRandomCursor()
{
    for (int iter=0; iter<500; ++iter)
    {
        Score s;
        for (int i=0; i<5 + rand()%20; ++i)
        {
            NoteStream n;
            int jc = i == 0 ? 4 : (rand()%20);
            for (int j=0; j<jc; ++j)
            {
                QList<Bar> rows;
                int kc = i == 0 ? 4 : (rand()%10);
                for (int k=0; k<kc; ++k)
                    rows << createRandomBar(i == 0 ? 4 : (rand()%17));
                n.appendBar(rows);
            }
            //qDebug().noquote() << ("\n" + n.toInfoString());
            s.appendNoteStream(n);
        }

        auto c = s.index(0,0,0,0), cprev = c;
        QVERIFY(c.isValid());

        for (int i=0; i<1000; ++i)
        {
            QString cmd;
            try
            {
                cprev = c;
                switch (rand()%8)
                {
                    case 0: c.nextBar(); cmd = "nextBar"; break;
                    case 1: c.prevBar(); cmd = "prevBar"; break;
                    case 2: c.nextNote(); cmd = "nextNote"; break;
                    case 3: c.prevNote(); cmd = "prevNote"; break;
                    case 4: c.nextRow(); cmd = "nextRow"; break;
                    case 5: c.prevRow(); cmd = "prevRow"; break;
                    case 6: c.nextStream(); cmd = "nextStream"; break;
                    case 7: c.prevStream(); cmd = "prevStream"; break;
                }
            }
            catch (const QProps::Exception& e)
            {
                qDebug().noquote() << "EXCEPTION" << e.what();
            }
            // test validity of cursor after each step
            if (!c.isValid())
                qDebug().noquote()
                    << "random cursor movement" << cmd << "failed: "
                    << "from" << cprev.toString()
                    << "to" << c.toString()
                    << (QString("\nstream %1\n").arg(cprev.stream())
                        + s.noteStream(cprev.stream()).toInfoString())
                    << (QString("\nstream %1\n").arg(c.stream())
                        + s.noteStream(c.stream()).toInfoString());
            QVERIFY(c.isValid());
        }
    }
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
        for (int j=0; j<5 + rand()%20; ++j)
            stream.appendBar( createRandomBar(rand()%4 + 4, i+1) );
        score1.appendNoteStream(stream);
    }
    score1.setTitle("Amazing Haze");
    score1.setAuthor("Convenieous Bar");
    score1.setCopyright("(c) 1964");
    score1.props().set("version", 23);

    score2.fromJsonString(score1.toJsonString());
    QCOMPARE(score1, score2);
    //qDebug() << score1.noteStream(0).toString();
    //qDebug() << score2.toJsonString();
}


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
    // xxxx xxx xxxxx  x  xxxxx xxxxxxxx xxx
    // xxxx x   xxxxx  x  x     xxxxxxxx x
}

void SonotCoreTest::testScoreIndexPrevNote()
{
    Score score = createScoreForIndexTest();
    Score::Index idx;
    try
    {
        idx = score.index(1,3,0,2);
        QVERIFY(idx.isValid());
        int cnt = 1;
        while (idx.prevNote()) ++cnt;
        QVERIFY(idx.isValid());
        QCOMPARE(idx, score.index(0, 0, 0, 0));
        QCOMPARE(cnt, 30);

        idx = score.index(1, 3, 1, 2);
        QVERIFY(idx.isValid());
        cnt = 1;
        while (idx.prevNote()) ++cnt;
        QVERIFY(idx.isValid());
        QCOMPARE(idx, score.index(0,0,1,0));
        QCOMPARE(cnt, 29);

        idx = score.index(1, 3, 2, 0);
        QVERIFY(idx.isValid());
        cnt = 1;
        while (idx.prevNote()) ++cnt;
        QVERIFY(idx.isValid());
        QCOMPARE(idx, score.index(0,0,2,0));
        QCOMPARE(cnt, 21);
    }
    catch (const QProps::Exception& e)
    {
        qDebug().noquote() << "idx = " << idx.toString()
                           << (QString("\n") + e.what());
        QFAIL("idx got invalid");
    }
}

void SonotCoreTest::testScoreIndexNextNote()
{
    Score score = createScoreForIndexTest();

    Score::Index idx = score.index(0,0,0,0);
    QVERIFY(idx.isValid());
    int cnt = 1;
    while (idx.nextNote()) ++cnt;
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(1, 3, 0, 2));
    QCOMPARE(cnt, 30);

    idx = score.index(0,0,1,0);
    QVERIFY(idx.isValid());
    cnt = 1;
    while (idx.nextNote()) ++cnt;
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(1, 3, 1, 2));
    QCOMPARE(cnt, 29);

    idx = score.index(0,0,2,0);
    QVERIFY(idx.isValid());
    cnt = 1;
    while (idx.nextNote()) ++cnt;
    QVERIFY(idx.isValid());
    QCOMPARE(idx, score.index(1, 3, 2, 0));
    QCOMPARE(cnt, 21);
}






QTEST_APPLESS_MAIN(SonotCoreTest)

#include "SonotCoreTest.moc"
