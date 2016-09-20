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
        , numPages      (0)
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
        R_PAGE_FINISHED,
        R_LINE_FINISHED
    };

    void initProps();
    void initLayout();
    void initEditor();

    Score* score() { return editor ? editor->score() : nullptr; }

    void createItems();
    bool createBarItems_Fixed(int pageIdx, Score::Index& scoreIdx);
    ReturnCode createBarItems_Fixed(
            int pageIdx, int lineIdx, Score::Index& scoreIdx, QPointF& pagePos);

    void emitRefresh() { if (editor) emit editor->refresh(); }
    void emitDocumentChanged() { if (editor) emit editor->documentChanged(); }

    ScoreDocument* p;

    ScoreEditor* editor;
    size_t numPages;

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
    QProps::Properties::NamedValues pageOrder;
    pageOrder.set("horizontal", tr("horizontal"),
                  tr("All pages are display beneath each other"),
                  (int)PO_HORIZONTAL);
    pageOrder.set("vertical", tr("vertical"),
                  tr("All pages are display one upon the other"),
                  (int)PO_VERTICAL);
    pageOrder.set("two-sided", tr("two sides"),
                  tr("Pages are display like in a book"),
                  (int)PO_TWO_SIDED);

    props.set("page-spacing", tr("page spacing"),
              tr("Spacing between displayed pages"),
              QPointF(2, 8));
    props.setMin("page-spacing", QPointF(0,0));

    props.set("page-order", tr("page ordering"),
              tr("The display order of pages"),
              pageOrder, (int)PO_HORIZONTAL);
}

const QProps::Properties& ScoreDocument::props() const { return p_->props; }
void ScoreDocument::setProperties(const QProps::Properties &p)
{
    p_->props = p;
    p_->createItems();
    p_->emitDocumentChanged();
    p_->emitRefresh();
}

void ScoreDocument::setScoreProperties(const QProps::Properties &p)
{
    if (auto s=p_->score())
    {
        s->setProperties(p);
        p_->emitDocumentChanged();
        p_->emitRefresh();
    }
}

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

/** Attaches "document" object to Score json */
QJsonObject ScoreDocument::toJson() const
{
    //QProps::JsonInterfaceHelper json("ScoreDocument");
    QJsonObject o, jscore;
    if (p_->score())
        jscore = p_->score()->toJson();

    o.insert("score-layout", p_->scoreLayout.toJson());
    o.insert("page-layout", p_->pageLayout.toJson());
    o.insert("annotation", p_->pageAnnotation.toJson());
    o.insert("props", p_->props.toJson());
    jscore.insert("document", o);

    return jscore;
}

void ScoreDocument::fromJson(const QJsonObject& jscore)
{
    QProps::JsonInterfaceHelper json("ScoreDocument");
    ScoreDocument tmp;
    Score tmpScore;

    tmpScore.fromJson(jscore);
    if (jscore.contains("document"))
    {
        QJsonObject o = json.expectChildObject(jscore, "document");
        tmp.p_->scoreLayout.fromJson(
                    json.expectChildObject(o, "score-layout") );
        tmp.p_->pageLayout.fromJson(
                    json.expectChildObject(o, "page-layout") );
        tmp.p_->pageAnnotation.fromJson(
                    json.expectChildObject(o, "annotation") );
        tmp.p_->props.fromJson(
                    json.expectChildObject(o, "props") );

        p_->scoreLayout = tmp.p_->scoreLayout;
        p_->pageLayout = tmp.p_->pageLayout;
        p_->pageAnnotation = tmp.p_->pageAnnotation;
        p_->props = tmp.p_->props;
    }

    setScore(tmpScore);
}


const Score* ScoreDocument::score() const
{
    return p_->score();
}

void ScoreDocument::setScore(const Score& s)
{
    p_->editor->setScore(s);
}

size_t ScoreDocument::numPages() const { return p_->numPages; }

ScoreDocument::PageOrdering ScoreDocument::pageOrdering() const
{
    return (PageOrdering)p_->props.get("page-order").toInt();
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
    switch (pageOrdering())
    {
        case PO_HORIZONTAL:
            return QPointF(pageIndex * (r.width() + spacing.x()), 0);
        break;
        default:
        case PO_VERTICAL:
            return QPointF(0, pageIndex * (r.height() + spacing.y()));
        break;
        case PO_TWO_SIDED:
            ++pageIndex;
            return QPointF(
                (pageIndex % 2) * (r.width() + spacing.x()),
                (pageIndex / 2) * (r.height() + spacing.y()));
        break;
    }
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
    switch (pageOrdering())
    {
        case PO_HORIZONTAL:
            if (p.y() < 0. || p.y() > 1.)
                return -1;
            return int(p.x());
        break;
        default:
        case PO_VERTICAL:
            if (p.x() < 0. || p.x() > 1.)
                return -1;
            return int(p.y());
        break;
        case PO_TWO_SIDED:
            if (p.x() >= 2. || p.y() < 0.)
                return -1;
            return int(p.y()) * 2 + int(p.x()) - 1;
        break;
    }
}

int ScoreDocument::pageIndexForScoreIndex(const Score::Index &idx) const
{
    if (!idx.isValid())
        return -1;
    auto item = getScoreItem(idx);
    return item ? item->docIndex().pageIdx() : -1;
}

ScoreDocument::Index ScoreDocument::docIndexForScoreIndex(
        const Score::Index &idx) const
{
    if (!idx.isValid())
        return Index();
    auto item = getScoreItem(idx);
    return item ? item->docIndex() : Index();
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
        if (items->docIndex.p_pageIdx == pageIdx
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
        if (items->docIndex.p_pageIdx == pageIdx
         && items->boundingBox.contains(pagePos))
        {
            for (const ScoreItem& i : items->items)
                if (i.boundingBox().contains(pagePos))
                    return i.scoreIndex();
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

Score::Index ScoreDocument::goToPrevRow(const Score::Index &idx) const
{
    // item for index
    auto item = getScoreItem(idx);
    if (!item)
        return Score::Index();
    // position of this item
    QPointF p = item->boundingBox().center();
    // score layout that applies to this page
    auto slayout = scoreLayout(item->docIndex().pageIdx());

    // go to prev row or line position
    if (idx.row() > 0)
        p.ry() -= slayout.rowSpacing();
    else
        p.ry() -= slayout.rowSpacing() + slayout.lineSpacing();

    // find closest item
    auto newItem = getClosestScoreItem(item->docIndex().pageIdx(), p);
    if (newItem && newItem->scoreIndex() != idx)
        return newItem->scoreIndex();

    // check prev page, transform point to prev page's layout
    auto playout = pageLayout(item->docIndex().pageIdx());
    auto playout2 = pageLayout(item->docIndex().pageIdx() - 1);
    p.setX( p.x() - playout.scoreRect().x() + playout2.scoreRect().x() );
    p.setY( playout2.scoreRect().bottom() );

    newItem = getClosestScoreItem(item->docIndex().pageIdx()-1, p);
    if (newItem)
        return newItem->scoreIndex();

    return Score::Index();
}

Score::Index ScoreDocument::goToNextRow(const Score::Index &idx) const
{
    // item for index
    auto item = getScoreItem(idx);
    if (!item)
        return Score::Index();
    // position of this item
    QPointF p = item->boundingBox().center();
    // score layout that applies to this page
    auto slayout = scoreLayout(item->docIndex().pageIdx());

    // go to next row or line position
    if (idx.row() + 1 < idx.numRows())
        p.ry() += slayout.rowSpacing();
    else
        p.ry() += slayout.rowSpacing() + slayout.lineSpacing();

    // find closest item
    auto newItem = getClosestScoreItem(item->docIndex().pageIdx(), p);
    if (newItem && newItem->scoreIndex() != idx)
        return newItem->scoreIndex();

    // check next page, transform point to next page's layout
    auto playout = pageLayout(item->docIndex().pageIdx());
    auto playout2 = pageLayout(item->docIndex().pageIdx() + 1);
    p.setX( p.x() - playout.scoreRect().x() + playout2.scoreRect().x() );
    p.setY( playout2.scoreRect().top() );

    newItem = getClosestScoreItem(item->docIndex().pageIdx()+1, p);
    if (newItem)
        return newItem->scoreIndex();

    return Score::Index();
}

Score::Index ScoreDocument::goToStart(const Score::Index &idx) const
{
    // item for index
    auto item = getScoreItem(idx);
    if (!item)
        return Score::Index();
    // position of this item
    QPointF p = item->boundingBox().center();
    // page layout that applies to this page
    auto playout = pageLayout(item->docIndex().pageIdx());
    // move to line start
    p.setX(playout.scoreRect().left());
    auto newItem = getClosestScoreItem(item->docIndex().pageIdx(), p);
    if (newItem && newItem->scoreIndex() != idx)
        return newItem->scoreIndex();
    // move to page start
    p = playout.scoreRect().topLeft();
    newItem = getClosestScoreItem(item->docIndex().pageIdx(), p);
    if (newItem)
        return newItem->scoreIndex();
    return Score::Index();
}

Score::Index ScoreDocument::goToEnd(const Score::Index &idx) const
{
    // item for index
    auto item = getScoreItem(idx);
    if (!item)
        return Score::Index();
    // position of this item
    QPointF p = item->boundingBox().center();
    // page layout that applies to this page
    auto playout = pageLayout(item->docIndex().pageIdx());
    // move to line end
    p.setX(playout.scoreRect().right());
    auto newItem = getClosestScoreItem(item->docIndex().pageIdx(), p);
    if (newItem && newItem->scoreIndex() != idx)
        return newItem->scoreIndex();
    // move to page end
    p = playout.scoreRect().bottomRight();
    newItem = getClosestScoreItem(item->docIndex().pageIdx(), p);
    if (newItem)
        return newItem->scoreIndex();
    return Score::Index();
}

ScoreItem* ScoreDocument::getClosestScoreItem(
        int pageIdx, const QPointF &pagePos) const
{
    ScoreItem* best = nullptr;
    double bestDist = 0.;
    for (auto& sp : p_->barItems)
    {
        BarItems* items = sp.get();
        if (items->docIndex.p_pageIdx != pageIdx)
            continue;

        for (ScoreItem& item : items->items)
        {
            QPointF p = item.boundingBox().center();
            double dist = (p-pagePos).manhattanLength();
            if (best == nullptr || dist < bestDist)
                best = &item, bestDist = dist;
        }
    }
    return best;
}



void ScoreDocument::initLayout() { p_->initLayout(); }

/** @todo This is to be templated */
void ScoreDocument::Private::initLayout()
{
    // PageLayout
    {
        pageLayout.clear();

        PageLayout l;
        auto props = l.margins();
        // title page
        props.set("score-top", 30.);
        l.setMargins(props);
        pageLayout.insert("title", l);
        // left page
        props.setDefault();
        props.set("left", 30.);
        props.set("right", 20.);
        l.setMargins(props);
        pageLayout.insert("left", l);
        // right page
        props.setDefault();
        props.set("left", 20.);
        props.set("right", 30.);
        l.setMargins(props);
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

                ti = TextItem();
                ti.setBoundingBox(QRectF(0,0,100,25));
                ti.setBoxAlignment(Qt::AlignHCenter | Qt::AlignTop);
                ti.setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
                ti.setText("#author");
                ti.setFontFlags(TextItem::F_BOLD);
                ti.setFontSize(4.);
                page.textItems().push_back(ti);
            }

            //if (!titlePage)
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
                ti.setFontSize(4.);
                page.textItems().push_back(ti);
            }

            pageAnnotation.insert(pageId, page);
        }
    }


}

void ScoreDocument::setPageAnnotation(int pageIndex, const PageAnnotation &p)
{
    setPageAnnotation( layoutKeyForIndex(pageIndex), p);
}

void ScoreDocument::setPageLayout(int pageIndex, const PageLayout &p)
{
    setPageLayout( layoutKeyForIndex(pageIndex), p);
}

void ScoreDocument::setScoreLayout(int pageIndex, const ScoreLayout &p)
{
    setScoreLayout( layoutKeyForIndex(pageIndex), p);
}

void ScoreDocument::setPageAnnotation(const QString& id, const PageAnnotation &p)
{
    p_->pageAnnotation.insert(id, p);
    p_->createItems();
    p_->emitDocumentChanged();
    p_->emitRefresh();
}

void ScoreDocument::setPageLayout(const QString& id, const PageLayout &p)
{
    p_->pageLayout.insert(id, p);
    p_->createItems();
    p_->emitDocumentChanged();
    p_->emitRefresh();
}


void ScoreDocument::setScoreLayout(const QString& id, const ScoreLayout &p)
{
    p_->scoreLayout.insert(id, p);
    p_->createItems();
    p_->emitDocumentChanged();
    p_->emitRefresh();
}


void ScoreDocument::Private::initEditor()
{
    QObject::connect(editor, &ScoreEditor::scoreReset,
            [=](Score* )
    {
        createItems();
    });

    QObject::connect(editor, &ScoreEditor::noteValuesChanged,
            [=](const ScoreEditor::IndexList& )
    {
        createItems();
    });
    QObject::connect(editor, &ScoreEditor::barsChanged,
            [=](const ScoreEditor::IndexList& )
    {
        createItems();
    });
    QObject::connect(editor, &ScoreEditor::streamsChanged,
            [=](const ScoreEditor::IndexList& )
    {
        createItems();
    });

    QObject::connect(editor, &ScoreEditor::notesAboutToBeDeleted,
            [=](const ScoreEditor::IndexList& )
    {

    });
    QObject::connect(editor, &ScoreEditor::barsAboutToBeDeleted,
            [=](const ScoreEditor::IndexList& )
    {

    });
    QObject::connect(editor, &ScoreEditor::streamsAboutToBeDeleted,
            [=](const ScoreEditor::IndexList& )
    {

    });

    QObject::connect(editor, &ScoreEditor::notesDeleted,
            [=](const ScoreEditor::IndexList& )
    {
        createItems();
    });
    QObject::connect(editor, &ScoreEditor::barsDeleted,
            [=](const ScoreEditor::IndexList& )
    {
        createItems();
    });
    QObject::connect(editor, &ScoreEditor::streamsDeleted,
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
    numPages = 0;
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
        numPages = page + 1;
    }
}

bool ScoreDocument::Private::createBarItems_Fixed(
        int pageIdx, Score::Index& scoreIdx)
{
    QPointF pagePos;
    for (int line=0; ; ++line)
    {
        ReturnCode ret = createBarItems_Fixed(pageIdx, line, scoreIdx, pagePos);
        if (ret == R_DATA_FINISHED)
            return false;
        else if (ret == R_PAGE_FINISHED)
            return true;
    }
    return false;
}

ScoreDocument::Private::ReturnCode
ScoreDocument::Private::createBarItems_Fixed(
        int pageIdx, int lineIdx,
        Score::Index& scoreIdx, QPointF& pagePos)
{
    const ScoreLayout& slayout = p->scoreLayout(pageIdx);
    const PageLayout& playout = p->pageLayout(pageIdx);
    const QRectF scoreRect = playout.scoreRect();

    const double lineHeight = slayout.lineHeight(scoreIdx.numRows()),
                 lineHeightFull = lineHeight + slayout.lineSpacing();

    // break if page ends
    if (pagePos.y() + lineHeight >= scoreRect.height() )
        return R_PAGE_FINISHED;

    const double
            noteSize = slayout.noteSize(),
            rowSpace = slayout.rowSpacing();
    const size_t numBarsPerLine = slayout.barsPerLine();
    QPROPS_ASSERT(numBarsPerLine > 0, "");
    const double barWidth = scoreRect.width() / numBarsPerLine;

    KeySignature keySig = scoreIdx.getStream().keySignature();

    for (size_t barIdx = 0; barIdx < numBarsPerLine; ++barIdx)
    {
        // container for one bar block
        auto items = new BarItems;
        items->docIndex = Index(pageIdx, lineIdx, barIdx);
        items->scoreIndex = scoreIdx;

        QString anno;

        // annotation at stream start
        if (scoreIdx.isStreamLeft())
        {
            // stream title
            anno += scoreIdx.getStream().props().get("title").toString();

            // signature
            if (!keySig.isEmpty())
            {
                if (!anno.isEmpty())
                    anno += " ";
                anno += keySig.toString();
            }
        }

        // tempo indicator
        if (scoreIdx.isStreamLeft() || scoreIdx.isTempoChange())
        {
            if (!anno.isEmpty())
                anno += " ";
            anno += QString("%1bpm").arg(scoreIdx.getBeatsPerMinute());
        }

        // print annotation
        if (!anno.isEmpty())
        {
            double si = noteSize;
            QRectF rect(scoreRect.x() + barIdx * barWidth,
                        scoreRect.y() + pagePos.y(),
                        si, si);
            items->items.push_back(
                ScoreItem(scoreIdx, items->docIndex, rect, anno) );
            pagePos.ry() += si + 2;
        }


        // create item for each note in this bar block
        for (size_t row=0; row<scoreIdx.numRows(); ++row)
        {
            double y = pagePos.y() + rowSpace * row;

            const Notes& b = scoreIdx.getNotes(row);
            for (size_t col=0; col<b.length(); ++col)
            {
                Note note = scoreIdx.offset(row, col).getNote();
                //note = keySig.transform(note);
                if (!note.isValid())
                    continue;

                double x = barIdx * barWidth
                         + b.columnTime(col+.5) * barWidth;

                QRectF rect(x + scoreRect.x() -noteSize/2.,
                            y + scoreRect.y(),
                            noteSize, noteSize);
                items->items.push_back(
                            ScoreItem(scoreIdx.offset(row, col),
                                      items->docIndex, rect, note) );
                scoreItemMap.insert(
                            items->items.back().scoreIndex(),
                            &items->items.back() );
            }
        }

        // leading bar-slash item
        double x = barIdx * barWidth + scoreRect.x(),
               y = pagePos.y() + scoreRect.y();
        QLineF line(x, y, x, y + slayout.lineHeight(scoreIdx.numRows()));
        items->items.push_back( ScoreItem(scoreIdx,
                                          items->docIndex,
                                          line) );

        if (barIdx + 1 == numBarsPerLine)
        {
            // end-of-line bar-slash item
            double x = scoreRect.right(),
                   y = pagePos.y() + scoreRect.y();
            QLineF line(x, y, x, y + lineHeight);
            items->items.push_back( ScoreItem(scoreIdx,
                                              items->docIndex,
                                              line) );
        }

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


        // forward index and check for stream change
        size_t curStream = scoreIdx.stream();
        bool finish = !scoreIdx.nextBar(),
             newStream = (scoreIdx.stream() != curStream);
        if (finish || newStream)
        {
            // trailing bar-slash item
            double x = (barIdx+1) * barWidth + scoreRect.x(),
                   y = pagePos.y() + scoreRect.y();
            QLineF line(x, y, x, y + lineHeight);
            items->items.push_back( ScoreItem(scoreIdx,
                                              items->docIndex,
                                              line) );
            line.translate(-1, 0.);
            items->items.push_back( ScoreItem(scoreIdx,
                                              items->docIndex,
                                              line) );
            // next line
            pagePos.ry() += lineHeightFull;

            return newStream ? R_LINE_FINISHED : R_DATA_FINISHED;
        }

    }
    // next line
    pagePos.ry() += lineHeightFull;

    return R_ALL_GOOD;
}

void ScoreDocument::paintScoreItems(QPainter &p, int pageIdx,
                                    const QRectF& updateRect) const
{
    for (auto& sp : p_->barItems)
    {
        BarItems* items = sp.get();
        if (items->docIndex.p_pageIdx != pageIdx)
            continue;

        if (updateRect.intersects(items->boundingBox))
        for (ScoreItem& item : items->items)
        {
            item.paint(p);
        }
    }
}

} // namespace Sonot
