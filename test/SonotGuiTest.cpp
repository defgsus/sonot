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

#include "core/NoteStream.h"
#include "core/Score.h"
#include "gui/ScoreDocument.h"
#include "gui/PageAnnotation.h"
#include "gui/PageLayout.h"
#include "gui/ScoreLayout.h"
#include "QProps/Properties.h"

using namespace Sonot;

#if 1
#   define PRINT(arg__) std::cout << arg__ << std::endl;
#else
#   define PRINT(unused__) { }
#endif



namespace QTest {

    template <>
    char* toString(const QProps::Properties& p)
    {
        return toString(p.toCompactString());
    }

    template <>
    char* toString(const TextItem& p)
    {
        return toString(p.props().toCompactString());
    }

} // namespace QTest


class SonotGuiTest : public QObject
{
    Q_OBJECT

public:
    SonotGuiTest() { }

    ScoreLayout createRandomScoreLayout();
    PageLayout createRandomPageLayout();
    TextItem createRandomTextItem();
    void randomizeScoreDocument(ScoreDocument& s);

private slots:

    void testJsonScoreLayout();
    void testJsonPageLayout();
    void testJsonTextItem();
    void testJsonScoreDocument();
};

// helper
namespace {

    double rnd(double mi, double ma)
    {
        return mi + (double(rand()) / RAND_MAX) * (ma - mi);
    }

    /** Returns a variable with randomly set flags
        taken from QProps::Properties::NamedValues */
    template <typename T>
    T randomFlags(const QProps::Properties::NamedValues& nv)
    {
        qlonglong flags = 0;
        for (const QProps::Properties::NamedValues::Value& v : nv)
            if (rnd(0,1) < .5)
            {
                //qDebug() << "RANDOM FLAG" << v.v.toLongLong() << v.name;
                flags |= v.v.toLongLong();
            }
        //qDebug() << "RESULTS IN " << flags;
        return T(int(flags));
    }

} // namespace

ScoreLayout SonotGuiTest::createRandomScoreLayout()
{
    ScoreLayout l;
    QProps::Properties p = l.props();
    p.set("line-spacing", rnd(1,10));
    p.set("min-bar-width", rnd(10,50));
    p.set("max-bar-width", rnd(50,100));
    p.set("note-size", rnd(2,12));
    p.set("note-spacing", rnd(1,10));
    p.set("row-spacing", rnd(1,10));
    l.setProperties(p);
    return l;
}

PageLayout SonotGuiTest::createRandomPageLayout()
{
    PageLayout l;
    QProps::Properties p = l.margins();
    p.set("left", rnd(1,40));
    p.set("right", rnd(1,40));
    p.set("top", rnd(1,40));
    p.set("bottom", rnd(1,40));
    p.set("score-left", rnd(1,40));
    p.set("score-right", rnd(1,40));
    p.set("score-top", rnd(1,40));
    p.set("score-bottom", rnd(1,40));
    l.setMargins(p);
    return l;
}

TextItem SonotGuiTest::createRandomTextItem()
{
    TextItem t;
    t.setBoundingBox(QRectF(rnd(0,100),rnd(0,100),rnd(10,100),rnd(10,100)));
    t.setBoxAlignment(randomFlags<Qt::Alignment>(
                          QProps::Properties::namedValuesQtAlignment()));
    t.setColor(QColor(rnd(0,255),rnd(0,255),rnd(0,255)));
    t.setFontFlags(randomFlags<TextItem::FontFlag>(
                       TextItem::fontFlagNamedValues()));
    t.setFontSize(rnd(1,20));
    t.setText("A non-random string");
    t.setTextAlignment(randomFlags<Qt::Alignment>(
                           QProps::Properties::namedValuesQtAlignment()));
    t.setTextFlags(randomFlags<Qt::TextFlag>(
                   QProps::Properties::namedValuesQtTextFlag()));
    return t;
}

void SonotGuiTest::randomizeScoreDocument(ScoreDocument& s)
{
    s.initLayout();
    s.setPageLayout(0, createRandomPageLayout());
    s.setPageLayout(1, createRandomPageLayout());
    s.setPageLayout(2, createRandomPageLayout());
    QProps::Properties p = s.props();
    p.set("page-spacing", QPointF(rnd(1,10), rnd(1,10)));
    s.setProperties(p);
}




void SonotGuiTest::testJsonScoreLayout()
{
    ScoreLayout l2, l = createRandomScoreLayout();
    l2.fromJsonString(l.toJsonString());
    PRINT(l2.props().toCompactString().toStdString());
    QCOMPARE(l, l2);
}

void SonotGuiTest::testJsonPageLayout()
{
    PageLayout l2, l = createRandomPageLayout();
    //qDebug() << "-----------" << l.toJsonString();
    l2.fromJsonString(l.toJsonString());
    PRINT(l2.toJsonString().toStdString());
    QCOMPARE(l, l2);
}

void SonotGuiTest::testJsonTextItem()
{
    TextItem t2, t = createRandomTextItem();
    PRINT(t.toJsonString().toStdString());
    t2.fromJsonString(t.toJsonString());

    QCOMPARE(t, t2);
}

void SonotGuiTest::testJsonScoreDocument()
{
    ScoreDocument s, s2;
    randomizeScoreDocument(s);
    PRINT(s.toJsonString().toStdString());
    s2.fromJsonString(s.toJsonString());
    //s2 = s;

    QCOMPARE(s, s2);
}

QTEST_APPLESS_MAIN(SonotGuiTest)

#include "SonotGuiTest.moc"
