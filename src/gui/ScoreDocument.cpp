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
        , editor        (new ScoreEditor(p, nullptr))
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
    bool createPageItems(int pageIdx, Score::Index& scoreIdx, bool isFixed);
    ReturnCode createLineOfScore(
            int pageIdx, int lineIdx,
            Score::Index& scoreIdx, QPointF& pagePos,
            bool isFixed);

    QString getBarAnnotation(const Score::Index& scoreIdx,
                             const KeySignature& keySig);
    BarItems* createSingleBarItems(const Score::Index& scoreIdx,
            const ScoreDocument::Index& docIdx,
            const KeySignature& keySig,
            const QRectF& barRect, double noteSize, double rowSpace, bool endOfLine);
    void updateNoteItems(const ScoreEditor::IndexList& idxs);

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
    QMap<Score::Index, ScoreItem*> noteItemMap;
};


ScoreDocument::ScoreDocument()
    : p_        (new Private(this))
{
    p_->initProps();
    p_->initLayout();
    p_->initEditor();
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

    o.insert("version", QJsonValue(2));
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
        const int ver = o.value("version").toInt(1);

        if (ver >= 2)
        {
            tmp.p_->scoreLayout.fromJson(
                    json.expectChildObject(o, "score-layout") );
        }
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

    editor()->setScore(tmpScore);
}


const Score* ScoreDocument::score() const
{
    return p_->score();
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

const PageLayout& ScoreDocument::pageLayout(int pageIdx) const
{
    QString key = layoutKeyForIndex(pageIdx);
    if (p_->pageLayout.contains(key))
        return p_->pageLayout[key];
    else
        return p_->pageLayout["global"];
}

const ScoreLayout& ScoreDocument::scoreLayout(int pageIdx) const
{
    QString key = layoutKeyForIndex(pageIdx);
    if (p_->scoreLayout.contains(key))
        return p_->scoreLayout[key];
    else
        return p_->scoreLayout["global"];
}

const PageAnnotation& ScoreDocument::pageAnnotation(int pageIdx) const
{
    QString key = layoutKeyForIndex(pageIdx);
    if (p_->pageAnnotation.contains(key))
        return p_->pageAnnotation[key];
    else
        return p_->pageAnnotation["global"];
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
    auto i = p_->noteItemMap.find(idx);
    return i == p_->noteItemMap.end() ? nullptr : i.value();
}

ScoreItem* ScoreDocument::getLeftScoreItem(const Score::Index& idx) const
{
    auto i = getScoreItem(idx);
    if (i == nullptr)
        return i;
    auto docIdx = i->docIndex();
    double ma = i->boundingBox().left();
    for (auto& sp : p_->barItems)
    {
        BarItems* items = sp.get();
        if (items->docIndex.pageIdx() == docIdx.pageIdx()
         && items->docIndex.lineIdx() == docIdx.lineIdx())
        {
            for (ScoreItem& sitem : items->items)
            {
                if (sitem.scoreIndex().row() != idx.row())
                    continue;
                if (sitem.boundingBox().left() < ma)
                {
                    ma = sitem.boundingBox().left();
                    i = &sitem;
                }
            }
        }
    }
    return i;
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



/** @todo This is to be templatized */
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


void ScoreDocument::p_setProperties(const QProps::Properties &p)
{
    p_->props = p;
    p_->createItems();
}

void ScoreDocument::p_setPageAnnotation(
        const QString& id, const PageAnnotation &p)
{
    p_->pageAnnotation.insert(id, p);
    p_->createItems();
}

void ScoreDocument::p_setPageLayout(
        const QString& id, const PageLayout &p)
{
    p_->pageLayout.insert(id, p);
    p_->createItems();
}


void ScoreDocument::p_setScoreLayout(
        const QString& id, const ScoreLayout &p)
{
    p_->scoreLayout.insert(id, p);
    p_->createItems();
}


void ScoreDocument::Private::initEditor()
{
    QObject::connect(editor, &ScoreEditor::scoreReset,
            [=](Score* )
    {
        createItems();
    });

    QObject::connect(editor, &ScoreEditor::noteValuesChanged,
            [=](const ScoreEditor::IndexList& idxs)
    {
        updateNoteItems(idxs);
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

void ScoreDocument::Private::updateNoteItems(const ScoreEditor::IndexList &idxs)
{
    for (const auto& idx : idxs)
    if (ScoreItem* i = p->getScoreItem(idx))
    {
        i->updateNote(idx.getNote());
    }
}

void ScoreDocument::Private::createItems()
{
    barItems.clear();
    barItemMap.clear();
    noteItemMap.clear();
    numPages = 0;
    if (!score())
        return;

    Score::Index scoreIdx = score()->index(0,0,0,0);
    if (!scoreIdx.isValid())
        return;

    int page = 0;
    bool isFixed = p->scoreLayout(0).isFixedBarWidth();
    while (createPageItems(page, scoreIdx, isFixed))
    {
        //qDebug() << scoreIdx.toString();
        ++page;
    }
    numPages = page + 1;
}

bool ScoreDocument::Private::createPageItems(
        int pageIdx, Score::Index& scoreIdx, bool isFixed)
{
    QPointF pageOffs;
    for (int line=0; ; ++line)
    {
        ReturnCode ret = createLineOfScore(
                    pageIdx, line, scoreIdx, pageOffs, isFixed);
        if (ret == R_DATA_FINISHED)
            return false;
        else if (ret == R_PAGE_FINISHED)
            return true;
    }
    return false;
}

ScoreDocument::Private::ReturnCode
ScoreDocument::Private::createLineOfScore(
        int pageIdx, int lineIdx,
        Score::Index& scoreIdx, QPointF& pageOffs, bool isFixed)
{
    const ScoreLayout& slayout = p->scoreLayout(pageIdx);
    const PageLayout& playout = p->pageLayout(pageIdx);
    const QRectF scoreRect = playout.scoreRect();

    const double lineHeight = slayout.lineHeight(scoreIdx.numRows()),
                 lineHeightFull = lineHeight + slayout.lineSpacing();

    // break if page ends
    if (pageOffs.y() + lineHeight >= scoreRect.height() )
        return R_PAGE_FINISHED;

    const double
            noteSize = slayout.noteSize(),
            rowSpace = slayout.rowSpacing();

    KeySignature keySig = scoreIdx.getStream().keySignature();

    if (isFixed)
    {
        const size_t numBarsPerLine = slayout.barsPerLine();
        QPROPS_ASSERT(numBarsPerLine > 0, "");
        const double barWidth = scoreRect.width() / numBarsPerLine;

        for (size_t barIdx = 0; barIdx < numBarsPerLine; ++barIdx)
        {
            Index docIdx(pageIdx, lineIdx, barIdx);

            if (!getBarAnnotation(scoreIdx, keySig).isEmpty())
            {
                pageOffs.ry() += noteSize + 2;
                // break if page ends
                if (pageOffs.y() + lineHeight >= scoreRect.height() )
                    return R_PAGE_FINISHED;
            }

            QRectF barRect = QRectF(
                        pageOffs.x() + scoreRect.x() + barIdx * barWidth,
                        pageOffs.y() + scoreRect.y(),
                        barWidth, lineHeight);

            createSingleBarItems(   scoreIdx,
                                    docIdx,
                                    keySig,
                                    barRect,
                                    noteSize,
                                    rowSpace,
                                    barIdx+1 >= numBarsPerLine
                                    );

            // forward index and check for stream change
            size_t curStream = scoreIdx.stream();
            bool finish = !scoreIdx.nextBar(),
                 newStream = (scoreIdx.stream() != curStream);
            if (finish || newStream)
            {
                // next line
                pageOffs.ry() += lineHeightFull;

                return newStream ? R_LINE_FINISHED : R_DATA_FINISHED;
            }
        }
    }
    // non-fixed barwidth
    else
    {
        const double
                noteSpace = slayout.noteSpacing(),
                minBarWidth = slayout.minBarWidth(),
                maxBarWidth = slayout.maxBarWidth();

        // look-ahead into next bars
        Score::Index idx = scoreIdx;
        QList<double> barWidths;
        double width = 0;
        bool isAnno = false;
        while (idx.isValid())
        {
            isAnno |= !getBarAnnotation(idx, keySig).isEmpty();
            int numNotes = idx.getBar().maxNumberNotes();
            double w = std::max(minBarWidth, std::min(maxBarWidth,
                numNotes * noteSize + (numNotes-1) * noteSpace ));
            if (width+w > scoreRect.width()-pageOffs.x())
                break;
            width += w;
            barWidths << w;

            size_t curStream = idx.stream();
            if (!idx.nextBar())
                break;
            if (idx.stream() != curStream)
                break;
        }

        if (barWidths.isEmpty())
        {
            return R_DATA_FINISHED;
        }

        // scale into page width
        for (auto& w : barWidths)
            w = std::max(minBarWidth, std::min(maxBarWidth,
                    w * (scoreRect.width()-pageOffs.x()) / std::max(1., width)
                ));

        if (isAnno)
            pageOffs.ry() += noteSize + 2;

        // break if page ends
        if (pageOffs.y() + lineHeight >= scoreRect.height() )
            return R_PAGE_FINISHED;

        double x = 0.;
        for (int barIdx = 0; barIdx < barWidths.size(); ++barIdx)
        {
            Index docIdx(pageIdx, lineIdx, barIdx);

            QRectF barRect = QRectF(
                        pageOffs.x() + scoreRect.x() + x,
                        pageOffs.y() + scoreRect.y(),
                        barWidths[barIdx], lineHeight);
            x += barWidths[barIdx];

            createSingleBarItems(   scoreIdx,
                                    docIdx,
                                    keySig,
                                    barRect,
                                    noteSize,
                                    rowSpace,
                                    barIdx+1 >= barWidths.size()
                                    );

            if (!scoreIdx.nextBar())
                return R_DATA_FINISHED;
        }

    }

    // next line
    pageOffs.ry() += lineHeightFull;

    return R_ALL_GOOD;
}

QString ScoreDocument::Private::getBarAnnotation(
        const Score::Index &scoreIdx, const KeySignature& keySig)
{
    QString anno;

    // annotation at stream start
    if (scoreIdx.isFirstBar())
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
    if (scoreIdx.isFirstBar() || scoreIdx.isTempoChange())
    {
        if (!anno.isEmpty())
            anno += " ";
        anno += QString("%1bpm").arg(scoreIdx.getBeatsPerMinute());
    }

    return anno;
}

ScoreDocument::BarItems* ScoreDocument::Private::createSingleBarItems(
        const Score::Index& scoreIdx,
        const ScoreDocument::Index& docIdx,
        const KeySignature& keySig,
        const QRectF& barRect,
        double noteSize, double rowSpace,
        bool endOfLine)
{
    // container for one bar block
    auto items = new BarItems;
    items->docIndex = docIdx;
    items->scoreIndex = scoreIdx;


    QString anno = getBarAnnotation(scoreIdx, keySig);

    // print annotation
    if (!anno.isEmpty())
    {
        QRectF rect = barRect;
        const double si = noteSize;
        rect.setTop(rect.top() - si - 2);
        rect.setHeight(si);
        items->items.push_back(
            ScoreItem(scoreIdx, docIdx, rect, anno) );
    }


    // create item for each note in this bar block
    for (size_t row=0; row<scoreIdx.numRows(); ++row)
    {
        double y = barRect.top() + rowSpace * row;

        const Notes& b = scoreIdx.getNotes(row);
        for (size_t col=0; col<b.length(); ++col)
        {
            Score::Index idx = scoreIdx.offset(row, col);
            Note note = idx.getNote();
            //note = keySig.transform(note);
            if (!note.isValid())
                continue;

            double x = barRect.left()
                     + barRect.width() * b.columnTime(col+.5);

            QRectF rect(x-noteSize/2., y, noteSize, noteSize);
            items->items.push_back(
                        ScoreItem(idx, docIdx, rect, note) );
            noteItemMap.insert(idx, &items->items.back() );

        }
    }

    // leading bar-slash item
    double x = barRect.x(),
           y = barRect.y();
    QLineF line(x, y, x, y + barRect.height());
    items->items.push_back( ScoreItem(scoreIdx, docIdx, line) );

    if (endOfLine || scoreIdx.isLastBar())
    {
        // end-of-line bar-slash item
        double x = barRect.right(),
               y = barRect.y();
        QLineF line(x, y, x, y + barRect.height());
        items->items.push_back( ScoreItem(scoreIdx, docIdx, line) );

        // end-of-part bar-slash item
        if (scoreIdx.isLastBar())
        {
            line.translate(-1,0);
            items->items.push_back( ScoreItem(scoreIdx, docIdx, line) );
        }
    }


    // get bounding rect
    if (!items->items.isEmpty())
    {
        items->boundingBox = items->items.at(0).boundingBox();
        for (const ScoreItem& i : items->items)
            items->boundingBox |= i.boundingBox();
    }

    // store/reference all bar items
    std::shared_ptr<BarItems> pit(items);
    barItems.push_back(pit);
    barItemMap.insert(items->docIndex, items);

    return items;
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

QList<QRectF> ScoreDocument::getSelectionRects(int pageIdx,
                                      const Score::Selection &s) const
{
    QList<QRectF> rects;

    auto i1 = getScoreItem(s.from()),
         i2 = getScoreItem(s.to());
    if (i1 && i2)
    {
        if (i1->docIndex().pageIdx() == i2->docIndex().pageIdx())
        {
            if (i1->docIndex().pageIdx() == pageIdx)
            {
                if (i1->docIndex().lineIdx() == i2->docIndex().lineIdx())
                {
                    rects << (i1->boundingBox() | i2->boundingBox());
                }
                else
                for (int i=i1->docIndex().lineIdx();
                     i <= i2->docIndex().lineIdx(); ++i)
                {

                    if (auto leftItem = getLeftScoreItem(s.from()))
                        rects << (leftItem->boundingBox() | i2->boundingBox());
                }
            }
        }
    }
    /*
    for (auto& sp : p_->barItems)
    {
        BarItems* items = sp.get();
        if (items->docIndex.p_pageIdx != pageIdx)
            continue;
    }*/

    return rects;
}


} // namespace Sonot
