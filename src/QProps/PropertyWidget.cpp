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

#ifdef QT_WIDGETS_LIB

#include <functional>
#include <limits>

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QToolButton>
#include <QFontComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QTimeEdit>
#include <QVector>
#include <QStandardItemModel>

#include "PropertyWidget.h"
#include "Properties.h"
#include "error.h"

namespace QProps {


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

    template <class Convert>
    QList<typename Convert::SpinBox*>
        createSpinboxes(QWidget* parent, int num, const char* signalName);

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
        const QString& id, const Properties * prop,
        QWidget *parent) throw(Exception)
    : QWidget       (parent)
    , p_            (new Private(QString(), prop->get(id), this))
{
    p_->id = id;
    p_->props = prop;
    p_->name = prop->hasName(id) ? prop->getName(id) : id;
    p_->tip = prop->getTip(id);
    p_->createWidgets();
}

const QVariant& PropertyWidget::value() const { return p_->v; }

QWidget* PropertyWidget::editWidget() const { return p_->edit; }

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
    if (p_->ignore_widget)
        return;
    qDebug() << "VALUE CHANGED " << p_->id;

    if (p_->f_update_value)
    {
        // copy widget value into own Properties
        p_->f_update_value();
        // emit the user signal
        emit valueChanged();
    }
}


namespace {

    template <class T, class SB>
    struct ConvertQSize
    {
        typedef SB SpinBox;
        static int toValue(const QVariant& v, int idx)
        {
            T s = v.value<T>();
            return idx == 0 ? s.width() : s.height();
        }

        static QVariant fromSpinbox(const QList<SpinBox*>& sb)
        {
            return T(sb[0]->value(), sb[1]->value());
        }
    };

    template <class T, class SB>
    struct ConvertQPoint
    {
        typedef SB SpinBox;
        static int toValue(const QVariant& v, int idx)
        {
            T s = v.value<T>();
            return idx == 0 ? s.x() : s.y();
        }

        static QVariant fromSpinbox(const QList<SpinBox*>& sb)
        {
            return T(sb[0]->value(), sb[1]->value());
        }
    };

    template <class T, class SB>
    struct ConvertQRect
    {
        typedef SB SpinBox;
        static double toValue(const QVariant& v, int idx)
        {
            T r = v.value<T>();
            return idx == 0 ? r.x() : idx == 1
                              ? r.y() : idx == 2 ? r.width() : r.height();
        }

        static QVariant fromSpinbox(const QList<SpinBox*>& sb)
        {
            return T(sb[0]->value(), sb[1]->value(),
                     sb[2]->value(), sb[3]->value());
        }
    };

    template <class T, class SB>
    struct ConvertQLine
    {
        typedef SB SpinBox;
        static double toValue(const QVariant& v, int idx)
        {
            T r = v.value<T>();
            return idx == 0 ? r.x1() : idx == 1
                              ? r.y1() : idx == 2 ? r.x2() : r.y2();
        }

        static QVariant fromSpinbox(const QList<SpinBox*>& sb)
        {
            return T(sb[0]->value(), sb[1]->value(),
                     sb[2]->value(), sb[3]->value());
        }
    };

    struct ConvertQColor
    {
        typedef QSpinBox SpinBox;
        static double toValue(const QVariant& v, int idx)
        {
            QColor c = v.value<QColor>();
            return idx == 0 ? c.red() : idx == 1
                              ? c.green() : idx == 2 ? c.blue() : c.alpha();
        }

        static QVariant fromSpinbox(const QList<SpinBox*>& sb)
        {
            return QColor(sb[0]->value(), sb[1]->value(),
                          sb[2]->value(), sb[3]->value());
        }
    };

} // namespace



template <class Convert>
QList<typename Convert::SpinBox*> PropertyWidget::Private::createSpinboxes(
        QWidget* parent,
        int num, const char* signalName)
{
    typedef typename Convert::SpinBox SpinBox;
    QList<SpinBox*> sb;
    for (int i=0; i<num; ++i)
    {
        sb << new SpinBox(parent);
        sb.back()->setRange(-9999999, 9999999);
        connect(sb.back(), signalName,
                widget, SLOT(onValueChanged_()));
    }
    if (props)
    {
        if (props->hasMin(id))
        {
            for (int i=0; i<num; ++i)
                sb[i]->setMinimum(Convert::toValue(props->getMin(id), i));
        }
        if (props->hasMax(id))
        {
            for (int i=0; i<num; ++i)
                sb[i]->setMaximum(Convert::toValue(props->getMax(id), i));
        }
        if (props->hasStep(id))
        {
            for (int i=0; i<num; ++i)
                sb[i]->setSingleStep(Convert::toValue(props->getStep(id), i));
        }
    }
    f_update_widget = [=]()
    {
        for (int i=0; i<num; ++i)
            sb[i]->setValue(Convert::toValue(v, i));
    };
    f_update_value = [=]()
    {
        v = Convert::fromSpinbox(sb);
    };
    return sb;
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

    QWidget* container = nullptr;

#define QPROPS__SUBLAYOUT(Layout__) \
    edit = container = new QWidget(widget); \
    auto layout = new Layout__(edit); \
    layout->setMargin(0);

#define QPROPS__FRAMED_SUBLAYOUT(Layout__) \
    edit = container = new QFrame(widget); \
    static_cast<QFrame*>(edit)->setFrameStyle(QFrame::Panel | QFrame::Raised); \
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
            /** @todo Checkboxes in QComboBox do not appear
                on my XUbuntu 14.04 / Qt 5.5 */
#ifndef Q_OS_LINUX
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
            QPROPS__FRAMED_SUBLAYOUT(QVBoxLayout);
            QList<QCheckBox*> boxes;
            for (const Properties::NamedValues::Value & i : nvi)
            {
                auto cb = new QCheckBox(container);
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

        QMetaType::Type metaType = QMetaType::Type(v.type());
        switch (metaType)
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

#define QPROPS__SB_IMPL(SB__, sb__, Type__, Meta__, Conv__, Signal__) \
                sb__ = new SB__(widget); \
                sb__->setRange(std::numeric_limits<Type__>::min(), \
                               std::numeric_limits<Type__>::max()); \
                if (props) \
                { \
                    if (props->hasMin(id)) \
                        sb__->setMinimum(props->getMin(id).Conv__()); \
                    if (props->hasMax(id)) \
                        sb__->setMaximum(props->getMax(id).Conv__()); \
                    if (props->hasStep(id)) \
                        sb__->setSingleStep(props->getStep(id).Conv__()); \
                } \
                f_update_widget = [=](){ sb__->setValue(v.Conv__()); }; \
                f_update_value = [=](){ v = (Type__)sb__->value(); }; \
                connect(sb__, Signal__, widget, SLOT(onValueChanged_()));

#define QPROPS__INT_SB(sb__, Type__, Meta__, Conv__) \
    QPROPS__SB_IMPL(QSpinBox, sb__, Type__, Meta__, Conv__, \
                    SIGNAL(valueChanged(int)))

#define QPROPS__FLOAT_SB(sb__, Type__, Meta__, Conv__) \
    QPROPS__SB_IMPL(QDoubleSpinBox, sb__, Type__, Meta__, Conv__, \
                    SIGNAL(valueChanged(double)))

            case QMetaType::Short:
            {
                QSpinBox* sb;
                QPROPS__INT_SB(sb, int16_t, Short, toInt);
                edit = sb;
            }
            break;

            case QMetaType::UShort:
            {
                QSpinBox* sb;
                QPROPS__INT_SB(sb, uint16_t, UShort, toUInt);
                edit = sb;
            }
            break;

            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::LongLong:
            {
                QSpinBox* sb;
                QPROPS__INT_SB(sb, int32_t, Int, toLongLong);
                edit = sb;
            }
            break;

            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::ULongLong:
            {
                QSpinBox* sb;
                QPROPS__INT_SB(sb, uint32_t, UInt, toULongLong);
                edit = sb;
            }
            break;

            case QMetaType::Float:
            {
                QDoubleSpinBox* sb;
                QPROPS__FLOAT_SB(sb, float, Float, toDouble);
                edit = sb;
            }
            break;

            case QMetaType::Double:
            {
                QDoubleSpinBox* sb;
                QPROPS__FLOAT_SB(sb, double, Float, toDouble);
                edit = sb;
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
                        QPROPS__SUBLAYOUT(QHBoxLayout);
                        auto e = new QLineEdit(container);
                        e->setReadOnly(true);
                        e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                        layout->addWidget(e);

                        auto b = new QToolButton(container);
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
                        QPROPS__SUBLAYOUT(QHBoxLayout);
                        auto e = new QLineEdit(container);
                        e->setReadOnly(true);
                        e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                        layout->addWidget(e);

                        auto b = new QToolButton(container);
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
                QPROPS__SUBLAYOUT(QHBoxLayout);
                auto sb = createSpinboxes<ConvertQSize<QSize, QSpinBox>>(
                            container, 2, SIGNAL(valueChanged(int)));
                for (auto s : sb)
                    layout->addWidget(s);
            }
            break;

            case QMetaType::QSizeF:
            {
                QPROPS__SUBLAYOUT(QHBoxLayout);
                auto sb = createSpinboxes<ConvertQSize<QSizeF,QDoubleSpinBox>>(
                            container, 2, SIGNAL(valueChanged(double)));
                for (auto s : sb)
                    layout->addWidget(s);
            }
            break;

            case QMetaType::QPointF:
            {
                QPROPS__SUBLAYOUT(QHBoxLayout);
                auto sb = createSpinboxes<ConvertQPoint<QPointF,QDoubleSpinBox>>(
                            container, 2, SIGNAL(valueChanged(double)));
                for (auto s : sb)
                    layout->addWidget(s);
            }
            break;

            case QMetaType::QPoint:
            {
                QPROPS__SUBLAYOUT(QHBoxLayout);
                auto sb = createSpinboxes<ConvertQPoint<QPoint,QSpinBox>>(
                            container, 2, SIGNAL(valueChanged(int)));
                for (auto s : sb)
                    layout->addWidget(s);
            }
            break;

            case QMetaType::QRectF:
            {
                QPROPS__SUBLAYOUT(QGridLayout);
                auto sb = createSpinboxes<ConvertQRect<QRectF,QDoubleSpinBox>>(
                            container, 4, SIGNAL(valueChanged(double)));
                sb[0]->setStatusTip(tr("X Position"));
                sb[1]->setStatusTip(tr("Y Position"));
                sb[2]->setStatusTip(tr("Width"));
                sb[3]->setStatusTip(tr("Height"));
                layout->addWidget(sb[0], 0, 0);
                layout->addWidget(sb[1], 1, 0);
                layout->addWidget(sb[2], 0, 1);
                layout->addWidget(sb[3], 1, 1);
            }
            break;

            case QMetaType::QRect:
            {
                QPROPS__SUBLAYOUT(QGridLayout);
                auto sb = createSpinboxes<ConvertQRect<QRect,QSpinBox>>(
                            container, 4, SIGNAL(valueChanged(int)));
                sb[0]->setStatusTip(tr("X Position"));
                sb[1]->setStatusTip(tr("Y Position"));
                sb[2]->setStatusTip(tr("Width"));
                sb[3]->setStatusTip(tr("Height"));
                layout->addWidget(sb[0], 0, 0);
                layout->addWidget(sb[1], 1, 0);
                layout->addWidget(sb[2], 0, 1);
                layout->addWidget(sb[3], 1, 1);
            }
            break;

            case QMetaType::QLine:
            {
                QPROPS__SUBLAYOUT(QGridLayout);
                auto sb = createSpinboxes<ConvertQLine<QLine,QSpinBox>>(
                            container, 4, SIGNAL(valueChanged(int)));
                sb[0]->setStatusTip(tr("X1 Position"));
                sb[1]->setStatusTip(tr("Y1 Position"));
                sb[2]->setStatusTip(tr("X2 Position"));
                sb[3]->setStatusTip(tr("Y2 Position"));
                layout->addWidget(sb[0], 0, 0);
                layout->addWidget(sb[1], 1, 0);
                layout->addWidget(sb[2], 0, 1);
                layout->addWidget(sb[3], 1, 1);
            }
            break;

            case QMetaType::QLineF:
            {
                QPROPS__SUBLAYOUT(QGridLayout);
                auto sb = createSpinboxes<ConvertQLine<QLineF,QDoubleSpinBox>>(
                            container, 4, SIGNAL(valueChanged(double)));
                sb[0]->setStatusTip(tr("X1 Position"));
                sb[1]->setStatusTip(tr("Y1 Position"));
                sb[2]->setStatusTip(tr("X2 Position"));
                sb[3]->setStatusTip(tr("Y2 Position"));
                layout->addWidget(sb[0], 0, 0);
                layout->addWidget(sb[1], 1, 0);
                layout->addWidget(sb[2], 0, 1);
                layout->addWidget(sb[3], 1, 1);
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
                QPROPS__SUBLAYOUT(QHBoxLayout);
                auto sb = createSpinboxes<ConvertQColor>(
                            container, 4, SIGNAL(valueChanged(int)));
                sb[0]->setStatusTip(tr("Red"));
                sb[1]->setStatusTip(tr("Green"));
                sb[2]->setStatusTip(tr("Blue"));
                sb[3]->setStatusTip(tr("Alpha"));
                for (auto s : sb)
                {
                    s->setRange(0, 255);
                    layout->addWidget(s);
                }
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

            case QMetaType::QTime:
            {
                auto e = new QTimeEdit(widget);
                edit = e;
                if (props)
                {
                    if (props->hasMin(id))
                        e->setMinimumTime(props->getMin(id).toTime());
                    if (props->hasMax(id))
                        e->setMaximumTime(props->getMax(id).toTime());
                }
                f_update_widget = [=](){ e->setTime(v.value<QTime>()); };
                f_update_value = [=](){ v = e->time(); };
                connect(e, SIGNAL(timeChanged(QTime)),
                        widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::QDateTime:
            {
                auto e = new QDateTimeEdit(widget);
                edit = e;
                if (props)
                {
                    if (props->hasMin(id))
                        e->setMinimumDateTime(props->getMin(id).toDateTime());
                    if (props->hasMax(id))
                        e->setMaximumDateTime(props->getMax(id).toDateTime());
                }
                f_update_widget = [=](){ e->setDateTime(v.value<QDateTime>()); };
                f_update_value = [=](){ v = e->dateTime(); };
                connect(e, SIGNAL(dateTimeChanged(QDateTime)),
                        widget, SLOT(onValueChanged_()));
            }
            break;

            case QMetaType::QDate:
            {
                auto e = new QDateEdit(widget);
                edit = e;
                if (props)
                {
                    if (props->hasMin(id))
                        e->setMinimumDate(props->getMin(id).toDate());
                    if (props->hasMax(id))
                        e->setMaximumDate(props->getMax(id).toDate());
                }
                f_update_widget = [=](){ e->setDate(v.value<QDate>()); };
                f_update_value = [=](){ v = e->date(); };
                connect(e, SIGNAL(dateChanged(QDate)),
                        widget, SLOT(onValueChanged_()));
            }
            break;

            default:
                isHandled = false;
            break;
        } // switch(type)
    }


    // ---- create QVector types ----

#define QPROPS__VECTOR(T__, SpinBox__, sigType__) \
    if (!isHandled && !strcmp(v.typeName(), "QVector<" #T__ ">")) \
    { \
        QPROPS__SUBLAYOUT(QHBoxLayout); \
        auto vec = v.value<QVector<T__>>(); \
        QVector<SpinBox__*> sb; \
        for (int i=0; i<vec.size(); ++i) \
        { \
            sb << new SpinBox__(container); \
            sb.back()->setRange(std::numeric_limits<T__>::min(), \
                                std::numeric_limits<T__>::max()); \
        } \
        if (props) \
        { \
            if (props->hasMin(id)) \
            { \
                auto val = props->getMin(id).value<QVector<T__>>(); \
                for (int i=0; i<std::min(vec.size(), val.size()); ++i) \
                    sb[i]->setMinimum(val[i]); \
            } \
            if (props->hasMax(id)) \
            { \
                auto val = props->getMax(id).value<QVector<T__>>(); \
                for (int i=0; i<std::min(vec.size(), val.size()); ++i) \
                    sb[i]->setMaximum(val[i]); \
            } \
            if (props->hasStep(id)) \
            { \
                auto val = props->getStep(id).value<QVector<T__>>(); \
                for (int i=0; i<std::min(vec.size(), val.size()); ++i) \
                    sb[i]->setSingleStep(val[i]); \
            } \
        } \
        f_update_widget = [=]() \
        { \
            auto val = v.value<QVector<T__>>(); \
            for (int i=0; i<vec.size(); ++i) \
                sb[i]->setValue(val[i]); \
        }; \
        f_update_value = [=]() \
        { \
            auto val = QVector<T__>(); \
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

    QPROPS__VECTOR(float,       QDoubleSpinBox, double)
    QPROPS__VECTOR(double,      QDoubleSpinBox, double)
    QPROPS__VECTOR(short,       QSpinBox, int)
    QPROPS__VECTOR(ushort,      QSpinBox, int)
    QPROPS__VECTOR(int,         QSpinBox, int)
    QPROPS__VECTOR(uint,        QSpinBox, int)
    QPROPS__VECTOR(long,        QSpinBox, int)
    QPROPS__VECTOR(ulong,       QSpinBox, int)
    QPROPS__VECTOR(qlonglong,   QSpinBox, int)
    QPROPS__VECTOR(qulonglong,  QSpinBox, int)

#undef QPROPS__VECTOR



    // ---- error-checking / finalize ----

    if (edit)
    {
        QPROPS_ASSERT(f_update_widget,
                      "No widget update defined for type '"
                      << v.typeName() << "'");
        QPROPS_ASSERT(f_update_value,
                      "No value update defined for type '"
                      << v.typeName() << "'");
        if (props)
            props->callWidgetCallback(id, edit);
        updateWidget();
        layout->addWidget(edit);
    }

    if (!isHandled)
    {
        QPROPS_PROG_ERROR("PropertyWidget: unhandled type '"
                          << v.typeName() << "'");
    }

#undef QPROPS__SUBLAYOUT
}


} // namespace QProps

#endif
