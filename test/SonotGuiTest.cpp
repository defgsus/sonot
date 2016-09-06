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

#include <iostream>

#include <QString>
#include <QtTest>

#include "io/Properties.h"
#include "core/NoteStream.h"
#include "core/Score.h"
#include "gui/ScoreDocument.h"
#include "gui/PageAnnotationTemplate.h"
#include "gui/PageLayout.h"
#include "gui/ScoreLayout.h"

using namespace Sonot;

#if 0
#   define PRINT(arg__) std::cout << arg__ << std::endl;
#else
#   define PRINT(unused__) { }
#endif



namespace QTest {

    template <>
    char* toString(const Properties& p)
    {
        return toString(p.toString());
    }

} // namespace QTest


class SonotGuiTest : public QObject
{
    Q_OBJECT

public:
    SonotGuiTest() { }

    static double rnd(double mi, double ma);
    ScoreLayout createRandomScoreLayout();
    PageLayout createRandomPageLayout();

private slots:

    void testJsonProperties();
    void testJsonScoreLayout();
    void testJsonPageLayout();
};


double SonotGuiTest::rnd(double mi, double ma)
{
    return mi + (double(rand()) / RAND_MAX) * (ma - mi);
}

ScoreLayout SonotGuiTest::createRandomScoreLayout()
{
    ScoreLayout l;
    l.setLineSpacing(rnd(1,10));
    l.setMaxBarWidth(rnd(50,100));
    l.setMinBarWidth(rnd(10,50));
    l.setNoteSize(rnd(2,12));
    l.setNoteSpacing(rnd(1,10));
    l.setRowSpacing(rnd(1,10));
    return l;
}

PageLayout SonotGuiTest::createRandomPageLayout()
{
    PageLayout l;
    l.marginsOdd().set("left", rnd(1,40));
    l.marginsOdd().set("right", rnd(1,40));
    l.marginsOdd().set("top", rnd(1,40));
    l.marginsOdd().set("bottom", rnd(1,40));
    l.marginsEven().set("left", rnd(1,40));
    l.marginsEven().set("right", rnd(1,40));
    l.marginsEven().set("top", rnd(1,40));
    l.marginsEven().set("bottom", rnd(1,40));
    l.scoreMarginsOdd().set("left", rnd(1,40));
    l.scoreMarginsOdd().set("right", rnd(1,40));
    l.scoreMarginsOdd().set("top", rnd(1,40));
    l.scoreMarginsOdd().set("bottom", rnd(1,40));
    l.scoreMarginsEven().set("left", rnd(1,40));
    l.scoreMarginsEven().set("right", rnd(1,40));
    l.scoreMarginsEven().set("top", rnd(1,40));
    l.scoreMarginsEven().set("bottom", rnd(1,40));
    return l;
}


void SonotGuiTest::testJsonProperties()
{
    Properties p("type-test");
    p.set("double", tr("name"), tr("tool-tip"),
                20., 1.);
    p.set("int",    23);
    p.set("float",  42.f);
    p.set("uint",   666u);
    p.set("long",   7777LL);
    p.set("string", QString("holladihoh"));

    PRINT( p.toString().toStdString() );
    PRINT( p.toJsonString().toStdString() );
    Properties p2("copy");
    p2.fromJson(p.toJson());

    PRINT( p2.toString().toStdString() );

    QCOMPARE(p, p2);
}



void SonotGuiTest::testJsonScoreLayout()
{
    ScoreLayout l2, l = createRandomScoreLayout();
    l2.fromJsonString(l.toJsonString());

    QCOMPARE(l, l2);
}

void SonotGuiTest::testJsonPageLayout()
{
    PageLayout l2, l = createRandomPageLayout();
    l2.fromJsonString(l.toJsonString());

    QCOMPARE(l, l2);
}

QTEST_APPLESS_MAIN(SonotGuiTest)

#include "SonotGuiTest.moc"
