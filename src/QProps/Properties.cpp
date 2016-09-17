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

#include <tuple>

#include "Properties.h"

#if 0
#   include <QDebug>
#   define QPROPS_DEBUG_PROP(arg__) qDebug() << arg__;
#else
#   define QPROPS_DEBUG_PROP(unused__) { }
#endif

namespace QProps {

Properties::NamedValues Properties::namedValuesQtAlignment()
{
    Properties::NamedValues nv;
    nv.setIsFlags(true);
    nv.set("absolute", tr("absolute"),
           tr("No alignment"),
           int(Qt::AlignAbsolute));
    nv.set("v-center", tr("center vertically"),
           tr("Align vertically in center"),
           int(Qt::AlignVCenter));
    nv.set("h-center", tr("center horizontally"),
           tr("Align horizontally in center"),
           int(Qt::AlignHCenter));
    nv.set("left", tr("left"),
           tr("Align on left side"),
           int(Qt::AlignLeft));
    nv.set("right", tr("right"),
           tr("Align on right side"),
           int(Qt::AlignRight));
    nv.set("top", tr("top"),
           tr("Align on top"),
           int(Qt::AlignTop));
    nv.set("bottom", tr("bottom"),
           tr("Align on bottom"),
           int(Qt::AlignBottom));
    return nv;
}

Properties::NamedValues Properties::namedValuesQtTextFlag()
{
    /** @todo These are only the ones supported by QPainter::drawText() */
    Properties::NamedValues nv;
    nv.setIsFlags(true);
    nv.set("dont-clip", tr("no clipping"),
           tr("Text will not be clipped by surrounding box"),
           int(Qt::TextDontClip));
    nv.set("single-line", tr("single line"),
           tr("Treats all whitespace as spaces and prints just one line"),
           int(Qt::TextSingleLine));
    nv.set("expand-tabs", tr("expand tabs"),
           tr("Makes the tab character move to the next tab stop"),
           int(Qt::TextExpandTabs));
    nv.set("show-mnemonic", tr("show mnemonic"),
           tr("Displays the string \"&P\" as underlined P, "
              "for an ampersand, use \"&&\""),
           int(Qt::TextShowMnemonic));
    nv.set("word-wrap", tr("wrap words"),
           tr("Breaks lines at appropriate points, e.g. at word boundaries"),
           int(Qt::TextWordWrap));
    nv.set("include-spaces", tr("include traling spaces"),
           tr("When this option is set, trailing spaces will "
              "be included to align the text"),
           (Qt::TextIncludeTrailingSpaces));
    return nv;
}




// --------------------- Properties::Property ------------------------------


bool Properties::Property::operator == (const Property& o) const
{
    return Properties::qvariant_compare(p_val_, o.p_val_);
}

QList<Properties::NamedValues::Value>
    Properties::Property::namedValuesByIndex() const
{
    QList<NamedValues::Value> vals;
    for (const auto & i : p_nv_)
        vals << i;

    qSort(vals.begin(), vals.end(),
    [](const NamedValues::Value& l, const NamedValues::Value& r)
    {
        return l.index < r.index;
    });

    return vals;
}

bool Properties::Property::testFlag(const QVariant &flag) const
{
    return (flag.toLongLong() & p_val_.toLongLong()) == flag.toLongLong();
}

QString Properties::Property::toString() const
{
    if (!hasNamedValues())
        return qvariant_to_string(value());
    if (!namedValues().isFlags())
        return QString("[%1]").arg( namedValues().getByValue(value()).name );
    QString s;
    for (const NamedValues::Value& v : namedValues())
    {
        if (!testFlag(v.v))
            continue;
        if (!s.isEmpty())
            s += ",";
        s += v.name;
    }
    return "[" + s + "]";
}


// --------------------- Properties::NamedValues ------------------------------

Properties::NamedValues::NamedValues()
    : p_curIndex_   (0)
    , p_isFlags_    (false)
{ }

bool Properties::NamedValues::hasValue(const QVariant &v) const
{
    for (auto & val : p_val_)
        if (val.v == v)
            return true;
    return false;
}

const Properties::NamedValues::Value&
    Properties::NamedValues::getByValue(const QVariant &v) const
{
    for (auto & val : p_val_)
        if (val.v == v)
            return val;

    static Value invalid;
    return invalid;
}

const Properties::NamedValues::Value&
    Properties::NamedValues::get(const QString &id) const
{
    auto i = p_val_.find(id);
    static Value invalid;
    return i == p_val_.end() ? invalid : i.value();
}


void Properties::NamedValues::set(
        const QString &id, const QString &name, const QString &statusTip,
        const QVariant &v)
{
    auto i = p_val_.find(id);
    // overwrite
    if (i != p_val_.end())
    {
        i.value().id = id;
        i.value().name = name;
        i.value().tip = statusTip;
        i.value().v = v;
    }
    // create
    else
    {
        Value val;
        val.id = id;
        val.name = name;
        val.tip = statusTip;
        val.v = v;
        val.index = p_curIndex_++;
        p_val_.insert(id, val);
    }
}

void Properties::NamedValues::set(
        const QString &id, const QString &name,
        const QVariant &v)
{
    set(id, name, "", v);
}


bool Properties::Property::hasNamedValues() const
{
    return !p_nv_.p_val_.isEmpty();
}





// ---------------------------- Properties ------------------------------------

const int Properties::subTypeMask = 0xfff;

Properties::Properties(const QString& id, bool explicitJsonType)
    : p_id_                 (id)
    , p_explicitJsonTypes_  (explicitJsonType)
{
}

void Properties::swap(Properties &other)
{
    p_map_.swap(other.p_map_);
    std::swap(p_cb_vis_, other.p_cb_vis_);
    std::swap(p_id_, other.p_id_);
    std::swap(p_explicitJsonTypes_, other.p_explicitJsonTypes_);
}

bool Properties::operator != (const Properties& o) const
{
    return p_map_ != o.p_map_;
}

void Properties::clear()
{
    p_map_.clear();
    p_cb_vis_ = 0;
}

void Properties::clear(const QString &id)
{
    p_map_.remove(id);
}





const Properties::Property& Properties::getProperty(const QString &id) const
{
    static Property invalid;

    auto i = p_map_.find(id);
    if (i == p_map_.end())
        return invalid;
    else
        return i.value();
}

QList<Properties::Property*> Properties::getSortedList() const
{
    QList<Properties::Property*> list;
    for (auto& p : *this)
        list << const_cast<Property*>(&p);
    qSort(list.begin(), list.end(), [](const Property* l, const Property* r)
    {
        return l->index() < r->index();
    });
    return list;
}

QVariant Properties::get(const QString &id, const QVariant& def) const
{
    auto i = p_map_.find(id);

    return i == p_map_.end() ? def : i.value().value();
}

QVariant Properties::get(const QString &id) const
{
#define QPROPS__GETTER(mem__, ret__) \
    auto i = mem__.find(id); \
    if (i == mem__.end()) \
        QPROPS_DEBUG_PROP("Properties::get[" #mem__ "](\"" \
                         << id << "\") unknown"); \
    if (i == mem__.end()) \
        return ret__;

    QPROPS__GETTER(p_map_, QVariant());
    return i.value().value();
}

QVariant Properties::getDefault(const QString &id) const
{
    QPROPS__GETTER(p_map_, QVariant());
    return i.value().defaultValue();
}

QVariant Properties::getMin(const QString &id) const
{
    QPROPS__GETTER(p_map_, QVariant());
    return i.value().minimum();
}

QVariant Properties::getMax(const QString &id) const
{
    QPROPS__GETTER(p_map_, QVariant());
    return i.value().maximum();
}

QVariant Properties::getStep(const QString &id) const
{
    QPROPS__GETTER(p_map_, QVariant());
    return i.value().step();
}

QString Properties::getName(const QString &id) const
{
    QPROPS__GETTER(p_map_, QString());
    return i.value().name();
}


QString Properties::getTip(const QString &id) const
{
    QPROPS__GETTER(p_map_, QString());
    return i.value().tip();
}

int Properties::getSubType(const QString &id) const
{
    QPROPS__GETTER(p_map_, -1);
    return i.value().subType();
    #undef QPROPS__GETTER
}






void Properties::set(const QString &id, const QVariant & v)
{
    QPROPS_DEBUG_PROP("Properties::set '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");

#define QPROPS__GETPROP \
    auto i = p_map_.find(id); \
    if (i == p_map_.end()) \
    { \
        p_map_.insert(id, Property()); \
        i = p_map_.find(id); \
        i.value().p_id_ = id; \
        i.value().p_idx_ = p_map_.size()-1; \
    } \

    QPROPS__GETPROP
    i.value().p_val_ = v;
}


void Properties::setDefault(const QString &id, const QVariant & v)
{
    QPROPS_DEBUG_PROP("Properties::setDefault '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    QPROPS__GETPROP
    i.value().p_def_ = v;
}

void Properties::setMin(const QString &id, const QVariant & v)
{
    QPROPS_DEBUG_PROP("Properties::setMin '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    QPROPS__GETPROP
    i.value().p_min_ = v;
}

void Properties::setMax(const QString &id, const QVariant & v)
{
    QPROPS_DEBUG_PROP("Properties::setMax '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    QPROPS__GETPROP
    i.value().p_max_ = v;
}

void Properties::setRange(const QString &id, const QVariant & mi, const QVariant & ma)
{
    QPROPS_DEBUG_PROP("Properties::setRange '" << id << "': " << mi << "-" << ma << " type "
                  << mi.typeName() << " (" << mi.type() << ")");
    QPROPS__GETPROP
    i.value().p_min_ = mi;
    i.value().p_max_ = ma;
}

void Properties::setStep(const QString &id, const QVariant & v)
{
    QPROPS_DEBUG_PROP("Properties::setStep '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    QPROPS__GETPROP
    i.value().p_step_ = v;
}


void Properties::setName(const QString &id, const QString& v)
{
    QPROPS_DEBUG_PROP("Properties::setName '" << id << "': " << v << ")");
    QPROPS__GETPROP
    i.value().p_name_ = v;
}

void Properties::setTip(const QString &id, const QString& v)
{
    QPROPS_DEBUG_PROP("Properties::setTip '" << id << "': " << v << ")");
    QPROPS__GETPROP
    i.value().p_tip_ = v;
}


void Properties::setSubType(const QString &id, int v)
{
    QPROPS_DEBUG_PROP("Properties::setSubType '" << id << "': " << v << ")");
    QPROPS__GETPROP
    i.value().p_subType_ = v;
}

void Properties::setNamedValues(const QString &id, const NamedValues &names)
{
    QPROPS_DEBUG_PROP("Properties::setNamedValues '" << id << "'");
    QPROPS__GETPROP
    i.value().p_nv_ = names;
}

void Properties::clearFlags(const QString &id)
{
    QPROPS_DEBUG_PROP("Properties::clearFlags(" << id << ")");

    auto p = getProperty(id);
    if (!p.isValid() || !p.hasNamedValues() || !p.namedValues().isFlags())
    {
        QPROPS_DEBUG_PROP("Properties::clearFlags(" << id << ") for "
                         "non-flag value");
        return;
    }
    QPROPS__GETPROP
    i.value().p_val_ = QVariant(qlonglong(0));
}

void Properties::setFlags(
        const QString &id, const std::vector<QString> &flagIds)
{
    QList<QString> ids;
    for (const QString& id : flagIds)
        ids << id;
    setFlags(id, ids);
}

void Properties::setFlags(
        const QString &id, const QList<QString> &flagIds)
{
    QPROPS_DEBUG_PROP("Properties::setFlags(" << id << ", "
                     << flagIds << ")");

    auto p = getProperty(id);
    if (!p.isValid() || !p.hasNamedValues() || !p.namedValues().isFlags())
    {
        QPROPS_DEBUG_PROP("Properties::setFlags(" << id << ") for "
                         "non-flag value");
        return;
    }
    QPROPS__GETPROP

    qlonglong flags = i.value().value().toLongLong();
    for (const QString& fid : flagIds)
    {
        QVariant v = p.namedValues().get(fid).v;
        if (!v.isNull())
        {
            bool ok;
            qlonglong flag = v.toLongLong(&ok);
            //qDebug() << flag << fid;
            if (ok)
                flags |= flag;
        }
    }
#if 0
    QString k;
    for (int i = 1; i < (1<<15); i <<= 1)
    {
        k += (flags & i) != 0 ? "x" : ".";
    }
    qDebug() << flags << k;
#endif
    i.value().p_val_ = QVariant(flags);
}



void Properties::setVisible(const QString &id, bool vis)
{
    QPROPS_DEBUG_PROP("Properties::setVisible '" << id << "' " << vis);
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        i.value().p_vis_ = vis;
}

void Properties::setWidgetCallback(
        const QString &id, std::function<void (QWidget *)> f)
{
    QPROPS_DEBUG_PROP("Properties::setWidgetCallback '" << id << "'");
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        i.value().p_cb_widget_ = f;
}


bool Properties::change(const QString &id, const QVariant & v)
{
    QPROPS_DEBUG_PROP("property '" << id << "': " << v
                     << " type "<< v.typeName() << " (" << v.type() << ")");
    auto i = p_map_.find(id);
    if (i == p_map_.end())
        return false;
    i.value().p_val_ = v;
    return true;
}

void Properties::setDefault(const QString &id)
{
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        i.value().p_val_ = i.value().p_def_;
}

void Properties::setDefault()
{
    for (auto i = p_map_.begin(); i!=p_map_.end(); ++i)
    {
        i.value().p_val_ = i.value().p_def_;
    }
}

void Properties::unify(const Properties &other)
{
    for (auto i = other.p_map_.begin(); i != other.p_map_.end(); ++i)
    {
        auto j = p_map_.find(i.key());
        if (j == p_map_.end())
            p_map_.insert(i.key(), i.value());
        else
            j.value().p_val_ = i.value().p_val_;
    }
}

void Properties::updateFrom(const Properties &other)
{
    for (auto i = other.p_map_.begin(); i != other.p_map_.end(); ++i)
    {
        auto j = p_map_.find(i.key());
        if (j != p_map_.end())
            j.value().p_val_ = i.value().p_val_;
    }
}

bool Properties::isVisible(const QString &id) const
{
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        return i.value().isVisible();
    return false;
}

bool Properties::testFlag(const QString &id, const QVariant &flag) const
{
    auto p = getProperty(id);
    if (p.isValid())
        return p.testFlag(flag);
    else
        return false;
}

QString Properties::toString(const QString &indent) const
{
    QString r;
    for (auto i = begin(); i != end(); ++i)
    {
        r += indent + i.key() + ": " + i.value().toString()
                + "; /* " + QString::number(i.value().value().type())
                + " " + i.value().value().typeName();
        if (i.value().hasNamedValues())
            r += " (" + QString::number(i.value().namedValues().p_val_.size())
                    + " NamedValues)";
        if (!i.value().isVisible())
            r += " (off)";
        r += " */\n";
    }
    return r;
}

QString Properties::toCompactString() const
{
    QString r;
    for (auto i = begin(); i != end(); ++i)
    {
        if (!r.isEmpty())
            r += ", ";
        r += i.key() + ":" + i.value().toString();
    }
    return r;
}


bool Properties::callUpdateVisibility()
{
    if (!p_cb_vis_)
        return false;

    // store state
    std::vector<bool> vis;
    for (auto & p : *this)
        vis.push_back(p.isVisible());

    p_cb_vis_(*this);

    // check difference
    auto v = vis.begin();
    for (auto & p : *this)
        if (p.isVisible() != *v++)
            return true;
    return false;
}

void Properties::callWidgetCallback(const QString & id, QWidget * w) const
{
    if (w)
    {
        auto i = p_map_.find(id);
        if (i != p_map_.end() && i.value().p_cb_widget_)
            i.value().p_cb_widget_(w);
    }
}


} // namespace QProps
