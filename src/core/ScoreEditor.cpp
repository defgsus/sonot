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

#include <functional>
#include <memory>

#include <QMimeData>

#include "QProps/error.h"
#include "QProps/JsonInterfaceHelper.h"

#include "ScoreEditor.h"
#include "Notes.h"
#include "NoteStream.h"

#ifdef SONOT_GUI
#   include "gui/ScoreDocument.h"
#endif

#if 0
#   include <QDebug>
#   define SONOT__DEBUG(arg__) { qDebug().noquote().nospace() \
        << "ScoreEditor::" << arg__; }
#else
#   define SONOT__DEBUG(unused__) { }
#endif


namespace Sonot {

struct ScoreEditor::Private
{
    Private(ScoreEditor* p)
        : p                 (p)
        , score_            (nullptr)
        , undoDataPos       (0)
        , doUndo      (true)
        , doUndoMerge       (true)
    { }

    struct UndoData
    {
        QString name, detail;
        std::function<void()> undo, redo;
    };

    Score* score() const { return score_; }

    /** Returns writeable NoteStream */
    NoteStream* getStream(const Score::Index& i, bool check = true);
    /** Returns writeable Bar */
    Bar* getBar(const Score::Index& i);
    /** Returns writeable Notes */
    Notes* getNotes(const Score::Index& i);

    /** Location string for undo name.
        @note this merges row/column undo actions into one */
    static QString barString(const Score::Index& idx)
        { return tr("part %1 bar %2").arg(idx.stream()).arg(idx.bar()); }
    /** Location string for undo name.
        @note this merges undo actions per stream into one */
    static QString partString(const Score::Index& idx)
        { return tr("part %1").arg(idx.stream()); }
    void addUndoData(UndoData*);
    void addBarChangeUndoData(const Score::Index& idx,
                              const Bar& newBar, const Bar& oldBar,
                              const QString& desc, const QString& detail);
    void addStreamChangeUndoData(const Score::Index& idx,
                                 const NoteStream& newStream,
                                 const NoteStream& oldStream,
                                 const QString& desc, const QString& detail);

    void setScore(const Score& s);
    void setStreamProperties(
            size_t streamIdx, const QProps::Properties& p);
    void setScoreProperties(const QProps::Properties&);
#ifdef SONOT_GUI
    void setDocumentProperties(const QProps::Properties&);
    void setPageLayout(const QString& id, const PageLayout&);
    void setScoreLayout(const QString& id, const ScoreLayout&);
#endif
    bool changeBar(const Score::Index& idx, const Bar& b);
    bool changeStream(size_t streamIdx, const NoteStream& s);
    bool insertBar(const Score::Index& idx, const Bar& bar, bool after);
    bool insertRow(const Score::Index& idx, bool after);
    bool insertStream(const Score::Index& idx, const NoteStream& s, bool after);
    bool deleteBar(const Score::Index& idx);
    bool deleteRow(const Score::Index& idx);
    bool deleteStream(const Score::Index& idx);
    bool transpose(const Score::Selection& sel, int steps, bool wholeSteps);

    ScoreEditor* p;

    Score* score_;
#ifdef SONOT_GUI
    ScoreDocument* document;
#endif
    QList<std::shared_ptr<UndoData>> undoData;
    int undoDataPos;
    bool doUndo, doUndoMerge;
};

#ifdef SONOT_GUI
ScoreEditor::ScoreEditor(ScoreDocument* doc, QObject *parent)
    : QObject   (parent)
    , p_        (new Private(this))
{
    QPROPS_ASSERT(doc, "No ScoreDocument given for ScoreEditor instance");
    p_->document = doc;
}
#else
ScoreEditor::ScoreEditor(QObject *parent)
    : QObject   (parent)
    , p_        (new Private(this))
{
}
#endif

ScoreEditor::~ScoreEditor()
{
    delete p_;
}

Score* ScoreEditor::score() const { return p_->score(); }
#ifdef SONOT_GUI
ScoreDocument* ScoreEditor::document() const { return p_->document; }
#endif

// ########################### UNDO/REDO #############################

void ScoreEditor::Private::addUndoData(UndoData* d)
{
    SONOT__DEBUG("Private::addUndoData('" << d->detail << "')");

    if (!doUndo)
    {
        delete d;
        return;
    }

    // clear redo-data
    while (undoDataPos < undoData.size())
        undoData.removeLast();

    if (doUndoMerge)
    if (!undoData.isEmpty())
    {
        if (undoData.last()->name == d->name)
        {
            undoData.last()->redo = d->redo;
            undoData.last()->detail = d->detail;
            delete d;
            emit p->undoAvailable(true, undoData.last()->name,
                                        undoData.last()->detail);
            return;
        }
    }
    undoData << std::shared_ptr<UndoData>(d);
    undoDataPos = undoData.size();

    emit p->undoAvailable(true, undoData.last()->name,
                                undoData.last()->detail);
}

void ScoreEditor::Private::addBarChangeUndoData(
        const Score::Index& idx_, const Bar& newBar, const Bar& oldBar,
        const QString& desc, const QString& detail)
{
    SONOT__DEBUG("Private::addBarChangeUndoData(" << idx.toString()
                 << ", '" << detail << "')");

    auto idx = idx_.topLeft();

    auto stream = getStream(idx);
    QPROPS_ASSERT(stream, "in ScoreEditor::Private::addBarChangeUndoData()");
    size_t orgRows = stream->numRows();

    auto undo = new UndoData();
    undo->name = desc;
    undo->detail = detail;
    undo->undo = [=]()
    {
        auto stream = getStream(idx);
        QPROPS_ASSERT(stream,
                      "in ScoreEditor::Private::addBarChangeUndoData():undo");
        bool rowChange = orgRows != stream->numRows();
        if (rowChange)
            stream->setNumRows(orgRows);
        changeBar(idx, oldBar);
        if (rowChange)
            emit p->streamsChanged(IndexList() << idx);
        emit p->cursorChanged(idx_);
    };
    undo->redo = [=]()
    {
        changeBar(idx, newBar);
        emit p->cursorChanged(idx_);
    };
    addUndoData(undo);
}

void ScoreEditor::Private::addStreamChangeUndoData(
        const Score::Index& idx,
        const NoteStream& newStream, const NoteStream& oldStream,
        const QString& desc, const QString& detail)
{
    SONOT__DEBUG("Private::addStreamChangeUndoData(" << idx.toString()
                 << ", '" << detail << "')");

    auto undo = new UndoData();
    undo->name = desc;
    undo->detail = detail;
    undo->undo = [=]()
    {
        changeStream(idx.stream(), oldStream);
        emit p->cursorChanged(idx);
    };
    undo->redo = [=]()
    {
        changeStream(idx.stream(), newStream);
        emit p->cursorChanged(idx);
    };
    addUndoData(undo);
}

bool ScoreEditor::undo()
{
    SONOT__DEBUG("undo()");

    if (p_->undoData.isEmpty())
        return false;
    if (p_->undoDataPos > p_->undoData.size())
        return false;
    if (p_->undoDataPos <= 0)
        return false;

    QPROPS_ASSERT(p_->undoData[p_->undoDataPos-1]->undo,
                  "No undo action defined");
    try
    {
        p_->undoData[p_->undoDataPos-1]->undo();
    }
    catch (QProps::Exception& e)
    {
        e << "\nFor undo action '"
          << p_->undoData[p_->undoDataPos-1]->detail << "'";
        throw;
    }

    --p_->undoDataPos;
    if (p_->undoDataPos > 0)
        emit undoAvailable(true, p_->undoData[p_->undoDataPos-1]->name,
                                 p_->undoData[p_->undoDataPos-1]->detail);
    else
        emit undoAvailable(false, "", "");
    if (p_->undoDataPos < p_->undoData.size())
        emit redoAvailable(true, p_->undoData[p_->undoDataPos]->name,
                                 p_->undoData[p_->undoDataPos]->detail);
    else
        emit redoAvailable(false, "", "");
    return true;
}

bool ScoreEditor::redo()
{
    SONOT__DEBUG("redo()");

    if (p_->undoData.isEmpty())
        return false;
    if (p_->undoDataPos >= p_->undoData.size())
        return false;

    QPROPS_ASSERT(p_->undoData[p_->undoDataPos]->redo,
                  "No redo action defined");
    try
    {
        p_->undoData[p_->undoDataPos]->redo();
    }
    catch (QProps::Exception& e)
    {
        e << "\nFor redo action '"
          << p_->undoData[p_->undoDataPos]->detail << "'";
        throw;
    }

    ++p_->undoDataPos;
    if (p_->undoDataPos > 0)
        emit undoAvailable(true, p_->undoData[p_->undoDataPos-1]->name,
                                 p_->undoData[p_->undoDataPos-1]->detail);
    else
        emit undoAvailable(false, "", "");
    if (p_->undoDataPos < p_->undoData.size())
        emit redoAvailable(true, p_->undoData[p_->undoDataPos]->name,
                                 p_->undoData[p_->undoDataPos]->detail);
    else
        emit redoAvailable(false, "", "");
    return true;
}

void ScoreEditor::clearUndo()
{
    p_->undoData.clear();
    p_->undoDataPos = 0;
    emit undoAvailable(false, "", "");
    emit redoAvailable(false, "", "");
}

void ScoreEditor::setEnableUndo(bool enable)
{
    p_->doUndo = enable;
}

void ScoreEditor::setMergeUndo(bool enable)
{
    p_->doUndoMerge = enable;
}





// ##################### editing ###########################

#define SONOT__CHECK_INDEX_INSTANCE(i__, ret__, arg__) \
    QPROPS_ASSERT(score(), "no Score assigned to ScoreEditor " << arg__); \
    QPROPS_ASSERT(i__.score() == score(), \
     "Score pointer in index " << i__.toString() \
     << " not matching ScoreEditor " << arg__);

#define SONOT__CHECK_INDEX(i__, ret__, arg__) \
    SONOT__CHECK_INDEX_INSTANCE(i__, ret__, arg__); \
    QPROPS_ASSERT(i__.isValid(), \
     "invalid index " << i__.toString() << " used with ScoreEditor, " \
     << score()->toInfoString() << " " << arg__ ); \
    if (!i__.isValid() || i__.score() != score()) return ret__;

#define SONOT__CHECK_SELECTION(i__, ret__, arg__) \
    SONOT__CHECK_INDEX(i__.from(), ret__, arg__); \
    SONOT__CHECK_INDEX(i__.to(), ret__, arg__);


void ScoreEditor::setScore(const Score& newScore)
{
    SONOT__DEBUG("setScore()");

    if (p_->doUndo)
    {
        auto undo = new Private::UndoData();
        undo->name = tr("set score");
        undo->detail = undo->name;
        if (p_->score_)
        {
            Score copy(*p_->score_);
            undo->undo = [=]()
            {
                p_->setScore(copy);
            };
        }
        else
        {
            undo->undo = [=]()
            {
                p_->setScore(Score());
            };
        }
        undo->redo = [=]()
        {
            p_->setScore(newScore);
        };
        p_->addUndoData(undo);
    }

    p_->setScore(newScore);
}

void ScoreEditor::Private::setScore(const Score& s)
{
    SONOT__DEBUG("Private::setScore(" << &s << ")");
    if (!score_)
        score_ = new Score(s);
    else
        *score_ = s;
    emit p->scoreReset(score_);
}

bool ScoreEditor::setStreamProperties(
        size_t streamIdx, const QProps::Properties& newProps)
{
    SONOT__DEBUG("setStreamProperties()");

    if (!score())
        return false;

    auto idx = score()->index(streamIdx, 0,0,0);
    if (NoteStream* stream = p_->getStream(idx))
    {
        if (p_->doUndo)
        {
            auto undo = new Private::UndoData();
            undo->name = tr("change part properties (%1)").arg(idx.stream());
            undo->detail = tr("change part properties %1").arg(idx.toString());
            auto copy = stream->props();
            undo->undo = [=]()
            {
                p_->setStreamProperties(streamIdx, copy);
            };

            undo->redo = [=]()
            {
                p_->setStreamProperties(streamIdx, newProps);
            };
            p_->addUndoData(undo);
        }

        p_->setStreamProperties(streamIdx, newProps);
        return true;
    }
    return false;
}

void ScoreEditor::Private::setStreamProperties(
        size_t streamIdx, const QProps::Properties& prop)
{
    SONOT__DEBUG("Private::setStreamProperties(" << streamIdx << ")");

    auto idx = score()->index(streamIdx, 0,0,0);
    if (NoteStream* stream = getStream(idx))
    {
        stream->setProperties(prop);
        emit p->streamPropertiesChanged(IndexList() << idx);
        emit p->documentChanged();
        emit p->refresh();
    }
}

void ScoreEditor::setScoreProperties(const QProps::Properties& newProps)
{
    SONOT__DEBUG("setScoreProperties()");

    if (!score())
        return;

    if (p_->doUndo)
    {
        auto undo = new Private::UndoData();
        undo->name = undo->detail = tr("change score properties");
        auto copy = score()->props();
        undo->undo = [=]()
        {
            p_->setScoreProperties(copy);
        };

        undo->redo = [=]()
        {
            p_->setScoreProperties(newProps);
        };
        p_->addUndoData(undo);
    }

    p_->setScoreProperties(newProps);
}

void ScoreEditor::Private::setScoreProperties(
        const QProps::Properties& newProps)
{
    SONOT__DEBUG("Private::setScoreProperties()");
    if (score())
    {
        score()->setProperties(newProps);
        emit p->scorePropertiesChanged();
        emit p->documentChanged();
        emit p->refresh();
    }
}

#ifdef SONOT_GUI

void ScoreEditor::setDocumentProperties(const QProps::Properties& newProps)
{
    SONOT__DEBUG("setDocumentProperties()");

    if (!score())
        return;

    if (p_->doUndo)
    {
        auto undo = new Private::UndoData();
        undo->name = undo->detail = tr("change document properties");
        auto copy = document()->props();
        undo->undo = [=]()
        {
            p_->setDocumentProperties(copy);
        };

        undo->redo = [=]()
        {
            p_->setDocumentProperties(newProps);
        };
        p_->addUndoData(undo);
    }

    p_->setDocumentProperties(newProps);
}

void ScoreEditor::Private::setDocumentProperties(
        const QProps::Properties& newProps)
{
    SONOT__DEBUG("Private::setDocumentProperties()");
    if (score())
    {
        document->p_setProperties(newProps);
        emit p->documentPropertiesChanged();
        emit p->documentChanged();
        emit p->refresh();
    }
}

void ScoreEditor::setPageLayout(
        const QString& id, const PageLayout& layout)
{
    SONOT__DEBUG("setPageLayout(" << id << ")");

    if (!score())
        return;

    if (p_->doUndo)
    {
        auto undo = new Private::UndoData();
        undo->name = tr("change page layout");
        undo->detail = tr("change page layout %1").arg(id);
        auto copy = document()->pageLayout(id);
        undo->undo = [=]()
        {
            p_->setPageLayout(id, copy);
        };

        undo->redo = [=]()
        {
            p_->setPageLayout(id, layout);
        };
        p_->addUndoData(undo);
    }

    p_->setPageLayout(id, layout);
}

void ScoreEditor::Private::setPageLayout(
        const QString& id, const PageLayout& newProps)
{
    SONOT__DEBUG("Private::setPageLayout(" << id << ")");
    if (score())
    {
        document->p_setPageLayout(id, newProps);
        emit p->pageLayoutChanged(id);
        emit p->documentChanged();
        emit p->refresh();
    }
}

void ScoreEditor::setScoreLayout(
        const QString& id, const ScoreLayout& layout)
{
    SONOT__DEBUG("setScoreLayout(" << id << ")");

    if (!score())
        return;

    if (p_->doUndo)
    {
        auto undo = new Private::UndoData();
        undo->name = tr("change score layout");
        undo->detail = tr("change score layout %1").arg(id);
        auto copy = document()->scoreLayout(id);
        undo->undo = [=]()
        {
            p_->setScoreLayout(id, copy);
        };

        undo->redo = [=]()
        {
            p_->setScoreLayout(id, layout);
        };
        p_->addUndoData(undo);
    }

    p_->setScoreLayout(id, layout);
}

void ScoreEditor::Private::setScoreLayout(
        const QString& id, const ScoreLayout& newProps)
{
    SONOT__DEBUG("Private::setScoreLayout(" << id << ")");
    if (score())
    {
        document->p_setScoreLayout(id, newProps);
        emit p->scoreLayoutChanged(id);
        emit p->documentChanged();
        emit p->refresh();
    }
}

#endif


bool ScoreEditor::insertNote(
        const Score::Index& idx, const Note& n, bool allRows)
{
    SONOT__DEBUG("insertNote(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in insertNote");
    if (!allRows)
    {
        if (Notes* notes = p_->getNotes(idx))
        {
            Bar oldBar = *p_->getBar(idx);
            notes->insertNote(idx.column(), n);
            emit barsChanged(IndexList() << idx);
            emit documentChanged();
            p_->addBarChangeUndoData(idx, *p_->getBar(idx), oldBar,
                                     tr("insert note in %1")
                                     .arg(Private::barString(idx)),
                                     tr("insert note at %1")
                                     .arg(idx.toString()));
            return true;
        }
    }
    else
    {
        bool changed = false;
        NoteStream* s = p_->getStream(idx);
        auto oldBarP = p_->getBar(idx);
        if (!oldBarP)
            return false;
        Bar oldBar = *oldBarP;
        for (size_t i=0; i<s->numRows(); ++i)
        {
            auto x = score()->index(idx.stream(),
                                    idx.bar(),
                                    i,
                                    idx.column());
            x = x.limitRight();
            if (Notes* bar = p_->getNotes(x))
            {
                bar->insertNote(idx.column(), n);
                changed = true;
            }
        }
        if (changed)
        {
            emit barsChanged(IndexList() << idx);
            emit documentChanged();
            p_->addBarChangeUndoData(idx, *p_->getBar(idx), oldBar,
                                     tr("insert note in %1")
                                     .arg(Private::barString(idx)),
                                     tr("insert note at %1")
                                     .arg(idx.toString()));
            return true;
        }
    }
    return false;
}

bool ScoreEditor::insertBar(
        const Score::Index& idx_, const Bar& bar, bool after)
{
    SONOT__DEBUG("insertBar(" << idx_.toString() << ")");

    SONOT__CHECK_INDEX(idx_, false, "in insertBar");
    auto idx = idx_.topLeft();
    NoteStream* stream = p_->getStream(idx);
    if (!stream)
        return false;

    if (p_->doUndo)
    {
        QString undoName = tr("insert bar in %1").arg(Private::partString(idx)),
                undoDetail = tr("insert bar %1 %2")
                                .arg(after ? "after" : "at")
                                .arg(idx.toString());
        if (!p_->doUndoMerge)
        {
            auto undo = new Private::UndoData();
            undo->name = undoName;
            undo->detail = undoDetail;

            int orgNumRows = -1;
            if (stream->numRows() < bar.numRows())
                orgNumRows = stream->numRows();

            undo->undo = [=]()
            {
                auto index = p_->score()->index(idx.stream(),
                                                idx.bar() + (after ? 1 : 0),
                                                0, 0);
                if (orgNumRows >= 0)
                {
                    NoteStream* stream = p_->getStream(index);
                    QPROPS_ASSERT(stream, "no stream for " << index.toString()
                                  << " in insertBar-undo-action");
                    SONOT__DEBUG("insertBar(" << idx_.toString()
                                 << "):undo:changing "
                                 "numRows from " << stream->numRows()
                                 << " to " << orgNumRows);
                    stream->setNumRows(orgNumRows);
                    //^ Note: p_->deleteBar() emits streamsChanged()
                    //        so this is safe in terms of GUI update
                }
                p_->deleteBar(index);
            };
            undo->redo = [=]()
            {
                p_->insertBar(idx, bar, after);
            };
            p_->addUndoData(undo);
        }
        else
        {
            auto oldStream = *stream;
            if (!p_->insertBar(idx, bar, after))
                return false;
            p_->addStreamChangeUndoData(idx, *stream, oldStream,
                                        undoName, undoDetail);
            return true;
        }
    }

    return p_->insertBar(idx, bar, after);
}

bool ScoreEditor::Private::insertBar(
        const Score::Index& idx, const Bar& bar, bool after)
{
    SONOT__DEBUG("Private::insertBar("
                 << idx.toString() << ", after=" << after << ")");

    SONOT__CHECK_INDEX_INSTANCE(idx, false, "in Private::insertBar");
    QPROPS_ASSERT_LT(idx.stream(), score()->numNoteStreams(),
                  "stream index out of range");
    NoteStream* stream = getStream(idx, false);
    if (!stream)
        return false;

    stream->insertBar(idx.bar() + (after ? 1 : 0), bar);
    emit p->streamsChanged(IndexList() << idx);
    emit p->documentChanged();
    return true;
}

bool ScoreEditor::insertBars(
        const Score::Index& idx, const NoteStream& stream, bool after)
{
    SONOT__DEBUG("insertBars(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in insertBars");
    if (NoteStream* dst = p_->getStream(idx))
    {
        size_t iidx = idx.bar() + (after ? 1 : 0);
        for (size_t b = 0; b < stream.numBars(); ++b)
        {
            dst->insertBar(iidx, stream.bar(b));
            iidx++;
        }
        emit streamsChanged(IndexList() << idx);
        emit documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::insertRow(const Score::Index& idx, bool after)
{
    SONOT__DEBUG("insertRow(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in insertRow");
    auto stream = p_->getStream(idx);
    if (!stream)
        return false;

    if (p_->doUndo)
    {
        QString undoName = QString("insert row in %1")
                            .arg(Private::partString(idx)),
                undoDetail = QString("insert row %1 %2")
                .arg(after ? "after" : "before").arg(idx.toString());

        if (!p_->doUndoMerge)
        {
            auto undo = new Private::UndoData();
            undo->name = undoName;
            undo->detail = undoDetail;
            undo->undo = [=]()
            {
                auto index = idx;
                if (after)
                    index.nextRow();
                p_->deleteRow(index);
            };
            undo->redo = [=]()
            {
                p_->insertRow(idx, after);
            };
            p_->addUndoData(undo);
        }
        else
        {
            auto oldStream = *stream;
            if (!p_->insertRow(idx, after))
                return false;
            p_->addStreamChangeUndoData(idx, *stream, oldStream,
                                     undoName, undoDetail);
            return true;
        }
    }

    return p_->insertRow(idx, after);
}

bool ScoreEditor::Private::insertRow(const Score::Index& idx, bool after)
{
    SONOT__DEBUG("Private::insertRow("
                 << idx.toString() << ", after=" << after << ")");

    SONOT__CHECK_INDEX(idx, false, "in Private::insertRow");
    if (NoteStream* stream = getStream(idx))
    {
        stream->insertRow(idx.row() + (after ? 1 : 0));
        emit p->streamsChanged(IndexList() << idx);
        emit p->documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::insertStream(
        const Score::Index& idx_, const NoteStream& s, bool after)
{
    SONOT__DEBUG("insertStream(" << idx_.toString() << ")");
    auto idx = idx_.streamTopLeft();

    SONOT__CHECK_INDEX(idx, false, "in insertStream");

    if (p_->doUndo)
    {
        QString undoName = tr("insert part"),
                undoDetail = tr("insert part %1 %2")
                            .arg(after ? "after" : "at").arg(idx.toString());
        if (!p_->doUndoMerge)
        {
            auto undo = new Private::UndoData();
            undo->name = undoName;
            undo->detail = undoDetail;
            undo->undo = [=]()
            {
                Score::Index index = idx;
                if (after)
                    index.nextStream();
                p_->deleteStream(index);
                emit cursorChanged(idx);
            };
            undo->redo = [=]()
            {
                p_->insertStream(idx, s, after);
                emit cursorChanged(idx);
            };
            p_->addUndoData(undo);
        }
        else
        {
            Score copy(*score());
            auto undo = new Private::UndoData();
            undo->name = undoName;
            undo->detail = undoDetail;
            undo->undo = [=]()
            {
                p_->setScore(copy);
                emit cursorChanged(idx_);
            };
            if (!p_->insertStream(idx, s, after))
            {
                delete undo;
                return false;
            }
            copy = *score();
            undo->redo = [=]()
            {
                p_->setScore(copy);
                emit cursorChanged(idx_);
            };
            p_->addUndoData(undo);
            return true;
        }
    }

    return p_->insertStream(idx, s, after);
}

bool ScoreEditor::Private::insertStream(
        const Score::Index& idx, const NoteStream& s, bool after)
{
    SONOT__DEBUG("Private::insertStream(" << idx.toString() << ")");

    SONOT__CHECK_INDEX_INSTANCE(idx, false, "in Private::insertStream");

    size_t iidx = idx.stream() + (after ? 1 : 0);
    if (iidx > score()->numNoteStreams())
        iidx = score()->numNoteStreams();

    score()->insertNoteStream(iidx, s);

    IndexList list;
    for (size_t i = iidx; i < score()->numNoteStreams(); ++i)
        list << score()->index(i, 0,0,0);
    emit p->streamsChanged(list);
    emit p->documentChanged();
    return true;
}

bool ScoreEditor::insertScore(
        const Score::Index& idx, const Score& s, bool after)
{
    SONOT__DEBUG("insertScore(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in insertScore");
    if (idx.stream() < score()->numNoteStreams())
    {
        size_t iidx = idx.stream() + (after ? 1 : 0);
        for (size_t i = 0; i<s.numNoteStreams(); ++i)
        {
            score()->insertNoteStream(iidx + i, s.noteStream(i));
        }

        IndexList list;
        for (size_t i = iidx; i < score()->numNoteStreams(); ++i)
            list << score()->index(i, 0,0,0);
        emit streamsChanged(list);
        emit documentChanged();
        return true;
    }
    return false;
}


bool ScoreEditor::deleteRow(const Score::Index& idx)
{
    SONOT__DEBUG("deleteRow(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in deleteRow");
    auto stream = p_->getStream(idx);
    if (!stream)
        return false;

    if (p_->doUndo)
    {
        QString undoName = QString("delete row in %1")
                    .arg(Private::partString(idx)),
                undoDetail = QString("delete row %1").arg(idx.toString());
        if (!p_->doUndoMerge)
        {
            auto undo = new Private::UndoData();
            undo->name = undoName;
            undo->detail = undoDetail;
            auto copy = *stream;
            undo->undo = [=]()
            {
                p_->changeStream(idx.stream(), copy);
                emit cursorChanged(idx);
            };
            undo->redo = [=]()
            {
                p_->deleteRow(idx);
                emit cursorChanged(idx);
            };
            p_->addUndoData(undo);
        }
        else
        {
            auto oldStream = *stream;
            if (!p_->deleteRow(idx))
                return false;
            p_->addStreamChangeUndoData(idx, *stream, oldStream,
                                        undoName, undoDetail);
            return true;
        }
    }

    return p_->deleteRow(idx);
}

bool ScoreEditor::Private::deleteRow(const Score::Index& idx)
{
    SONOT__DEBUG("Private::deleteRow(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in Private::deleteRow");
    if (NoteStream* stream = getStream(idx))
    {
        stream->removeRow(idx.row());
        emit p->streamsChanged(IndexList() << idx);
        emit p->documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::changeNote(const Score::Index& idx, const Note& n)
{
    SONOT__DEBUG("changeNote(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in changeNote");

    if (Bar* bar = p_->getBar(idx))
    if (Notes* notes = p_->getNotes(idx))
    {
        Bar oldBar = *bar;
        notes->setNote(idx.column(), n);
        emit noteValuesChanged(IndexList() << idx);
        emit documentChanged();

        if (p_->doUndo)
        {
            // store the whole bar so merging undo actions
            // will keep all changed notes
            p_->addBarChangeUndoData(idx, *bar, oldBar,
                                     tr("change note in %1")
                                     .arg(Private::barString(idx)),
                                     tr("change note %1").arg(idx.toString()));
        }
        return true;
    }
    return false;
}

bool ScoreEditor::changeBar(const Score::Index& idx_, const Bar& b)
{
    SONOT__DEBUG("changeBar(" << idx.toString() << ")");

    auto idx = idx_.topLeft();

    NoteStream* stream = p_->getStream(idx);
    Bar* bar = p_->getBar(idx);
    if (!stream || !bar)
        return false;

    if (p_->doUndo)
    {
        if (b.numRows() != stream->numRows() && p_->doUndoMerge)
        {
            auto oldStream = *stream;
            if (!p_->changeBar(idx, b))
                return false;
            p_->addStreamChangeUndoData(idx, *stream, oldStream,
                                        tr("change bar %1:%2")
                                        .arg(idx.stream()).arg(idx.bar()),
                                        tr("change bar %1")
                                        .arg(idx.toString()));
            return true;
        }
        p_->addBarChangeUndoData(idx, b, *bar,
                                 tr("change bar %1:%2")
                                 .arg(idx.stream()).arg(idx.bar()),
                                 tr("change bar %1").arg(idx.toString()));
    }

    return p_->changeBar(idx, b);
}

bool ScoreEditor::Private::changeBar(const Score::Index& idx, const Bar& b)
{
    SONOT__DEBUG("Private::changeBar(" << idx.toString() << ")");

    if (NoteStream* stream = getStream(idx))
    if (Bar* bar = getBar(idx))
    {
        if (b.numRows() < stream->numRows())
        {
            Bar b2 = b;
            b2.resize(stream->numRows());
            *bar = b2;
            emit p->barsChanged(IndexList() << idx);
        }
        else if (b.numRows() > stream->numRows())
        {
            stream->setNumRows(b.numRows());
            *bar = b;
            emit p->streamsChanged(IndexList() << idx);
        }
        else
        {
            *bar = b;
            emit p->barsChanged(IndexList() << idx);
        }
        emit p->documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::Private::changeStream(size_t streamIdx, const NoteStream& s)
{
    SONOT__DEBUG("Private::changeStream(" << streamIdx << ", "
                 << s.numBars() << "x" << s.numRows() << ")");

    if (!score() || streamIdx >= score()->numNoteStreams())
        return false;

    score()->setNoteStream(streamIdx, s);
    emit p->streamsChanged(IndexList() << score()->index(streamIdx,0,0,0));
    emit p->documentChanged();
    return true;
}

bool ScoreEditor::transpose(
        const Score::Selection& sel, int steps, bool wholeSteps)
{
    SONOT__DEBUG("transpose(" << sel.toString()
                 << ", steps=" << steps << ", whole=" << wholeSteps << ")");

    SONOT__CHECK_SELECTION(sel, false, "in transpose");

    /** @todo This is quite inefficient.
        The whole score is saved because i'm too lazy right now
        to fine-tune the granularity.
        The selection can contain a single note up to the whole score..
        It's probably best to create a generic function that creates
        undo data for the portion contained in a selection! */
    if (p_->doUndo)
    {
        Score copy(*score());
        auto undo = new Private::UndoData();
        undo->name = tr("transpose");
        undo->detail = tr("transpose %1 by %2")
                .arg(sel.toString()).arg(steps);
        undo->undo = [=]()
        {
            p_->setScore(copy);
        };
        undo->redo = [=]()
        {
            p_->transpose(sel, steps, wholeSteps);
        };
        if (!p_->transpose(sel, steps, wholeSteps))
        {
            delete undo;
            return false;
        }
        p_->addUndoData(undo);
        return true;
    }

    return p_->transpose(sel, steps, wholeSteps);
}

bool ScoreEditor::Private::transpose(
        const Score::Selection& sel, int steps, bool wholeSteps)
{
    SONOT__DEBUG("Private::transpose(" << sel.toString()
                 << ", steps=" << steps << ", whole=" << wholeSteps << ")");

    if (steps == 0)
        return false;
    auto indices = sel.containedNoteIndices();
    if (indices.isEmpty())
        return false;

    for (Score::Index& idx : indices)
    {
        idx.setNote( idx.getNote().transposed(steps, wholeSteps) );
    }
    emit p->noteValuesChanged(indices);
    emit p->documentChanged();
    return true;
}

bool ScoreEditor::deleteNote(const Score::Index& idx, bool allRows)
{
    SONOT__DEBUG("deleteNote(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in deleteNote");
    if (Bar* bar = p_->getBar(idx))
    if (Notes* notes = p_->getNotes(idx))
    {
        Bar oldBar = *bar;
        QString undoDesc = tr("delete note%1 %2")
                .arg(allRows ? "-column" : "").arg(idx.toString());

        IndexList list;
        if (!allRows)
        {
            list << idx;
            emit notesAboutToBeDeleted(list);
            notes->removeNote(idx.column());
        }
        else
        {
            for (size_t i=0; i<idx.getStream().numRows(); ++i)
            {
                auto x = score()->index(idx.stream(), idx.bar(),
                                        i, idx.column());
                if (x.isValid())
                    list << x;
            }
            emit notesAboutToBeDeleted(list);
            for (auto& x : list)
            if (Notes* notes = p_->getNotes(x))
            {
                notes->removeNote(x.column());
            }
        }

        emit notesDeleted(list);
        emit documentChanged();

        if (p_->doUndo)
        {
            p_->addBarChangeUndoData(idx, *bar, oldBar,
                                     tr("delete note in %1")
                                     .arg(Private::barString(idx)),
                                     undoDesc);
        }

        return true;
    }
    return false;
}

bool ScoreEditor::deleteBar(const Score::Index& idx_)
{
    SONOT__DEBUG("deleteBar(" << idx_.toString() << ")");

    auto idx = idx_.topLeft();
    SONOT__CHECK_INDEX(idx, false, "in deleteBar");
    Bar* bar = p_->getBar(idx);
    NoteStream* stream = p_->getStream(idx);
    if (!bar || !stream)
        return false;

    if (p_->doUndo)
    {
        QString undoName = tr("delete bar in %1")
                            .arg(Private::partString(idx)),
                undoDetail = tr("delete bar %1").arg(idx.toString());
        if (!p_->doUndoMerge)
        {
            auto undo = new Private::UndoData();
            undo->name = undoName;
            undo->detail = undoDetail;
            Bar copy(*bar);
            undo->undo = [=]()
            {
                /** @todo The call to idx.toString() "fixes" a problem
                    in SonotCoreTest::testUndoRedo().
                    Maybe it's a bug(?) in lambda captures or
                    some optimization thing.. It's really strange,
                    it does not help to enable debug printing because
                    the problem disappears then. */
                idx.toString();
                //qDebug() << "insert" << idx.toString();
                p_->insertBar(idx, copy, false);
            };
            undo->redo = [=]()
            {
                idx.toString();
                //qDebug() << "delete" << idx.toString();
                p_->deleteBar(idx);
            };
            p_->addUndoData(undo);
        }
        else
        {
            auto oldStream = *stream;
            if (!p_->deleteBar(idx))
                return false;
            p_->addStreamChangeUndoData(idx, *stream, oldStream,
                                        undoName, undoDetail);
            return true;
        }
    }

    return p_->deleteBar(idx);
}

bool ScoreEditor::Private::deleteBar(const Score::Index& idx)
{
    SONOT__DEBUG("Private::deleteBar(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in deleteBar");
    if (NoteStream* stream = getStream(idx))
    {
        IndexList list; list << idx;
        emit p->barsAboutToBeDeleted(list);
        stream->removeBar(idx.bar());
        emit p->barsDeleted(list);
        emit p->streamsChanged(list);
        emit p->documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::deleteStream(const Score::Index& idx_)
{
    SONOT__DEBUG("deleteStream(" << idx_.toString() << ")");
    auto idx = idx_.streamTopLeft();

    SONOT__CHECK_INDEX(idx, false, "in deleteStream");
    NoteStream* stream = p_->getStream(idx);
    if (!stream)
        return false;

    if (p_->doUndo)
    {
        auto undo = new Private::UndoData();
        undo->name = tr("delete part %1").arg(idx.stream());
        undo->detail = tr("delete part %1").arg(idx.toString());
        if (!p_->doUndoMerge)
        {
            NoteStream copy(*stream);
            undo->undo = [=]()
            {
                p_->insertStream(idx, copy, false);
                emit cursorChanged(idx_);
            };
            undo->redo = [=]()
            {
                p_->deleteStream(idx);
                emit cursorChanged(idx_);
            };
            p_->addUndoData(undo);
        }
        else
        {
            Score copy(*score());
            undo->undo = [=]()
            {
                p_->setScore(copy);
                emit cursorChanged(idx_);
            };
            if (!p_->deleteStream(idx))
            {
                delete undo;
                return false;
            }
            copy = *score();
            undo->redo = [=]()
            {
                p_->setScore(copy);
                emit cursorChanged(idx_);
            };
            p_->addUndoData(undo);
            return true;
        }
    }

    return p_->deleteStream(idx);
}

bool ScoreEditor::Private::deleteStream(const Score::Index& idx)
{
    SONOT__DEBUG("Private::deleteStream(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in Private::deleteStream");
    if (idx.stream() < score()->numNoteStreams())
    {
        IndexList list; list << idx;
        emit p->streamsAboutToBeDeleted(list);
        score()->removeNoteStream(idx.stream());
        emit p->streamsDeleted(list);
        emit p->documentChanged();
        return true;
    }
    return false;
}

bool ScoreEditor::splitStream(const Score::Index& idx)
{
    SONOT__DEBUG("splitStream(" << idx.toString() << ")");

    SONOT__CHECK_INDEX(idx, false, "in splitStream");
    if (idx.isLastBar())
        return false;
    NoteStream* org = p_->getStream(idx);
    if (!org)
        return false;
    NoteStream cpy = *org;
    cpy.removeBars(0, idx.bar() + 1);
    org->removeBars(idx.bar() + 1);
    score()->insertNoteStream(idx.stream() + 1, cpy);
    // get propper indices after change
    auto i1 = score()->index(idx.stream(), 0,0,0),
         i2 = score()->index(idx.stream()+1, 0,0,0);
    QPROPS_ASSERT(i1.isValid(), "");
    QPROPS_ASSERT(i2.isValid(), "");
    emit streamsChanged(IndexList() << i1 << i2);
    emit documentChanged();
    return true;
}

NoteStream* ScoreEditor::Private::getStream(const Score::Index& idx, bool check)
{
    if (check)
    {
        SONOT__CHECK_INDEX(idx, nullptr, "in Private::getStream");
    }
    else if (idx.stream() >= score_->numNoteStreams())
        return nullptr;
    return const_cast<NoteStream*>(&score_->noteStreams()[idx.stream()]);
}

Bar* ScoreEditor::Private::getBar(const Score::Index& idx)
{
    SONOT__CHECK_INDEX_INSTANCE(idx, false, "in Private::getBar");
    QPROPS_ASSERT_LT(idx.stream(),
                     score()->numNoteStreams(),
                     "in ScoreEditor::Private::getBar");
    QPROPS_ASSERT_LT(idx.bar(), score()->noteStream(idx.stream()).numBars(),
                     "in ScoreEditor::Private::getBar");
    if (idx.stream() >= score()->numNoteStreams())
        return nullptr;
    if (idx.bar() >= score()->noteStream(idx.stream()).numBars())
        return nullptr;

    return const_cast<Bar*>(&score_->noteStreams()[idx.stream()]
                            .bar(idx.bar()));
}

Notes* ScoreEditor::Private::getNotes(const Score::Index& idx)
{
    SONOT__CHECK_INDEX(idx, nullptr, "in Private::getNotes");
    return const_cast<Notes*>(&score_->noteStreams()[idx.stream()]
                            .notes(idx.bar(), idx.row()));
}

bool ScoreEditor::pasteMimeData(const Score::Index& idx,
                                const QMimeData* data, bool after)
{
    SONOT__DEBUG("pasteMimeData(" << idx.toString() << ")");

    if (!data->hasText())
        return false;
    auto doc = QJsonDocument::fromJson(data->data("text/plain"));
    if (doc.isNull())
        return false;

    QProps::JsonInterfaceHelper json("ScoreEditor");

    QJsonObject o = doc.object();

    if (o.contains("bar"))
    {
        Bar bar;
        bar.fromJson( json.expectChildObject(o, "bar") );

        int startRow = o.value("row-start").toInt(-1),
            endRow = o.value("row-end").toInt(-1),
            startCol = o.value("col-start").toInt(-1),
            endCol = o.value("col-end").toInt(-1);
        QPROPS_ASSERT(startRow < 0 || endRow >= 0, "range not fully set");
        QPROPS_ASSERT(startCol < 0 || endCol >= 0, "range not fully set");
        qDebug() << startRow << endRow << startCol << endCol;
        if (startRow >= 0 && endRow >= 0
         && startCol >= 0 && endCol >= 0)
        {
            QPROPS_ASSERT_LT((size_t)startRow, bar.numRows(), "");
            QPROPS_ASSERT_LT((size_t)endRow, bar.numRows(), "");
            Bar* dst = p_->getBar(idx);
            if (!dst)
                return false;
            int maRow = -1, maCol = -1;
            for (int r=startRow; r<=endRow; ++r)
            {
                int row = idx.row() + r - startRow;
                if (row >= 0 && (size_t)row < dst->numRows())
                {
                    QPROPS_ASSERT_LT((size_t)startCol, bar[r].length(), "");
                    QPROPS_ASSERT_LT((size_t)endCol, bar[r].length(), "");
                    maRow = std::max(maRow, row);
                    Notes n = (*dst)[row];
                    for (int c=startCol; c<=endCol; ++c)
                    {
                        int col = idx.column() + c - startCol;
                        if (col >= 0 && (size_t)col < n.length())
                        {
                            maCol = std::max(maCol, col);
                            n.setNote(col, bar[r].note(c));
                        }
                    }
                    dst->setNotes(row, n);
                }
            }
            if (maRow >= 0 && maCol >= 0)
            {
                emit barsChanged(IndexList() << idx);
                emit documentChanged();
                emit pasted(Score::Selection(
                                idx, score()->index(idx.stream(),
                                                    idx.bar(), maRow, maCol)));
                return true;
            }
            return false;
        }
        else
        {
            if (insertBar(idx, bar, after))
            {
                auto k = idx;
                if (after)
                    k.nextBar();
                if (k.isValid())
                {
                    auto i1 = Score::Selection::fromBar(k);
                    if (i1.isValid())
                        emit pasted(i1);
                }
                return true;
            }
            return false;
        }

    }
    else if (o.contains("bars"))
    {
        NoteStream s;
        s.fromJson( json.expectChildObject(o, "bars") );
        if (insertBars(idx, s, after))
        {
            auto i1 = idx.streamTopLeft();
            if (after)
                i1.nextStream();
            auto s1 = Score::Selection::fromStream(i1);
            if (s1.isValid())
                emit pasted(s1);
            return true;
        }
        return false;
    }
    else if (o.contains("stream"))
    {
        NoteStream s;
        s.fromJson( json.expectChildObject(o, "stream") );
        return insertStream(idx, s, after);
    }
    else if (o.contains("score"))
    {
        Score s;
        s.fromJson( json.expectChildObject(o, "score") );
        return insertScore(idx, s, after);
    }

    return false;
}


} // namespace Sonot
