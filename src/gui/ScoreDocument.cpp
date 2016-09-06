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

#include "ScoreDocument.h"
#include "core/Score.h"
#include "PageAnnotationTemplate.h"
#include "PageLayout.h"
#include "ScoreLayout.h"

namespace Sonot {

struct ScoreDocument::Private
{
    Private(ScoreDocument* p)
        : p                     (p)
        , pageSpacing           (2., 10.)
    {
        pageAnnotationTemplate.init("default");
    }

    ~Private()
    {
    }

    ScoreDocument* p;

    Score score;
    ScoreLayout scoreLayout;
    PageAnnotationTemplate pageAnnotationTemplate;
    PageLayout pageLayout;

    QPointF pageSpacing;
};

ScoreDocument::ScoreDocument()
    : p_        (new Private(this))
{
    p_->score.setTitle("Space Invaders");
    p_->score.setCopyright("(c) 1963, Ingsoc");
    p_->score.setAuthor("Dorian Gray");
}

ScoreDocument::~ScoreDocument()
{
    delete p_;
}



const Score& ScoreDocument::score() const
{
    return p_->score;
}

QRectF ScoreDocument::pageRect() const
{
    return p_->pageLayout.pageRect();
}

const PageLayout& ScoreDocument::pageLayout(int /*pageIdx*/) const
{
    return p_->pageLayout;
}

const ScoreLayout& ScoreDocument::scoreLayout(int /*pageIdx*/) const
{
    return p_->scoreLayout;
}

PageAnnotation ScoreDocument::pageAnnotation(int pageIdx) const
{
    return p_->pageAnnotationTemplate.getPage(pageIdx);
}

int ScoreDocument::pageNumberForIndex(int pageIndex) const
{
    return pageIndex + 1;
}

QPointF ScoreDocument::pagePosition(int pageIndex) const
{
    auto r = pageRect();
    ++pageIndex;
    return QPointF(
                (pageIndex % 2) * (r.width() + p_->pageSpacing.x()),
                (pageIndex / 2) * (r.height() + p_->pageSpacing.y()));
}

int ScoreDocument::pageIndexForDocumentPosition(const QPointF& p0) const
{
    if (p0.x() < 0.)
        return -1;
    auto r = pageRect();
    QPointF p(p0);
    p -= r.topLeft();
    p.rx() /= (r.width() + p_->pageSpacing.x());
    p.ry() /= (r.height() + p_->pageSpacing.y());
    if (p.rx() >= 2. || p.ry() < 0.)
        return -1;

    return int(p.ry()) * 2 + int(p.rx()) - 1;
}




void ScoreDocument::setScore(const Score& s)
{
    p_->score = s;
}


} // namespace Sonot
