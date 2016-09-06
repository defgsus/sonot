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
#include "gui/ScoreDocument.h"
#include "gui/PageAnnotationTemplate.h"
#include "gui/PageLayout.h"
#include "gui/ScoreLayout.h"

using namespace Sonot;

class SonotGuiTest : public QObject
{
    Q_OBJECT

public:
    SonotGuiTest() { }

    static double rnd(double mi, double ma);
    ScoreLayout createRandomScoreLayout();
    PageLayout createRandomPageLayout();

private slots:

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
    /** @todo */
    return l;
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
