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

#include <vector>
#include <memory>

#include <QPainter>

#include "QProps/JsonInterfaceHelper.h"
#include "QProps/error.h"

#include "ScoreDocument.h"
#include "core/NoteStream.h"
#include "core/ScoreEditor.h"
#include "PageLayout.h"
#include "ScoreLayout.h"
#include "PerPage.h"
#include "ScoreItem.h"

namespace Sonot {

struct ScoreDocument::Private
{
    Private(ScoreDocument* p)
        : p             (p)
        , editor        (new ScoreEditor(nullptr))
        , props         ("score-document")
    {
    }

    ~Private()
    {
        delete editor;
    }

    enum ReturnCode
    {
        R_ALL_GOOD,
        R_DATA_FINISHED,
        R_PAGE_FINISHED
    };

    void initProps();
    void initLayout();
    void initEditor();

    Score* score() { return editor ? editor->score() : nullptr; }

    void createItems();
    bool createBarItems_Fixed(int pageIdx, Score::Index& scoreIdx);
    ReturnCode createBarItems_Fixed(
            int pageIdx, int lineIdx, Score::Index& scoreIdx);

    ScoreDocument* p;

    ScoreEditor* editor;

    // -- config --

    PerPage<ScoreLayout> scoreLayout;
    PerPage<PageAnnotation> pageAnnotation;
    PerPage<PageLayout> pageLayout;

    QProps::Properties props;

    // -- render --

    std::vector<std::shared_ptr<BarItems>> barItems;

    // maps each page/line/bar to BarItems
    QMap<Index, BarItems*> barItemMap;
    // maps Score::Index to each ScoreItem
    QMap<Score::Index, ScoreItem*> scoreItemMap;
};


ScoreDocument::ScoreDocument()
    : p_        (new Private(this))
{
    p_->initProps();
    p_->initEditor();
    initLayout();
}

/*
ScoreDocument::ScoreDocument(const ScoreDocument& o)
    : p_        (new Private(this))
{
    *this = o;
}*/

ScoreDocument::~ScoreDocument()
{
    delete p_;
}

void ScoreDocument::Private::initProps()
{
    props.set("page-spacing", tr("page spacing"),
              tr("Spacing between displayed pages"),
              QPointF(2, 10));
}

const QProps::Properties& ScoreDocument::props() const { return p_->props; }
QProps::Properties& ScoreDocument::props() { return p_->props; }
void ScoreDocument::setProperties(const QProps::Properties &p) { p_->props = p; }

ScoreEditor* ScoreDocument::editor() const { return p_->editor; }

/*
ScoreDocument& ScoreDocument::operator = (const ScoreDocument& o)
{
    p_->scoreLayout = o.p_->scoreLayout;
    p_->pageAnnotation = o.p_->pageAnnotation;
    p_->pageLayout = o.p_->pageLayout;
    p_->props = o.p_->props;
    return *this;
}*/

bool ScoreDocument::operator == (const ScoreDocument& o) const
{
    return p_->scoreLayout == o.p_->scoreLayout
        && p_->pageAnnotation == o.p_->pageAnnotation
        && p_->pageLayout == o.p_->pageLayout
        && p_->props == o.p_->props
            ;
}


QJsonObject ScoreDocument::toJson() const
{
    QProps::JsonInterfaceHelper json("ScoreDocument");
    QJsonObject o;
    //if (p_->score())
    //    o.insert("score", p_->score()->toJson());
    o.insert("score-layout", p_->scoreLayout.toJson());
    o.insert("page-layout", p_->pageLayout.toJson());
    o.insert("annotation", p_->pageAnnotation.toJson());
    o.insert("props", p_->props.toJson());
    return o;
}

void ScoreDocument::fromJson(const QJsonObject& o)
{
    QProps::JsonInterfaceHelper json("ScoreDocument");
    ScoreDocument tmp;
    tmp.p_->scoreLayout.fromJson( json.expectChildObject(o, "score-layout") );
    tmp.p_->pageLayout.fromJson( json.expectChildObject(o, "page-layout") );
    tmp.p_->pageAnnotation.fromJson(
                json.expectChildObject(o, "annotation") );
    tmp.p_->props = p_->props;
    tmp.p_->props.fromJson( json.expectChildObject(o, "props") );

    /*if (o.contains("score"))
    {
        Score score;
        score.fromJson( json.expectChildObject(o, "score") );
    }*/

    p_->scoreLayout = tmp.p_->scoreLayout;
    p_->pageLayout = tmp.p_->pageLayout;
    p_->pageAnnotation = tmp.p_->pageAnnotation;
    p_->props = tmp.p_->props;
}


const Score* ScoreDocument::score() const
{
    return p_->score();
}

void ScoreDocument::setScore(const Score& s)
{
    p_->editor->setScore(s);
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

QString ScoreDocument::layoutKeyForIndex(int pageIdx) const
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

QRectF ScoreDocument::pageRect(int pageIdx) const
{
    QRectF r = pageRect();
    r.moveTo(pagePosition(pageIdx));
    return r;
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

int ScoreDocument::pageIndexForScoreIndex(const Score::Index &idx) const
{
    if (!idx.isValid())
        return -1;
    auto item = getScoreItem(idx);
    return item ? item->docIndex().pageIdx : -1;
}

ScoreDocument::BarItems* ScoreDocument::getBarItems(const Index& idx) const
{
    auto i = p_->barItemMap.find(idx);
    return i == p_->barItemMap.end() ? nullptr : i.value();
}


ScoreItem* ScoreDocument::getScoreItem(const Score::Index &idx) const
{
    auto i = p_->scoreItemMap.find(idx);
    return i == p_->scoreItemMap.end() ? nullptr : i.value();
}

ScoreDocument::BarItems* ScoreDocument::getBarItems(
        int pageIdx, const QPointF& pagePos) const
{
    for (auto& sp : p_->barItems)
    {
        BarItems* items = sp.get();
        if (items->docIndex.pageIdx == pageIdx
         && items->boundingBox.contains(pagePos))
        {
            return items;
        }
    }
    return nullptr;
}

Score::Index ScoreDocument::getScoreIndex(
        int pageIdx, const QPointF& pagePos) const
{
    for (auto& sp : p_->barItems)
    {
        BarItems* items = sp.get();
        if (items->docIndex.pageIdx == pageIdx
         && items->boundingBox.contains(pagePos))
        {
            for (const ScoreItem& i : items->items)
                if (i.boundingBox().contains(pagePos))
                    return i.index();
        }
    }
    return Score::Index();
}

Score::Index ScoreDocument::getScoreIndex(
        const QPointF& documentPos) const
{
    int page = pageIndexForDocumentPosition(documentPos);
    if (page < 0)
        return Score::Index();
    return getScoreIndex(page, documentPos - pagePosition(page));
}

/*
void ScoreDocument::updateScoreIndex(const Score::Index& i)
{
    QPROPS_ASSERT(i.isValid(), "in ScoreDocument::updateScoreIndex()");
    auto s = getScoreItem(i);
}
*/

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
    const QString id = layoutKeyForIndex(pageIndex);
    p_->pageAnnotation.insert(id, p);
}

void ScoreDocument::setPageLayout(int pageIndex, const PageLayout &p)
{
    const QString id = layoutKeyForIndex(pageIndex);
    p_->pageLayout.insert(id, p);
}

void ScoreDocument::Private::initEditor()
{
    QObject::connect(editor, &ScoreEditor::scoreReset,
            [=](Score* )
    {
        createItems();
    });
    QObject::connect(editor, &ScoreEditor::streamsChanged,
            [=](const ScoreEditor::IndexList& )
    {
        createItems();
    });
    QObject::connect(editor, &ScoreEditor::noteValuesChanged,
            [=](const ScoreEditor::IndexList& )
    {
        createItems();
    });
}


void ScoreDocument::Private::createItems()
{
    barItems.clear();
    barItemMap.clear();
    scoreItemMap.clear();
    if (!score())
        return;

    Score::Index scoreIdx = score()->index(0,0,0,0);
    if (!scoreIdx.isValid())
        return;

    /// @todo non-fixed layout
    if (p->scoreLayout(0).isFixedBarWidth())
    {
        int page = 0;
        while (createBarItems_Fixed(page, scoreIdx))
        {
            //qDebug() << scoreIdx.toString();
            ++page;
        }
    }
}

bool ScoreDocument::Private::createBarItems_Fixed(
        int pageIdx, Score::Index& scoreIdx)
{
    for (int line=0; ; ++line)
    {
        ReturnCode ret = createBarItems_Fixed(pageIdx, line, scoreIdx);
        if (ret == R_DATA_FINISHED)
            return false;
        else if (ret == R_PAGE_FINISHED)
            return true;
    }
    return false;
}

ScoreDocument::Private::ReturnCode
ScoreDocument::Private::createBarItems_Fixed(
        int pageIdx, int lineIdx, Score::Index& scoreIdx)
{
    const ScoreLayout& slayout = p->scoreLayout(pageIdx);
    const PageLayout& playout = p->pageLayout(pageIdx);
    const QRectF scoreRect = playout.scoreRect();

    double lineHeight = slayout.lineHeight(scoreIdx.numRows())
                      + slayout.lineSpacing();

    // break if page ended
    if ((lineIdx+1) * lineHeight
            >= scoreRect.height() - slayout.lineSpacing())
        return R_PAGE_FINISHED;

    const double
            noteSize = slayout.noteSize(),
            rowSpace = slayout.rowSpacing();
    const size_t numBarsPerLine = slayout.barsPerLine();
    QPROPS_ASSERT(numBarsPerLine > 0, "");
    const double barWidth = scoreRect.width() / numBarsPerLine;

    for (size_t barIdx = 0; barIdx < numBarsPerLine; ++barIdx)
    {
        // container for one bar block
        auto items = new BarItems;
        items->docIndex = Index(pageIdx, lineIdx, barIdx);
        items->scoreIndex = scoreIdx;

        // create item for each note in this bar block
        for (size_t row=0; row<scoreIdx.numRows(); ++row)
        {
            double y = lineIdx * lineHeight + rowSpace * row;

            const Bar& b = scoreIdx.getBar(row);
            for (size_t col=0; col<b.length(); ++col)
            {
                if (! scoreIdx.offset(row, col).getNote().isValid())
                    continue;

                double x = barIdx * barWidth
                         + b.columnTime(col+.5) * barWidth;

                QRect rect(x + scoreRect.x() -noteSize/2.,
                           y + scoreRect.y(),
                           noteSize, noteSize);
                items->items.push_back(
                            ScoreItem(scoreIdx.offset(row, col),
                                      items->docIndex, rect) );
                scoreItemMap.insert(
                            items->items.back().index(),
                            &items->items.back() );
            }
        }

        // bar-slash item
        double x = barIdx * barWidth + scoreRect.x(),
               y = lineIdx * lineHeight + scoreRect.y();
        QLineF line(x, y, x, y + slayout.lineHeight(scoreIdx.numRows()));
        items->items.push_back( ScoreItem(scoreIdx,
                                          items->docIndex,
                                          line) );

        // get bounding rect
        if (!items->items.isEmpty())
        {
            items->boundingBox = items->items.at(0).boundingBox();
            for (const ScoreItem& i : items->items)
                items->boundingBox |= i.boundingBox();
        }

        // store items
        std::shared_ptr<BarItems> pit(items);
        barItems.push_back(pit);

        barItemMap.insert(items->docIndex, items);

        if (!scoreIdx.nextBar())
            return R_DATA_FINISHED;
    }
    return R_ALL_GOOD;
}

void ScoreDocument::paintScoreItems(QPainter &p, int pageIdx,
                                    const QRectF& updateRect) const
{
    for (auto& sp : p_->barItems)
    {
        BarItems* items = sp.get();
        if (items->docIndex.pageIdx != pageIdx)
            continue;

        if (updateRect.intersects(items->boundingBox))
        for (ScoreItem& item : items->items)
        {
            item.paint(p);
        }
    }
}

} // namespace Sonot
