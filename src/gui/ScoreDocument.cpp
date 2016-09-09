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

#include <QJsonObject>
#include <QJsonValue>

#include "ScoreDocument.h"
#include "core/Score.h"
#include "PageLayout.h"
#include "ScoreLayout.h"
#include "PerPage.h"

namespace Sonot {

struct ScoreDocument::Private
{
    Private(ScoreDocument* p)
        : p             (p)
        , props         ("score-document")
    {
        props.set("page-spacing", tr("page spacing"),
                  tr("Spacing between displayed pages"),
                  QPointF(2, 10));
    }

    ~Private()
    {
    }

    void initLayout();

    ScoreDocument* p;

    Score score;
    PerPage<ScoreLayout> scoreLayout;
    PerPage<PageAnnotation> pageAnnotation;
    PerPage<PageLayout> pageLayout;

    // -- config --

    Properties props;
};

ScoreDocument::ScoreDocument()
    : p_        (new Private(this))
{
    initLayout();
    p_->score.setTitle("Space Invaders");
    p_->score.setCopyright("(c) 1963, Ingsoc");
    p_->score.setAuthor("Dorian Gray");
}

ScoreDocument::ScoreDocument(const ScoreDocument& o)
    : p_        (new Private(this))
{
    *this = o;
}

ScoreDocument::~ScoreDocument()
{
    delete p_;
}

const Properties& ScoreDocument::props() const { return p_->props; }
Properties& ScoreDocument::props() { return p_->props; }
void ScoreDocument::setProperties(const Properties &p) { p_->props = p; }

ScoreDocument& ScoreDocument::operator = (const ScoreDocument& o)
{
    p_->score = o.p_->score;
    p_->scoreLayout = o.p_->scoreLayout;
    p_->pageAnnotation = o.p_->pageAnnotation;
    p_->pageLayout = o.p_->pageLayout;
    p_->props = o.p_->props;
    return *this;
}

bool ScoreDocument::operator == (const ScoreDocument& o) const
{
    return p_->score == o.p_->score
        && p_->scoreLayout == o.p_->scoreLayout
        && p_->pageAnnotation == o.p_->pageAnnotation
        && p_->pageLayout == o.p_->pageLayout
        && p_->props == o.p_->props
            ;
}


QJsonObject ScoreDocument::toJson() const
{
    JsonHelper json("ScoreDocument");
    QJsonObject o;
    o.insert("score", p_->score.toJson());
    o.insert("score-layout", p_->scoreLayout.toJson());
    o.insert("page-layout", p_->pageLayout.toJson());
    o.insert("annotation", p_->pageAnnotation.toJson());
    o.insert("props", p_->props.toJson());
    return o;
}

void ScoreDocument::fromJson(const QJsonObject& o)
{
    JsonHelper json("ScoreDocument");
    ScoreDocument tmp;
    tmp.p_->score.fromJson( json.expectChildObject(o, "score") );
    tmp.p_->scoreLayout.fromJson( json.expectChildObject(o, "score-layout") );
    tmp.p_->pageLayout.fromJson( json.expectChildObject(o, "page-layout") );
    tmp.p_->pageAnnotation.fromJson(
                json.expectChildObject(o, "annotation") );
    tmp.p_->props = p_->props;
    tmp.p_->props.fromJson( json.expectChildObject(o, "props") );

    *this = tmp;
}


const Score& ScoreDocument::score() const
{
    return p_->score;
}

QRectF ScoreDocument::pageRect() const
{
    return p_->pageLayout["title"].pageRect();
}

const PageLayout& ScoreDocument::pageLayout(const QString& id) const
{
    return p_->pageLayout[id];
}

const ScoreLayout& ScoreDocument::scoreLayout(const QString& id) const
{
    return p_->scoreLayout[id];
}

const PageAnnotation& ScoreDocument::pageAnnotation(const QString& id) const
{
    return p_->pageAnnotation[id];
}

int ScoreDocument::pageNumberForIndex(int pageIndex) const
{
    return pageIndex + 1;
}

QString ScoreDocument::keyForIndex(int pageIdx) const
{
    return pageIdx == 0 ? "title"
                        : (pageIdx & 1) == 1 ? "left"
                                             : "right";
}

QPointF ScoreDocument::pagePosition(int pageIndex) const
{
    auto r = pageRect();
    auto spacing = pageSpacing();
    ++pageIndex;
    return QPointF(
                (pageIndex % 2) * (r.width() + spacing.x()),
                (pageIndex / 2) * (r.height() + spacing.y()));
}

int ScoreDocument::pageIndexForDocumentPosition(const QPointF& p0) const
{
    if (p0.x() < 0.)
        return -1;
    auto r = pageRect();
    QPointF p(p0), spacing = pageSpacing();
    p -= r.topLeft();
    p.rx() /= (r.width() + spacing.x());
    p.ry() /= (r.height() + spacing.y());
    if (p.rx() >= 2. || p.ry() < 0.)
        return -1;

    return int(p.ry()) * 2 + int(p.rx()) - 1;
}



void ScoreDocument::initLayout() { p_->initLayout(); }

void ScoreDocument::Private::initLayout()
{
    // PageLayout
    {
        pageLayout.clear();

        PageLayout l;
        l.init(false);
        pageLayout.insert("title", l);
        pageLayout.insert("left", l);
        l.init(true);
        pageLayout.insert("right", l);
    }

    // PageAnnotation
    {
        pageAnnotation.clear();

        for (int k = 0; k < 3; ++k)
        {
            const bool titlePage = k == 0;
            const bool leftPage = k == 1;
            const QString pageId = titlePage ? "title"
                                             : leftPage ? "left" : "right";
            PageAnnotation page;
            TextItem ti;

            if (titlePage)
            {
                ti = TextItem();
                ti.setBoundingBox(QRectF(0,0,100,10));
                ti.setBoxAlignment(Qt::AlignHCenter | Qt::AlignTop);
                ti.setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
                ti.setText("#title");
                ti.setFontFlags(TextItem::F_ITALIC);
                page.textItems().push_back(ti);
            }

            if (!titlePage)
            {
                ti = TextItem();
                ti.setBoundingBox(QRectF(0,0,100,10));
                ti.setBoxAlignment(Qt::AlignHCenter | Qt::AlignBottom);
                ti.setText("#copyright");
                ti.setFontFlags(TextItem::F_ITALIC);
                ti.setFontSize(4.);
                page.textItems().push_back(ti);
            }

            if (!titlePage)
            {
                ti = TextItem();
                ti.setBoundingBox(QRectF(0,0,20,10));
                ti.setBoxAlignment(Qt::AlignBottom |
                                   (leftPage ? Qt::AlignLeft : Qt::AlignRight));
                ti.setTextAlignment(Qt::AlignBottom |
                                    (leftPage ? Qt::AlignLeft : Qt::AlignRight));
                ti.setText("#page");
                ti.setFontSize(5.);
                page.textItems().push_back(ti);
            }

            pageAnnotation.insert(pageId, page);
        }
    }


}

void ScoreDocument::setPageAnnotation(int pageIndex, const PageAnnotation &p)
{
    const QString id = keyForIndex(pageIndex);
    p_->pageAnnotation.insert(id, p);
}

void ScoreDocument::setPageLayout(int pageIndex, const PageLayout &p)
{
    const QString id = keyForIndex(pageIndex);
    p_->pageLayout.insert(id, p);
}


void ScoreDocument::setScore(const Score& s)
{
    p_->score = s;
}


} // namespace Sonot
