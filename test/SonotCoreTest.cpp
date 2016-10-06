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

#include "core/KeySignature.h"
#include "core/NoteStream.h"
#include "core/Score.h"
#include "core/ScoreEditor.h"
#include "core/ExportMusicXML.h"
#include "QProps/Properties.h"
#include "QProps/error.h"

using namespace Sonot;

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
        return toString(n.toString());
    }

    template <>
    char* toString(const Notes& n)
    {
        return toString(n.toString());
    }

    template <>
    char* toString(const Bar& n)
    {
        return toString(n.toString());
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

    template <>
    char* toString(const Score& s)
    {
        return toString(s.toInfoString());
    }

} // namespace QTest


class SonotCoreTest : public QObject
{
    Q_OBJECT

public:
    SonotCoreTest() { }

    static Note createRandomNote();
    static Notes createRandomNotes(size_t length);
    static Bar createRandomBar(size_t length, size_t rows);
    static Score createRandomScore(int mi=1, int ma=20, int numStreams=9);
    static Score createScoreForIndexTest();
    static Score::Index getRandomIndex(const Score* score);
    static bool compare(const Score& a, const Score& b);
    static bool makeRandomEditorAction(ScoreEditor& editor);

private slots:

    void testNoteFromString();
    void testNoteFromValue();
    void testNoteTranspose();
    void testKeySignature();
    void testResize();
    void testRandomCursor();
    void testKeepDataOnResize();
    void testJsonNotes();
    void testJsonStream();
    void testJsonScore();
    void testScoreIndexNextNote();
    void testScoreIndexPrevNote();
    void testScoreSelection();

    void testUndoRedo();
    void testUndoRedoMany();

    void testExportMusicXML();
};

namespace { int randi(int mi, int ma)
{
    return ma==mi ? mi : (rand()%(ma-mi)) + mi;
} }

Note SonotCoreTest::createRandomNote()
{
    if (rand()%5 < 4)
        return Note( Note::Name(rand()%7),
                     1 + rand()%6,
                     rand()%4 == 0 ? (rand()%5-1) : 0);
    else
        return Note( rand()%4 == 0 ? Note::Rest : Note::Space );
}

Notes SonotCoreTest::createRandomNotes(size_t length)
{
    Notes b(length);
    for (size_t x = 0; x < length; ++x)
    {
        if (rand()%10 <= 7)
        {
            b.setNote(x, createRandomNote() );
        }
    }
    return b;
}

Bar SonotCoreTest::createRandomBar(size_t length, size_t rows)
{
    Bar b;
    for (size_t i=0; i<rows; ++i)
        b.append( createRandomNotes(length) );
    return b;
}

Score::Index SonotCoreTest::getRandomIndex(const Score* score)
{
    QPROPS_ASSERT(score->numNoteStreams(), score->toInfoString());
    for (int i=0; i<1000; ++i)
    {
        int stream = rand() % score->numNoteStreams();
        if (score->noteStream(stream).numBars() == 0
         || score->noteStream(stream).numRows() == 0)
            continue;
        int bar = rand() % score->noteStream(stream).numBars();
        int row = rand() % score->noteStream(stream).numRows();
        if (score->noteStream(stream).bar(bar).notes(row).length() == 0)
            continue;
        int col = rand() % score->noteStream(stream)
                                    .bar(bar).notes(row).length();
        return score->index(stream, bar, row, col);
    }
    QPROPS_ERROR("No random index found in Score " << score->toInfoString());
}

bool SonotCoreTest::compare(const Score &a, const Score &b)
{
#define MY_QCOMPARE(actual, expected) \
    { if (!QTest::qCompare(actual, expected, #actual, #expected, \
            __FILE__, __LINE__)) return false; }

    if (a == b)
        return true;
    if (a.props() != b.props())
        qDebug() << "properties don't match";
    MY_QCOMPARE(a.numNoteStreams(), b.numNoteStreams());

    if (a.numNoteStreams() == b.numNoteStreams())
    for (size_t i=0; i<a.numNoteStreams(); ++i)
    {
        if (a.noteStream(i) != b.noteStream(i))
        {
            MY_QCOMPARE(a.noteStream(i).numBars(), b.noteStream(i).numBars());
            MY_QCOMPARE(a.noteStream(i).numRows(), b.noteStream(i).numRows());

            if (a.noteStream(i).numBars() == b.noteStream(i).numBars())
            for (size_t j=0; j<a.noteStream(i).numBars(); ++j)
            {
                const Bar& barA = a.noteStream(i).bar(j),
                           barB = b.noteStream(i).bar(j);
                MY_QCOMPARE(barA, barB);
            }
        }
    }

    return false;
}




void SonotCoreTest::testNoteFromString()
{
#define SONOT__COMPARE(str__, N__, oct__, acc__) \
    QCOMPARE(Note(str__),       Note(Note::N__, 3+(oct__), (acc__))); \
    QCOMPARE(Note(str__ "4"),   Note(Note::N__, 4        , (acc__))); \
    QCOMPARE(Note(str__ "-5"),  Note(Note::N__, 5        , (acc__))); \
    QCOMPARE(Note(str__ ",,"),  Note(Note::N__, 1+(oct__), (acc__))); \
    QCOMPARE(Note(str__ "'''"), Note(Note::N__, 6+(oct__), (acc__)));
    //QCOMPARE(Note(str__).value(), int8_t(Note::N__ + 3 * 12));
    //qDebug() << Note(str__).toNoteString() << Note(Note::N__, 3).toNoteString();

    SONOT__COMPARE("C",     C   , 0, 0);
    SONOT__COMPARE("c",     C   , 0, 0);
    SONOT__COMPARE("d",     D   , 0, 0);
    SONOT__COMPARE("e",     E   , 0, 0);
    SONOT__COMPARE("f",     F   , 0, 0);
    SONOT__COMPARE("g",     G   , 0, 0);
    SONOT__COMPARE("a",     A   , 0, 0);
    SONOT__COMPARE("b",     B   , 0, 0);
    SONOT__COMPARE("b#",    B   , 0, 1);
    SONOT__COMPARE("bb",    B   , 0, -1);
    SONOT__COMPARE("h",     B   , 0, 0);

    SONOT__COMPARE("1b",    F   , 0, -1);
    SONOT__COMPARE("1",     F   , 0, 0);
    SONOT__COMPARE("1#",    F   , 0, 1);
    SONOT__COMPARE("2b",    G   , 0, -1);
    SONOT__COMPARE("2",     G   , 0, 0);
    SONOT__COMPARE("2#",    G   , 0, 1);
    SONOT__COMPARE("3b",    A   , 0, -1);
    SONOT__COMPARE("3",     A   , 0, 0);
    SONOT__COMPARE("3#",    A   , 0, 1);
    SONOT__COMPARE("4b",    B   , 0, -1);
    SONOT__COMPARE("4",     B   , 0, 0);
    SONOT__COMPARE("4#",    B   , 0, 1);
    SONOT__COMPARE("5b",    C   , 1, -1);
    SONOT__COMPARE("5",     C   , 1, 0);
    SONOT__COMPARE("5#",    C   , 1, 1);
    SONOT__COMPARE("6b",    D   , 1, -1);
    SONOT__COMPARE("6",     D   , 1, 0);
    SONOT__COMPARE("6#",    D   , 1, 1);
    SONOT__COMPARE("7b",    E   , 1, -1);
    SONOT__COMPARE("7",     E   , 1, 0);
    SONOT__COMPARE("7#",    E   , 1, 1);
}

void SonotCoreTest::testNoteFromValue()
{
    for (int8_t oct=0; oct<10; ++oct)
    {
        int8_t o = oct * 12;
        QCOMPARE( Note::fromValue(o+0),  Note(Note::C, oct, 0) );
        QCOMPARE( Note::fromValue(o+1),  Note(Note::C, oct, 1) );
        QCOMPARE( Note::fromValue(o+2),  Note(Note::D, oct, 0) );
        QCOMPARE( Note::fromValue(o+3),  Note(Note::D, oct, 1) );
        QCOMPARE( Note::fromValue(o+4),  Note(Note::E, oct, 0) );
        QCOMPARE( Note::fromValue(o+5),  Note(Note::F, oct, 0) );
        QCOMPARE( Note::fromValue(o+6),  Note(Note::F, oct, 1) );
        QCOMPARE( Note::fromValue(o+7),  Note(Note::G, oct, 0) );
        QCOMPARE( Note::fromValue(o+8),  Note(Note::G, oct, 1) );
        QCOMPARE( Note::fromValue(o+9),  Note(Note::A, oct, 0) );
        QCOMPARE( Note::fromValue(o+10), Note(Note::A, oct, 1) );
        QCOMPARE( Note::fromValue(o+11), Note(Note::B, oct, 0) );
    }
    for (int8_t i=0;i!=-128; ++i)
    {
        QCOMPARE( Note::fromValue(i).value(), i );
    }
}

void SonotCoreTest::testNoteTranspose()
{
    //for (int i=0; i<127; ++i)
    //    qDebug() << Note::fromValue(i).toNoaString();

    Note a, b;
#define SONOT__COMP(a__, t__, b__) \
    a = Note(a__).transposed(t__); b = Note(b__); \
    if (a != b) qDebug() << "mismatch" << b.toNoaString() \
                         << "->" << a.toNoaString(); \
    QCOMPARE( Note(a__).transposed(t__), Note(b__) );

    SONOT__COMP("C",  1, "C#");
    SONOT__COMP("C#", 1, "D" );
    SONOT__COMP("C#", 2, "D#");
    SONOT__COMP("B",  1, "C4");
#undef SONOT__COMP
}

void SonotCoreTest::testKeySignature()
{
    KeySignature k, k1;
    k.setKey(Note::B, -1);
    k.setKey(Note::F,  1);
    k.setKey(Note::G, -2);

    //QCOMPARE(k.transform(Note(Note::B)), Note(Note::B, 3, -1));
    //QCOMPARE(k.transform(Note(Note::F)), Note(Note::F, 3,  1));
    //QCOMPARE(k.transform(Note(Note::G)), Note(Note::F, 3, -2));

    qDebug() << k.toString();
    k1.fromJson(k.toJson());
    qDebug() << k1.toString();

    QCOMPARE(k1, k);
}

void SonotCoreTest::testResize()
{
    Notes bar(2);

    QCOMPARE(bar.length(), size_t(2));

    bar.resize(3);

    QCOMPARE(bar.length(), size_t(3));
}

/** Generates random score (WITH EMPTY STREAMS/BARS!!)
    and verifies that Score::Index iterators always
    lead to valid positions. */
void SonotCoreTest::testRandomCursor()
{
    for (int iter=0; iter<300; ++iter)
    {
        Score s;
        for (int i=0; i<5 + rand()%20; ++i)
        {
            NoteStream n;
            int jc = i == 0 ? 4 : (rand()%20);
            for (int j=0; j<jc; ++j)
            {
                Bar bar;
                int kc = i == 0 ? 4 : (rand()%10);
                for (int k=0; k<kc; ++k)
                    bar.append(
                        createRandomNotes(i == 0 ? 4 : (rand()%17)));
                n.appendBar(bar);
            }
            //qDebug().noquote() << ("\n" + n.toInfoString());
            s.appendNoteStream(n);
        }

        auto c = s.index(0,0,0,0), cprev = c;
        QVERIFY(c.isValid());

        for (int i=0; i<00; ++i)
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
    Notes bar1(2), bar2(5);
    bar1.setNote(0, Note(Note::A));
    bar1.setNote(1, Note(Note::B));
    bar2.setNote(0, Note(Note::A));
    bar2.setNote(1, Note(Note::B));

    bar1.resize(5);

    QCOMPARE(bar1, bar2);
}



void SonotCoreTest::testJsonNotes()
{
    Notes n2, n1 = createRandomNotes(8);

    n2.fromJsonString(n1.toJsonString());
    QCOMPARE(n2, n1);
}

void SonotCoreTest::testJsonStream()
{
    NoteStream stream1, stream2;
    for (int i=0; i<5; ++i)
        stream1.appendBar( createRandomNotes(rand()%4 + 4) );

    stream2.fromJsonString(stream1.toJsonString());
    //qDebug().noquote() << "\n" << stream1.toJsonString()
    //                   << "\n" << stream2.toJsonString();
    QCOMPARE(stream2, stream1);
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
    auto p = score1.props();
    p.set("version", 23);
    score1.setProperties(p);

    score2.fromJsonString(score1.toJsonString());
    QCOMPARE(score1, score2);
    //qDebug() << score1.noteStream(0).toString();
    //qDebug() << score2.toJsonString();
}

Score SonotCoreTest::createRandomScore(int mi, int ma, int numStreams)
{
    Score score;
    for (int i=0; i<numStreams; ++i)
    {
        NoteStream s;
        for (int j=0; j<randi(mi,ma); ++j)
            s.appendBar( createRandomBar(randi(mi,ma), randi(mi,ma)) );
        score.appendNoteStream(s);
    }
    score.setTitle("Amazing Haze");
    score.setAuthor("Convenieous Bar");
    score.setCopyright("(c) 1964");
    return score;
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
#if 0
    qDebug().noquote().nospace()
            << "\n" << score.noteStream(0).toTabString()
            << "\n" << score.noteStream(1).toTabString();
#endif
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

void SonotCoreTest::testScoreSelection()
{
    Score score = createRandomScore(2, 20);

    Score::Selection s1(score.index(0,0,0,0));

    QCOMPARE( s1.isSingleNote(), true);
    QCOMPARE( s1.isSingleStream(), true);
    QCOMPARE( s1.unified(score.index(1,0,0,0)).isSingleStream(), false);
    QCOMPARE( s1.unified(score.index(0,0,1,0)).isSingleColumn(), true);
    QCOMPARE( s1.unified(score.index(0,0,0,1)).isSingleRow(), true);


    //Score::Selection s2 = Score::Selection::fromBar(s1.from());
}

bool SonotCoreTest::makeRandomEditorAction(ScoreEditor &editor)
{
    Score::Index idx = getRandomIndex(editor.score());
    if (!idx.isValid())
    {
        qDebug() << "invalid random index" << idx.toString();
        return false;
    }

    switch (rand() % 6)
    {
        case 0:
            return editor.insertNote( idx, createRandomNote(), rand()%2 );
        break;

        case 1:
            return editor.insertRow( idx, rand()%2 );
        break;

        case 2:
            return editor.insertBar( idx, createRandomBar(
                                  1+rand()%12, 1+rand()%30), rand()%2 );
        break;

        case 3:
        {
            auto props = editor.score()->noteStream(
                                            idx.stream()).props();
            props.set("bpm", double(rand()%30000)/100.);
            props.set("title", props.get("title").toString()
                                + QString("%1").arg(rand()));
            return editor.setStreamProperties(idx.stream(), props);
        }
        break;

        case 4:
            return editor.deleteBar( idx );
        break;

        case 5:
            return editor.deleteRow( idx );
        break;
    }
    return false;
}

void SonotCoreTest::testUndoRedo()
{
    ScoreEditor editor;
    editor.setCollapseUndo(false);
    editor.setScore( createRandomScore() );

    QString undoAction;
    connect(&editor, &ScoreEditor::undoAvailable,
            [&](bool e, const QString&, const QString& n)
    {
        if (e) undoAction = n;
    });

    for (int it=0; it<1000; ++it)
    {
        editor.clearUndo();
        undoAction.clear();
        Score backup = *editor.score();

        QVERIFY( makeRandomEditorAction(editor) );
        if (backup == *editor.score())
            qDebug() << "no change to score for #" << it << undoAction;
        QVERIFY(backup != *editor.score());
        Score backupAction = *editor.score();

        if (!editor.undo())
        {
            qDebug() << "undo returned false for #" << it << undoAction;
            QFAIL("");
        }
        if (!compare(*editor.score(), backup))
            qDebug() << "mismatch after undo #" << it << undoAction;
        QCOMPARE(*editor.score(), backup);

        if (!editor.redo())
        {
            qDebug() << "redo returned false for #" << it << undoAction;
            QFAIL("");
        }
        if (!compare(*editor.score(), backupAction))
            qDebug() << "mismatch after redo #" << it << undoAction;
        QCOMPARE(*editor.score(), backupAction);
    }
}

void SonotCoreTest::testUndoRedoMany()
{
    ScoreEditor editor;
    editor.setCollapseUndo(false);
    editor.setScore( createRandomScore() );
    editor.clearUndo();

    QString undoAction;
    connect(&editor, &ScoreEditor::undoAvailable,
            [&](bool e, const QString& n)
    {
        if (e) undoAction = n;
    });

    QList<Score> history;

    for (int it=0; it<300; ++it)
    {

        history << *editor.score();

        try
        {
            QVERIFY( makeRandomEditorAction(editor) );
        }
        catch (const QProps::Exception& e)
        {
            qDebug().noquote() << e.text();
            QFAIL("exception in editor action");
        }

        Score current = *editor.score();

        // test undo

        int numUndos = std::max(1,std::min(200, int(rand() % history.size()) ));
        try
        {
            for (int i=0; i<numUndos; ++i)
                editor.undo();
        }
        catch (const QProps::Exception& e)
        {
            qDebug().noquote() << e.text();
            QFAIL("exception in undo action");
        }

        if (!compare(*editor.score(), history[history.size()-numUndos]))
            qDebug() << "mismatch after undo " << undoAction;
        QCOMPARE(*editor.score(), history[history.size()-numUndos]);
#if 1
        // test redo
        try
        {
            // redo all actions until most recent score
            int k=0;
            while (editor.redo()) ++k;
            QCOMPARE(k, numUndos);
        }
        catch (const QProps::Exception& e)
        {
            qDebug().noquote() << e.text();
            QFAIL("exception in redo action");
        }

        if (!compare(*editor.score(), current))
            qDebug() << "mismatch after redo " << undoAction;
        QCOMPARE(*editor.score(), current);
#endif
        if ((it % 50) == 49)
            qDebug().noquote() << (it+1) << "actions";
    }
}


void SonotCoreTest::testExportMusicXML()
{
    return;
    Score score = createRandomScore(2, 10, 1);
    ExportMusicXML exp(score);
    qDebug().noquote().nospace() << "\n" << exp.toString();
}


QTEST_APPLESS_MAIN(SonotCoreTest)

#include "SonotCoreTest.moc"
