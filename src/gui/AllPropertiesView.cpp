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

#include <QLayout>
#include <QComboBox>

#include "AllPropertiesView.h"
#include "QProps/PropertiesView.h"
#include "ScoreDocument.h"
#include "PageLayout.h"
#include "ScoreLayout.h"

#include "core/ScoreEditor.h"
#include "core/NoteStream.h"

#include "audio/SynthDevice.h"


namespace Sonot {


struct AllPropertiesView::Private
{
    Private(AllPropertiesView* p)
        : p             (p)
        , document      (nullptr)
        , synth         (nullptr)
        , ignoreSubComboSig(false)
        , ignoreProbsChange(false)
    { }

    void createWidgets();
    void updateViews() { updateComboBox(); updateViewFromProps(); }
    void updateComboBox();
    void updateViewFromProps();
    void updatePropsFromView();

    QString curId();
    QString curSubId();

    AllPropertiesView* p;

    ScoreDocument* document;
    SynthDevice* synth;
    Score::Index curScoreIndex;
    bool ignoreSubComboSig,
         ignoreProbsChange;
    QComboBox* combo, *subCombo;
    QProps::PropertiesView* propsView;

};

AllPropertiesView::AllPropertiesView(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    p_->createWidgets();
}

AllPropertiesView::~AllPropertiesView()
{
    delete p_;
}

void AllPropertiesView::Private::createWidgets()
{
    auto lv = new QVBoxLayout(p);
    lv->setMargin(1);

        combo = new QComboBox(p);
        lv->addWidget(combo);
        combo->addItem(tr("synth"), "synth");
        combo->addItem(tr("document"), "document");
        combo->addItem(tr("score"), "score");
        combo->addItem(tr("section"), "stream");
        combo->addItem(tr("page layout"), "page-layout");
        combo->addItem(tr("score layout"), "score-layout");
        connect(combo, static_cast<void(QComboBox::*)(int)>(
                                       &QComboBox::currentIndexChanged),
                [=](){ updateViews(); });

        subCombo = new QComboBox(p);
        lv->addWidget(subCombo);
        connect(subCombo, static_cast<void(QComboBox::*)(int)>(
                                       &QComboBox::currentIndexChanged),
                [=](){ if (!ignoreSubComboSig) updateViewFromProps(); });

        propsView = new QProps::PropertiesView(p);
        propsView->setSizePolicy(
                    QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        lv->addWidget(propsView);
        connect(propsView, &QProps::PropertiesView::propertyChanged, [=]()
        {
            updatePropsFromView();
        });
}

void AllPropertiesView::setDocument(ScoreDocument* s)
{
    bool changed = p_->document != s;
    if (!changed && s && p_->document)
        changed = s->editor() != p_->document->editor();

    p_->document = s;
    p_->curScoreIndex = s && s->score() ? s->score()->index(0,0,0,0)
                                        : Score::Index();

    if (p_->document && changed)
    {
        connect(p_->document->editor(), &ScoreEditor::documentPropertiesChanged,
                [=]() { if (!p_->ignoreProbsChange) p_->updateViews(); });
        connect(p_->document->editor(), &ScoreEditor::scorePropertiesChanged,
                [=]() { if (!p_->ignoreProbsChange) p_->updateViews(); });
        connect(p_->document->editor(), &ScoreEditor::streamPropertiesChanged,
                [=]() { if (!p_->ignoreProbsChange) p_->updateViews(); });
        connect(p_->document->editor(), &ScoreEditor::pageLayoutChanged,
                [=]() { if (!p_->ignoreProbsChange) p_->updateViews(); });
        connect(p_->document->editor(), &ScoreEditor::scoreLayoutChanged,
                [=]() { if (!p_->ignoreProbsChange) p_->updateViews(); });
    }

    p_->updateViews();
}

void AllPropertiesView::setSynthStream(SynthDevice* s)
{
    p_->synth = s;
    p_->updateViews();
}

void AllPropertiesView::setScoreIndex(const Score::Index& i)
{
    bool changed = i != p_->curScoreIndex;
    p_->curScoreIndex = i;
    if (changed && p_->curId() == "stream")
        p_->updateViews();
}

QString AllPropertiesView::Private::curId()
{
#if QT_VERSION >= 0x050200
    return combo->currentData().toString();
#else
    return combo->itemData(combo->currentIndex()).toString();
#endif
}

QString AllPropertiesView::Private::curSubId()
{
#if QT_VERSION >= 0x050200
    return subCombo->currentData().toString();
#else
    return subCombo->itemData(subCombo->currentIndex()).toString();
#endif
}

void AllPropertiesView::Private::updateComboBox()
{
    p->setUpdatesEnabled(false);
    ignoreSubComboSig = true;

    subCombo->clear();

    if (curId() == "synth")
    {
        subCombo->addItem(tr("master"), "master");
        for (size_t i=0; i<synth->synth().numberModVoices(); ++i)
            subCombo->addItem(tr("modulator voice %1").arg(i+1),
                              QString("mod-%1").arg(i));
    }
    else if (curId() == "page-layout")
    {
        subCombo->addItem(tr("title"), "title");
        subCombo->addItem(tr("left"), "left");
        subCombo->addItem(tr("right"), "right");
    }
    else if (curId() == "score-layout")
    {
        subCombo->addItem(tr("global"), "global");
    }

    subCombo->setVisible(subCombo->count() > 0);

    ignoreSubComboSig = false;
    p->setUpdatesEnabled(true);
}

void AllPropertiesView::Private::updateViewFromProps()
{
    if (curId() == "synth" && synth)
    {
        if (curSubId() == "master")
        {
            propsView->setProperties(synth->synth().props());
            return;
        }
        else if (curSubId().startsWith("mod-"))
        {
            int idx = curSubId().mid(4).toInt();
            if (idx < (int)synth->synth().numberModVoices())
            {
                propsView->setProperties(synth->synth().modProps(idx));
                return;
            }
        }

        propsView->clear();
        return;
    }

    if (!document)
    {
        propsView->clear();
        return;
    }

    if (curId() == "document")
    {
        propsView->setProperties(document->props());
        return;
    }
    if (curId() == "score")
    {
        propsView->setProperties(document->score()->props());
        return;
    }
    if (curId() == "stream" && curScoreIndex.isValid())
    {
        propsView->setProperties(curScoreIndex.getStream().props());
        return;
    }
    if (curId() == "page-layout")
    {
        propsView->setProperties(
                    document->pageLayout(curSubId()).margins());
        return;
    }
    if (curId() == "score-layout")
    {
        propsView->setProperties(
                    document->scoreLayout(curSubId()).props());
        return;
    }

    propsView->clear();
}


void AllPropertiesView::Private::updatePropsFromView()
{
    if (curId() == "synth" && synth)
    {
        if (curSubId() == "master")
        {
            synth->setSynthProperties(propsView->properties());
            updateComboBox();
        }
        else if (curSubId().startsWith("mod-"))
        {
            int idx = curSubId().mid(4).toInt();
            if (idx < (int)synth->synth().numberModVoices())
                synth->setSynthModProperties(idx, propsView->properties());
        }
        emit p->synthChanged();
        return;
    }
    if (!document)
        return;

    ignoreProbsChange = true;

    if (curId() == "document")
    {
        document->editor()->setDocumentProperties(propsView->properties());
    }
    else if (curId() == "page-layout")
    {
        auto l = document->pageLayout(curSubId());
        l.setMargins(propsView->properties());
        document->editor()->setPageLayout(curSubId(), l);
    }
    else if (curId() == "score-layout")
    {
        auto l = document->scoreLayout(curSubId());
        l.setProperties(propsView->properties());
        document->editor()->setScoreLayout(curSubId(), l);
    }
    else if (curId() == "score")
    {
        document->editor()->setScoreProperties(propsView->properties());
    }
    else if (curId() == "stream")
    {
        if (curScoreIndex.isValid())
        {
            document->editor()->setStreamProperties(
                        curScoreIndex.stream(), propsView->properties());
        }
    }

    ignoreProbsChange = false;

    /*
    PageAnnotation anno = scoreView->scoreDocument().pageAnnotation(0);
    anno.textItems()[0].setProperties(propsView->properties());
    scoreView->scoreDocument().setPageAnnotation(0, anno);
    scoreView->update();
    */

}

} // namespace Sonot
