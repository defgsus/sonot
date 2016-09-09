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
#include "PropertiesView.h"
#include "ScoreDocument.h"
#include "PageLayout.h"
#include "ScoreLayout.h"

namespace Sonot {


struct AllPropertiesView::Private
{
    Private(AllPropertiesView* p)
        : p         (p)
        , scoreDoc     (nullptr)
    { }

    void createWidgets();
    void updateComboBox();

    AllPropertiesView* p;

    ScoreDocument* scoreDoc;

    QComboBox* combo;
    PropertiesView* props;


};

AllPropertiesView::AllPropertiesView(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{

}

AllPropertiesView::~AllPropertiesView()
{
    delete p_;
}

void AllPropertiesView::Private::createWidgets()
{
    auto lv = new QVBoxLayout(p);

        combo = new QComboBox(p);
        lv->addWidget(combo);

        props = new PropertiesView(p);
        lv->addWidget(props);
}

void AllPropertiesView::setDocument(ScoreDocument* s)
{
    p_->scoreDoc = s;
    p_->updateComboBox();
}

void AllPropertiesView::Private::updateComboBox()
{
    p->setUpdatesEnabled(false);

    combo->clear();
    if (scoreDoc)
    {
        combo->addItem(tr("score layout"), "score-layout");

        //scoreDoc->scoreLayout()
    }
    p->setUpdatesEnabled(true);
}


} // namespace Sonot
