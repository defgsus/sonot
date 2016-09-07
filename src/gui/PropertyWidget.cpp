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

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QToolButton>
#include <QFontComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVector>
#include <QStandardItemModel>

#include "PropertyWidget.h"
#include "io/Properties.h"
#include "io/error.h"

namespace Sonot {


struct PropertyWidget::Private
{
    Private(const QString& n, const QVariant& v, PropertyWidget * w)
        : widget(w), props(0), v(v), name(n),
          ignore_widget(false),
          layout(0), label(0), edit(0) { }

    /** Creates or re-creates the appropriate widgets.
        Can safely be called before or after changing the variant type */
    void createWidgets();
    /** Updates the widget to the current value */
    void updateWidget();

    PropertyWidget * widget;
    const Properties * props;
    QString id, tip;
    QVariant v;
    QString name;

    std::function<void()>
        f_update_widget,
        f_update_value;
    bool ignore_widget;

    QHBoxLayout * layout;
    QLabel * label;
    QWidget * edit;
};

PropertyWidget::PropertyWidget(
        const QString& id, const Properties * prop, QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(QString(), prop->get(id), this))
{
    p_->id = id;
    p_->props = prop;
    p_->name = prop->hasName(id) ? prop->getName(id) : id;
    p_->tip = prop->getTip(id);
    p_->createWidgets();
}


const QVariant& PropertyWidget::value() const
{
    return p_->v;
}

void PropertyWidget::Private::updateWidget()
{
    if (f_update_widget)
    {
        // don't emit user signal
        ignore_widget = true;
        // update the widget value
        f_update_widget();
        ignore_widget = false;
    }
}

void PropertyWidget::onValueChanged_()
{
    qDebug() << "VALUE CHANGED " << p_->id;
    if (p_->ignore_widget)
        return;

    if (p_->f_update_value)
    {
        // copy widget value into own Properties
        p_->f_update_value();
        // emit the user signal
        emit valueChanged();
    }
}

void PropertyWidget::Private::createWidgets()
{
    if (!layout)
    {
        layout = new QHBoxLayout(widget);
        layout->setMargin(2);
        //l->setSizeConstraint(QLayout::SetMaximumSize);
    }

    if (!label)
    {
        label = new QLabel(name, widget);
        layout->addWidget(label);
    }
    else
        label->setText(name);

    //layout->addStretch(1);

    // clear previous edit widget
    if (edit)
    {
        edit->setVisible(false);
        edit->deleteLater();
        edit = 0;
        f_update_value = 0;
        f_update_widget = 0;
    }

    widget->setStatusTip(tip);
    label->setStatusTip(tip);

    // ----- create appropriate sub-widgets -----

#define MO__SUBLAYOUT(Layout__) \
    edit = new QWidget(widget); \
    auto layout = new Layout__(edit); \
    layout->setMargin(0);

    bool isHandled = false;

    // --- Properties::NamedValues ---

    if (props && props->getProperty(id).hasNamedValues())
    {
        auto nv = props->getProperty(id).namedValues();
        auto nvi = props->getProperty(id).namedValuesByIndex();

        // single value from list
        if (!nv.isFlags())
        {
            auto cb = new QComboBox(widget);
            edit = cb;

            for (const Properties::NamedValues::Value & i : nvi)
            {
                cb->addItem(i.name, i.id);
            }

            f_update_widget = [=]()
            {
                auto val = nv.getByValue(v);
                cb->setCurrentText(val.name);
            };
            f_update_value = [=]()
            {
                const QString id = cb->itemData(cb->currentIndex()).toString();
                auto val = nv.get(id);
                v = val.v;
            };
            connect(cb, SIGNAL(currentIndexChanged(int)),
                    widget, SLOT(onValueChanged_()));
        }
        else
        // flags
        {
#ifndef Q_OS_LINUX
            /** @todo Checkboxes in QComboBox do not appear
                on my XUbuntu 14.04 / Qt 5.5 */
            auto cb = new QComboBox(widget);
            edit = cb;

            auto model = new QStandardItemModel(nvi.size(), 1, widget);
            for (const Properties::NamedValues::Value & i : nvi)
            {
                auto item = new QStandardItem(i.name);
                item->setData(i.id);
                item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                item->setData(Qt::Unchecked, Qt::CheckStateRole);
                model->setItem(i.index, item);
            }
            cb->setModel(model);
            f_update_widget = [=]()
            {
                auto nvi = props->getProperty(id).namedValuesByIndex();
                qlonglong flags = v.toLongLong();
                for (const Properties::NamedValues::Value & i : nvi)
                {
                    qlonglong flag = i.v.toLongLong();
                    bool checked = (flags & flag) == flag;
                    model->item(i.index, 0)->setCheckState(
                                checked ? Qt::Checked : Qt::Unchecked);
                }
            };
            f_update_value = [=]()
            {
                /** @todo */
            };
#else
            MO__SUBLAYOUT(QVBoxLayout);
            QList<QCheckBox*> boxes;
            for (const Properties::NamedValues::Value & i : nvi)
            {
                auto cb = new QCheckBox(widget);
                cb->setText(i.name);
                cb->setStatusTip(i.tip);
                connect(cb, SIGNAL(clicked(bool)),
                        widget, SLOT(onValueChanged_()));
                layout->addWidget(cb);
                boxes << cb;
            }
            f_update_widget = [=]()
            {
                qlonglong flags = v.toLongLong();
                auto nvi = props->getProperty(id).namedValuesByIndex();
                for (const Properties::NamedValues::Value & i : nvi)
                {
                    qlonglong flag = i.v.toLongLong();
                    bool checked = (flags & flag) == flag;
                    if (i.index < (size_t)boxes.size())
                        boxes[i.index]->setChecked(checked);
                }
            };
            f_update_value = [=]()
            {
                qlonglong flags = 0;
                auto nvi = props->getProperty(id).namedValuesByIndex();
                for (const Properties::NamedValues::Value & i : nvi)
                {
                    if (i.index < (size_t)boxes.size()
                     && boxes[i.index]->isChecked())
                        flags |= i.v.toLongLong();
                }
                v = flags;
            };
#endif
        }

        isHandled = true;
    }

    // ---- select widget by type (standard types) ----
    if (!isHandled)
    {
        isHandled = true;

        switch ((int)v.type())
        {
            case QMetaType::Bool:
            {
                auto cb = new QCheckBox(widget);
                edit = cb;
                f_update_widget = [=](){ cb->setChecked(v.toBool()); };
                f_update_value = [=](){ v = cb->isChecked(); };
                connect(cb, SIGNAL(stateChanged(int)), widget,
                        SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::Int:
            case QMetaType::UInt:
            {
                bool hasSign = v.type() == QVariant::UInt;
                auto sb = new QSpinBox(widget);
                edit = sb;
                sb->setRange(hasSign ? -999999999 : 0, 999999999);
                if (props)
                {
                    if (props->hasMin(id))
                        sb->setMinimum(props->getMin(id).toInt());
                    if (props->hasMax(id))
                        sb->setMaximum(props->getMax(id).toInt());
                    if (props->hasStep(id))
                        sb->setSingleStep(props->getStep(id).toInt());
                }
                f_update_widget = [=](){ sb->setValue(v.toInt()); };
                if (hasSign)
                    f_update_value = [=](){ v = sb->value(); };
                else
                    f_update_value = [=](){ v = (unsigned)sb->value(); };
                connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::Double:
            case QMetaType::Float:
            {
                auto sb = new QDoubleSpinBox(widget);
                edit = sb;
                //sb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                sb->setRange(-9999999, 9999999);
                sb->setDecimals(7);
                if (props)
                {
                    if (props->hasMin(id))
                        sb->setMinimum(props->getMin(id).toDouble());
                    if (props->hasMax(id))
                        sb->setMaximum(props->getMax(id).toDouble());
                    if (props->hasStep(id))
                        sb->setSingleStep(props->getStep(id).toDouble());
                }
                f_update_widget = [=](){ sb->setValue(v.toDouble()); };
                if (v.type() == QVariant::Double)
                    f_update_value = [=](){ v = double(sb->value()); };
                else
                    f_update_value = [=](){ v = float(sb->value()); };
                connect(sb, SIGNAL(valueChanged(double)), widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::QString:
            {
                int subtype = props && props->hasSubType(id)
                        ? props->getSubType(id)
                        : -1;

                if (subtype > 0)
                {
                    // string display with edit button (->TextEditDialog)
                    if ((subtype & Properties::ST_TEXT) == Properties::ST_TEXT)
                    {
                        MO__SUBLAYOUT(QHBoxLayout);
                        auto e = new QLineEdit(widget);
                        e->setReadOnly(true);
                        e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                        layout->addWidget(e);

                        auto b = new QToolButton(widget);
                        b->setText(tr("..."));
                        layout->addWidget(b);
                        connect(b, &QToolButton::clicked, [=]()
                        {
#ifdef __TODO___
                            auto diag = new TextEditDialog(
                                        TextType(subtype & Properties::subTypeMask), widget);
                            diag->setAttribute(Qt::WA_DeleteOnClose);
                            diag->setWindowTitle(name);
                            diag->setText(v.toString());
                            if (props)
                                props->callWidgetCallback(id, diag);
                            connect(diag, &TextEditDialog::textChanged, [=]()
                            {
                                e->setText(diag->getText());
                                widget->onValueChanged_();
                            });
                            diag->show();
#endif
                        });
                        /** @todo missing update of dialog
                            (would require a pointer to running dialogs) */
                        f_update_widget = [=](){ e->setText(v.toString()); };
                        f_update_value = [=](){ v = e->text(); };
                    }
                    // filename with button (IO::File::getOpenFilename())
                    else
                    if ((subtype & Properties::ST_FILENAME) == Properties::ST_FILENAME)
                    {
                        MO__SUBLAYOUT(QHBoxLayout);
                        auto e = new QLineEdit(widget);
                        e->setReadOnly(true);
                        e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                        layout->addWidget(e);

                        auto b = new QToolButton(widget);
                        b->setText(tr("..."));
                        layout->addWidget(b);
                        connect(b, &QToolButton::clicked, [=]()
                        {
#ifdef __TODO__
                            QString fn = IO::Files::getOpenFileName(
                                        IO::FileType(subtype & Properties::subTypeMask));
                            if (fn.isEmpty())
                                return;
                            e->setText(fn);
                            widget->onValueChanged_();
#endif
                        });
                        f_update_widget = [=](){ e->setText(v.toString()); };
                        f_update_value = [=](){ v = e->text(); };
                    }
                }
                // generic string edit
                else
                {
                    auto e = new QLineEdit(widget);
                    edit = e;
                    e->setReadOnly(false);
                    e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                    f_update_widget = [=](){ e->setText(v.toString()); };
                    f_update_value = [=](){ v = e->text(); };
                    connect(e, SIGNAL(textChanged(QString)), widget, SLOT(onValueChanged_()));
                }
            }
            break;

            case QMetaType::QSize:
            {
                MO__SUBLAYOUT(QHBoxLayout);
                auto sb1 = new QSpinBox(widget),
                     sb2 = new QSpinBox(widget);
                sb1->setRange(0, 9999999);
                sb2->setRange(0, 9999999);
                if (props)
                {
                    if (props->hasMin(id))
                    {
                        sb1->setMinimum(props->getMin(id).toSize().width());
                        sb2->setMinimum(props->getMin(id).toSize().height());
                    }
                    if (props->hasMax(id))
                    {
                        sb1->setMaximum(props->getMax(id).toSize().width());
                        sb2->setMaximum(props->getMax(id).toSize().height());
                    }
                    if (props->hasStep(id))
                    {
                        sb1->setSingleStep(props->getStep(id).toSize().width());
                        sb2->setSingleStep(props->getStep(id).toSize().height());
                    }
                }
                layout->addWidget(sb1);
                layout->addWidget(sb2);
                f_update_widget = [=](){ auto s = v.toSize(); sb1->setValue(s.width()); sb2->setValue(s.height()); };
                f_update_value = [=](){ v = QSize(sb1->value(), sb2->value()); };
                connect(sb1, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
                connect(sb2, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::QSizeF:
            {
                MO__SUBLAYOUT(QHBoxLayout);
                auto sb1 = new QDoubleSpinBox(widget),
                     sb2 = new QDoubleSpinBox(widget);
                sb1->setRange(0, 9999999); sb1->setDecimals(4);
                sb2->setRange(0, 9999999); sb2->setDecimals(4);
                if (props)
                {
                    if (props->hasMin(id))
                    {
                        sb1->setMinimum(props->getMin(id).toSizeF().width());
                        sb2->setMinimum(props->getMin(id).toSizeF().height());
                    }
                    if (props->hasMax(id))
                    {
                        sb1->setMaximum(props->getMax(id).toSizeF().width());
                        sb2->setMaximum(props->getMax(id).toSizeF().height());
                    }
                    if (props->hasStep(id))
                    {
                        sb1->setSingleStep(props->getStep(id).toSizeF().width());
                        sb2->setSingleStep(props->getStep(id).toSizeF().height());
                    }
                }
                layout->addWidget(sb1);
                layout->addWidget(sb2);
                f_update_widget = [=](){ auto s = v.toSize(); sb1->setValue(s.width()); sb2->setValue(s.height()); };
                f_update_value = [=](){ v = QSizeF(sb1->value(), sb2->value()); };
                connect(sb1, SIGNAL(valueChanged(double)), widget, SLOT(onValueChanged_()));
                connect(sb2, SIGNAL(valueChanged(double)), widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::QPointF:
            {
                MO__SUBLAYOUT(QHBoxLayout);
                auto sb1 = new QDoubleSpinBox(widget),
                     sb2 = new QDoubleSpinBox(widget);
                sb1->setRange(0, 9999999); sb1->setDecimals(4);
                sb2->setRange(0, 9999999); sb2->setDecimals(4);
                if (props)
                {
                    if (props->hasMin(id))
                    {
                        sb1->setMinimum(props->getMin(id).toPointF().x());
                        sb2->setMinimum(props->getMin(id).toPointF().y());
                    }
                    if (props->hasMax(id))
                    {
                        sb1->setMaximum(props->getMax(id).toPointF().x());
                        sb2->setMaximum(props->getMax(id).toPointF().y());
                    }
                    if (props->hasStep(id))
                    {
                        sb1->setSingleStep(props->getStep(id).toPointF().x());
                        sb2->setSingleStep(props->getStep(id).toPointF().y());
                    }
                }
                layout->addWidget(sb1);
                layout->addWidget(sb2);
                f_update_widget = [=]()
                {
                    auto s = v.toSizeF();
                    sb1->setValue(s.width()); sb2->setValue(s.height());
                };
                f_update_value = [=]()
                    { v = QPointF(sb1->value(), sb2->value()); };
                connect(sb1, SIGNAL(valueChanged(double)),
                        widget, SLOT(onValueChanged_()));
                connect(sb2, SIGNAL(valueChanged(double)),
                        widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::QRectF:
            {
                MO__SUBLAYOUT(QGridLayout);
                auto sb1 = new QDoubleSpinBox(widget),
                     sb2 = new QDoubleSpinBox(widget),
                     sb3 = new QDoubleSpinBox(widget),
                     sb4 = new QDoubleSpinBox(widget);
                sb1->setStatusTip(tr("X position"));
                sb2->setStatusTip(tr("Y position"));
                sb3->setStatusTip(tr("Width"));
                sb4->setStatusTip(tr("Height"));
                sb1->setRange(0, 9999999); sb1->setDecimals(4);
                sb2->setRange(0, 9999999); sb2->setDecimals(4);
                sb3->setRange(0, 9999999); sb3->setDecimals(4);
                sb4->setRange(0, 9999999); sb4->setDecimals(4);
                if (props)
                {
                    if (props->hasMin(id))
                    {
                        sb1->setMinimum(props->getMin(id).toRectF().x());
                        sb2->setMinimum(props->getMin(id).toRectF().y());
                        sb3->setMinimum(props->getMin(id).toRectF().width());
                        sb4->setMinimum(props->getMin(id).toRectF().height());
                    }
                    if (props->hasMax(id))
                    {
                        sb1->setMaximum(props->getMax(id).toRectF().x());
                        sb2->setMaximum(props->getMax(id).toRectF().y());
                        sb3->setMaximum(props->getMax(id).toRectF().width());
                        sb4->setMaximum(props->getMax(id).toRectF().height());
                    }
                    if (props->hasStep(id))
                    {
                        sb1->setSingleStep(props->getStep(id).toRectF().x());
                        sb2->setSingleStep(props->getStep(id).toRectF().y());
                        sb3->setSingleStep(props->getStep(id).toRectF().width());
                        sb4->setSingleStep(props->getStep(id).toRectF().height());
                    }
                }
                layout->addWidget(sb1, 0, 0);
                layout->addWidget(sb2, 1, 0);
                layout->addWidget(sb3, 0, 1);
                layout->addWidget(sb4, 1, 1);
                f_update_widget = [=]()
                {
                    auto r = v.toRectF();
                    sb1->setValue(r.x());
                    sb2->setValue(r.y());
                    sb3->setValue(r.width());
                    sb4->setValue(r.height());
                };
                f_update_value = [=]()
                {
                    v = QRectF(sb1->value(), sb2->value(),
                               sb3->value(), sb4->value());
                };
                connect(sb1, SIGNAL(valueChanged(double)),
                        widget, SLOT(onValueChanged_()));
                connect(sb2, SIGNAL(valueChanged(double)),
                        widget, SLOT(onValueChanged_()));
                connect(sb3, SIGNAL(valueChanged(double)),
                        widget, SLOT(onValueChanged_()));
                connect(sb4, SIGNAL(valueChanged(double)),
                        widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::QPoint:
            {
                MO__SUBLAYOUT(QHBoxLayout);
                auto sb1 = new QSpinBox(widget),
                     sb2 = new QSpinBox(widget);
                sb1->setRange(-9999999, 9999999);
                sb2->setRange(-9999999, 9999999);
                if (props)
                {
                    if (props->hasMin(id))
                    {
                        sb1->setMinimum(props->getMin(id).toPoint().x());
                        sb2->setMinimum(props->getMin(id).toPoint().y());
                    }
                    if (props->hasMax(id))
                    {
                        sb1->setMaximum(props->getMax(id).toPoint().x());
                        sb2->setMaximum(props->getMax(id).toPoint().y());
                    }
                    if (props->hasStep(id))
                    {
                        sb1->setSingleStep(props->getStep(id).toPoint().x());
                        sb2->setSingleStep(props->getStep(id).toPoint().y());
                    }
                }
                layout->addWidget(sb1);
                layout->addWidget(sb2);
                f_update_widget = [=](){ auto s = v.toPoint(); sb1->setValue(s.x()); sb2->setValue(s.y()); };
                f_update_value = [=](){ v = QPoint(sb1->value(), sb2->value()); };
                connect(sb1, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
                connect(sb2, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
            }
            break;
            case QMetaType::QColor:
            {
#ifdef __TODO___
                auto e = new ColorEditWidget(widget);
                edit = e;
                f_update_widget = [=](){ e->setCurrentColor(v.value<QColor>()); };
                f_update_value = [=](){ v = e->currentColor(); };
                connect(e, SIGNAL(textChanged(QString)),
                        widget, SLOT(onValueChanged_()));
#else
                MO__SUBLAYOUT(QHBoxLayout);
                auto sb1 = new QSpinBox(widget),
                     sb2 = new QSpinBox(widget),
                     sb3 = new QSpinBox(widget),
                     sb4 = new QSpinBox(widget);
                sb1->setRange(0, 255);
                sb2->setRange(0, 255);
                sb3->setRange(0, 255);
                sb4->setRange(0, 255);
                layout->addWidget(sb1);
                layout->addWidget(sb2);
                layout->addWidget(sb3);
                layout->addWidget(sb4);
                f_update_widget = [=]()
                {
                    auto c = v.value<QColor>();
                    sb1->setValue(c.red());
                    sb2->setValue(c.green());
                    sb3->setValue(c.blue());
                    sb4->setValue(c.alpha());
                };
                f_update_value = [=]()
                {
                    v = QColor(sb1->value(), sb2->value(),
                               sb3->value(), sb4->value());
                };
                connect(sb1, SIGNAL(valueChanged(int)),
                        widget, SLOT(onValueChanged_()));
                connect(sb2, SIGNAL(valueChanged(int)),
                        widget, SLOT(onValueChanged_()));
                connect(sb3, SIGNAL(valueChanged(int)),
                        widget, SLOT(onValueChanged_()));
                connect(sb4, SIGNAL(valueChanged(int)),
                        widget, SLOT(onValueChanged_()));
#endif
            }
            break;
            case QMetaType::QFont:
            {
                auto e = new QFontComboBox(widget);
                edit = e;
                f_update_widget = [=](){ e->setCurrentFont(v.value<QFont>()); };
                f_update_value = [=](){ v = e->currentFont(); };
                connect(e, SIGNAL(currentFontChanged(QFont)),
                        widget, SLOT(onValueChanged_()));
            }
            break;

            default:
                isHandled = false;
            break;
        }

    } // switch(type)



    // ---- create QVector types ----

#define MO__VECTOR(T__, SpinBox__, sigType__, negRange__) \
    if (!isHandled && !strcmp(v.typeName(), #T__)) \
    { \
        MO__SUBLAYOUT(QHBoxLayout); \
        auto vec = v.value<T__>(); \
        QVector<SpinBox__*> sb; \
        for (int i=0; i<vec.size(); ++i) \
        { \
            sb << new SpinBox__(widget); \
            sb.back()->setRange(negRange__, 9999999); \
        } \
        if (props) \
        { \
            if (props->hasMin(id)) \
            { \
                auto val = props->getMin(id).value<T__>(); \
                for (int i=0; i<std::min(vec.size(), val.size()); ++i) \
                    sb[i]->setMinimum(val[i]); \
            } \
            if (props->hasMax(id)) \
            { \
                auto val = props->getMax(id).value<T__>(); \
                for (int i=0; i<std::min(vec.size(), val.size()); ++i) \
                    sb[i]->setMaximum(val[i]); \
            } \
            if (props->hasStep(id)) \
            { \
                auto val = props->getStep(id).value<T__>(); \
                for (int i=0; i<std::min(vec.size(), val.size()); ++i) \
                    sb[i]->setSingleStep(val[i]); \
            } \
        } \
        f_update_widget = [=]() \
        { \
            auto val = v.value<T__>(); \
            for (int i=0; i<vec.size(); ++i) \
                sb[i]->setValue(val[i]); \
        }; \
        f_update_value = [=]() \
        { \
            auto val = T__(); \
            for (int i=0; i<vec.size(); ++i) \
                val << sb[i]->value(); \
            v = QVariant::fromValue(val); \
        }; \
        for (int i=0; i<vec.size(); ++i) \
        { \
            layout->addWidget(sb[i]); \
            connect(sb[i], SIGNAL(valueChanged(sigType__)), \
                    widget, SLOT(onValueChanged_())); \
        } \
        isHandled = true; \
    }

    MO__VECTOR(QVector<float>, QDoubleSpinBox, double, -9999999)
    MO__VECTOR(QVector<double>, QDoubleSpinBox, double, -9999999)
    MO__VECTOR(QVector<int>, QSpinBox, int, -9999999)
    MO__VECTOR(QVector<uint>, QSpinBox, int, 0)

#undef MO__VECTOR



    // ---- error-checking / finalize ----

    if (edit)
    {
        SONOT_ASSERT(f_update_widget, "No widget update defined for type '" << v.typeName() << "'");
        SONOT_ASSERT(f_update_value, "No value update defined for type '" << v.typeName() << "'");
        if (props)
            props->callWidgetCallback(id, edit);
        updateWidget();
        layout->addWidget(edit);
    }

    if (!isHandled)
    {
        SONOT_PROG_ERROR("PropertyWidget: unhandled type '"
                         << v.typeName() << "'");
    }

#undef MO__SUBLAYOUT
}


} // namespace Sonot
