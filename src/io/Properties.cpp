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

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "Properties.h"
#include "error.h"

#if 0
#   include <QDebug>
#   define SONOT_DEBUG_PROP(arg__) qDebug() << arg__;
#else
#   define SONOT_DEBUG_PROP(unused__) { }
#endif

namespace Sonot {

#if 0
bool Properties::isAlignment(const QVariant & v) { return !strcmp(v.typeName(), "MO::Properties::Alignment"); }
const Properties::NamedStates Properties::alignmentStates = Properties::NamedStates("Alignment",
{
                { "left",           QVariant::fromValue(Alignment(A_LEFT)) },
                { "right",          QVariant::fromValue(Alignment(A_RIGHT)) },
                { "top",            QVariant::fromValue(Alignment(A_TOP)) },
                { "bottom",         QVariant::fromValue(Alignment(A_BOTTOM)) },
                { "top-left",       QVariant::fromValue(Alignment(A_TOP | A_LEFT)) },
                { "top-right",      QVariant::fromValue(Alignment(A_TOP | A_RIGHT)) },
                { "bottom-left",    QVariant::fromValue(Alignment(A_BOTTOM | A_LEFT)) },
                { "bottom-right",   QVariant::fromValue(Alignment(A_BOTTOM | A_RIGHT)) },
                { "vcenter-left",   QVariant::fromValue(Alignment(A_VCENTER | A_LEFT)) },
                { "vcenter-right",  QVariant::fromValue(Alignment(A_VCENTER | A_RIGHT)) },
                { "top-hcenter",    QVariant::fromValue(Alignment(A_TOP | A_HCENTER)) },
                { "bottom-hcenter", QVariant::fromValue(Alignment(A_BOTTOM | A_HCENTER)) },
                { "center",         QVariant::fromValue(Alignment(A_CENTER)) },
});
#endif


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


// --------------------- Properties::NamedValues ------------------------------

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

Properties::Properties(const QString& id)
    : p_id_     (id)
{
}

void Properties::swap(Properties &other)
{
    p_map_.swap(other.p_map_);
    std::swap(p_cb_vis_, other.p_cb_vis_);
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


QJsonObject Properties::toJson() const
{
    QJsonObject main;
    main.insert("id", p_id_);

    QJsonArray array;
    for (const Property& p : p_map_)
    {
        QJsonObject o;
        o.insert("id", p.id());
        if (p.hasNamedValues())
        {
            // write value-id of NamedValue
            auto nv = p.namedValues().getByValue(p.value());
            o.insert("nv", nv.id);
        }
        else
        {
            // write value from QVariant
            o.insert("v", QJsonValue::fromVariant(p.value()));
        }

        array.append(o);
    }
    main.insert("props", array);

    return main;
}

void Properties::fromJson(const QJsonObject& main)
{
    JsonHelper json("Properties");
    Properties tmp(*this);

    QJsonArray array = json.expectChildArray(main, "props");

    for (int i=0; i<array.size(); ++i)
    {
        QJsonObject o = json.expectObject(array.at(i));

        QString id = json.expectChild<QString>(o, "id");
        Property p = tmp.getProperty(id);

        if (p.hasNamedValues())
        {
            QString vid = json.expectChild<QString>(o, "nv");
            if (!p.namedValues().has(vid))
                SONOT_IO_ERROR("Unknown value-id '" << id << "' in json "
                               "for property '" << id << "'");
            tmp.set(id, p.namedValues().get(vid).v);
        }
        else
        {
            QVariant v = json.expectChildValue(o, "v").toVariant();
            // convert to runtime-type
            if (p.isValid())
            {
                auto vtype = p.value().type();
                if (vtype != v.type() && v.convert(vtype))
                {
                    SONOT_IO_ERROR("Unexpected value type '" << v.typeName()
                                   << "' in json for property '" << id << "', "
                                   "expected '" << p.value().typeName() << "'");
                }
            }
            tmp.set(id, v);
        }

    }

    *this = tmp;
}


#if 0
void Properties::serialize(IO::DataStream & io) const
{
    io.writeHeader("props", 1);

    // Note: Reimplementation of QDataStream << QMap<QString,QVariant>
    // because overloading operator << does not catch enums
    io << quint64(p_map_.size());
    for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
    {
        io << i.key();

        // store id of named values
        if (i.value().hasNamedValues())
        {
            auto nv = i.value().namedValues().getByValue(
                        i.value().value());
            io << qint8(1) << nv.id;
        }

        // non-user types use the QVariant version
        else
        if (QMetaType::Type(i.value().value().type()) < QMetaType::User)
            io << qint8(0) << i.value().value();

        // handle special compound types
#define SONOT__IO(type__, flag__) \
        if (!strcmp(i.value().value().typeName(), #type__)) \
        { \
            io << qint8(flag__) \
               << i.value().value().value<type__>(); \
        }
        else
        SONOT__IO(QVector<float>, 24)
        else
        SONOT__IO(QVector<double>, 25)
        else
        SONOT__IO(QVector<int>, 26)
        else
        SONOT__IO(QVector<uint>, 27)
        else
        SONOT__IO(QVector<unsigned>, 28)

        else
        {
            io << qint8(-1);
            MO_IO_WARNING(WRITE, "Properties::serialize() unhandled QVariant '"
                       << i.value().value().typeName() << "'");
        }
#undef SONOT__IO

    }
}

void Properties::deserialize(IO::DataStream & io)
{
    //const auto ver =
            io.readHeader("props", 1);

    // reconstruct map
    Map temp;
    quint64 num;
    io >> num;
    for (quint64 i=0; i<num; ++i)
    {
        QString key;
        qint8 type;
        QVariant v;
        io >> key >> type;
        if (type == -1)
        {
            MO_IO_WARNING(READ, "Properties::deserialize() can't restore "
                       "unhandled QVariant for '" << key << "'");
            continue;
        }

#define SONOT__IO(type__, flag__) \
        if (type == flag__) \
        { \
            type__ val__; \
            io >> val__; \
            v = QVariant::fromValue(val__); \
        }

        // default QVariant
        if (type == 0)
            io >> v;

        // named values
        else if (type == 1)
        {
            QString id;
            io >> id;
            if (!has(key))
            {
                MO_IO_WARNING(READ, "Properties::deserialize() '" << key
                              << "' unknown named-value, ignored");
                continue;
            }
            auto p = getProperty(key);
            if (!p.hasNamedValues())
            {
                MO_IO_WARNING(READ, "Properties::deserialize() '" << key
                              << "' is expected to be a named value but is not, ignored");
                continue;
            }
            if (!p.namedValues().has(id))
            {
                MO_IO_WARNING(READ, "Properties::deserialize() '" << key
                              << "' unknown named-value id '" << id << "', ignored");
                continue;
            }
            v = p.namedValues().get(id).v;
        }

        // user types
        else
        SONOT__IO(QVector<float>, 24)
        else
        SONOT__IO(QVector<double>, 25)
        else
        SONOT__IO(QVector<int>, 26)
        else
        SONOT__IO(QVector<uint>, 27)
        else
        SONOT__IO(QVector<unsigned>, 28)

        // unknown
        else
        {
            MO_IO_WARNING(READ, "Properties::deserialize() ignoring unknown usertype " << type
                       << " for item '" << key << "'");
            continue;
        }

        Property prop;
        prop.p_val_ = v;
        temp.insert(key, prop);
    }

    // finally assign
    for (auto i = temp.begin(); i != temp.end(); ++i)
    {
        set(i.key(), i.value().value());
    }
}

void Properties::serialize(IO::XmlStream & io) const
{
    io.createSection("properties");

        io.write("version", 1);

        for (auto i = p_map_.begin(); i != p_map_.end(); ++i)
        {
            io.createSection("property");

                io.write("id", i.key());

                // named values are stored by id
                if (i.value().hasNamedValues())
                {
                    auto nv = i.value().namedValues().getByValue(
                                i.value().value());
                    io.write("nv", nv.id);
                }

                // for everything else we count on XmlStream to handle the QVariant
                else
                //if (QMetaType::Type(i.value().value().type()) < QMetaType::User)
                    io.write("v", i.value().value());
#if 0
                else
                {
                    MO_IO_WARNING(WRITE, "Properties::serialize() unhandled QVariant '"
                               << i.value().value().typeName() << "'");
                }
#endif
            io.endSection();
        }

    io.endSection();
}

/** @todo move to io/streamoperators_qt.h */
std::ostream& operator << (std::ostream& s, const QVariant& v)
{
    QString val;
    if (v.type() == QVariant::Size)
    {
        auto t = v.toSize();
        val = QString("%1,%2").arg(t.width()).arg(t.height());
    }
    else
        val = v.toString();

    s << "QVariant(" << val << "/" << v.typeName() << ")";
    return s;
}


void Properties::deserialize(IO::XmlStream& io)
{
    Properties tmp;

    io.verifySection("properties");

        const int ver = io.expectInt("version");
        Q_UNUSED(ver);

        while (io.nextSubSection())
        {
            if (io.section() == "property")
            {
                QString id = io.expectString("id");

                // general variant value?
                if (io.hasAttribute("v"))
                {
                    QVariant v = io.expectVariant("v");
                    //MO_DEBUG("read " << id << ", " << v);
                    tmp.set(id, v);
                }

                // named value
                else if (io.hasAttribute("nv"))
                {
                    QString nv = io.expectString("nv");

                    if (!has(id))
                    {
                        MO_IO_WARNING(READ, "Properties::deserialize() '" << id
                                      << "' unknown named-value, ignored");
                        io.leaveSection();
                        continue;
                    }
                    auto p = getProperty(id);
                    if (!p.hasNamedValues())
                    {
                        MO_IO_WARNING(READ, "Properties::deserialize() '" << id
                                      << "' is expected to be a named value but is not, ignored");
                        io.leaveSection();
                        continue;
                    }
                    if (!p.namedValues().has(nv))
                    {
                        MO_IO_WARNING(READ, "Properties::deserialize() '" << id
                                      << "' unknown named-value id '" << nv << "', ignored");
                        io.leaveSection();
                        continue;
                    }
                    tmp.set(id, p.namedValues().get(nv).v);
                }
            }

            io.leaveSection();
        }

    // get all new values
    unify(tmp);
}
#endif


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
#define SONOT__GETTER(mem__, ret__) \
    auto i = mem__.find(id); \
    if (i == mem__.end()) \
        SONOT_DEBUG_PROP("Properties::get[" #mem__ "](\"" << id << "\") unknown"); \
    if (i == mem__.end()) \
        return ret__;

    SONOT__GETTER(p_map_, QVariant());
    return i.value().value();
}

QVariant Properties::getDefault(const QString &id) const
{
    SONOT__GETTER(p_map_, QVariant());
    return i.value().defaultValue();
}

QVariant Properties::getMin(const QString &id) const
{
    SONOT__GETTER(p_map_, QVariant());
    return i.value().minimum();
}

QVariant Properties::getMax(const QString &id) const
{
    SONOT__GETTER(p_map_, QVariant());
    return i.value().maximum();
}

QVariant Properties::getStep(const QString &id) const
{
    SONOT__GETTER(p_map_, QVariant());
    return i.value().step();
}

QString Properties::getName(const QString &id) const
{
    SONOT__GETTER(p_map_, QString());
    return i.value().name();
}


QString Properties::getTip(const QString &id) const
{
    SONOT__GETTER(p_map_, QString());
    return i.value().tip();
}

int Properties::getSubType(const QString &id) const
{
    SONOT__GETTER(p_map_, -1);
    return i.value().subType();
    #undef SONOT__GETTER
}






void Properties::set(const QString &id, const QVariant & v)
{
    SONOT_DEBUG_PROP("Properties::set '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");

#define SONOT__GETPROP \
    auto i = p_map_.find(id); \
    if (i == p_map_.end()) \
    { \
        p_map_.insert(id, Property()); \
        i = p_map_.find(id); \
        i.value().p_id_ = id; \
        i.value().p_idx_ = p_map_.size()-1; \
    } \

    SONOT__GETPROP
    i.value().p_val_ = v;
}


void Properties::setDefault(const QString &id, const QVariant & v)
{
    SONOT_DEBUG_PROP("Properties::setDefault '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    SONOT__GETPROP
    i.value().p_def_ = v;
}

void Properties::setMin(const QString &id, const QVariant & v)
{
    SONOT_DEBUG_PROP("Properties::setMin '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    SONOT__GETPROP
    i.value().p_min_ = v;
}

void Properties::setMax(const QString &id, const QVariant & v)
{
    SONOT_DEBUG_PROP("Properties::setMax '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    SONOT__GETPROP
    i.value().p_max_ = v;
}

void Properties::setRange(const QString &id, const QVariant & mi, const QVariant & ma)
{
    SONOT_DEBUG_PROP("Properties::setRange '" << id << "': " << mi << "-" << ma << " type "
                  << mi.typeName() << " (" << mi.type() << ")");
    SONOT__GETPROP
    i.value().p_min_ = mi;
    i.value().p_max_ = ma;
}

void Properties::setStep(const QString &id, const QVariant & v)
{
    SONOT_DEBUG_PROP("Properties::setStep '" << id << "': " << v << " type "
                  << v.typeName() << " (" << v.type() << ")");
    SONOT__GETPROP
    i.value().p_step_ = v;
}


void Properties::setName(const QString &id, const QString& v)
{
    SONOT_DEBUG_PROP("Properties::setName '" << id << "': " << v << ")");
    SONOT__GETPROP
    i.value().p_name_ = v;
}

void Properties::setTip(const QString &id, const QString& v)
{
    SONOT_DEBUG_PROP("Properties::setTip '" << id << "': " << v << ")");
    SONOT__GETPROP
    i.value().p_tip_ = v;
}


void Properties::setSubType(const QString &id, int v)
{
    SONOT_DEBUG_PROP("Properties::setSubType '" << id << "': " << v << ")");
    SONOT__GETPROP
    i.value().p_subType_ = v;
}

void Properties::setNamedValues(const QString &id, const NamedValues &names)
{
    SONOT_DEBUG_PROP("Properties::setNamedValues '" << id << "'");
    SONOT__GETPROP
    i.value().p_nv_ = names;
}

void Properties::setVisible(const QString &id, bool vis)
{
    SONOT_DEBUG_PROP("Properties::setVisible '" << id << "' " << vis);
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        i.value().p_vis_ = vis;
}

void Properties::setWidgetCallback(
        const QString &id, std::function<void (QWidget *)> f)
{
    SONOT_DEBUG_PROP("Properties::setWidgetCallback '" << id << "'");
    auto i = p_map_.find(id);
    if (i != p_map_.end())
        i.value().p_cb_widget_ = f;
}


bool Properties::change(const QString &id, const QVariant & v)
{
    SONOT_DEBUG_PROP("property '" << id << "': " << v << " type "<< v.typeName() << " (" << v.type() << ")");
    auto i = p_map_.find(id);
    if (i == p_map_.end())
        return false;
    i.value().p_val_ = v;
    return true;
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

QString Properties::toString(const QString &indent) const
{
    QString r;
    for (auto i = begin(); i != end(); ++i)
    {
        /** @todo print correct value for all types */
        r += indent + i.key() + ": " + i.value().value().toString()
                + "; // " + QString::number(i.value().value().type())
                + " " + i.value().value().typeName();
        if (i.value().hasNamedValues())
            r += " (" + QString::number(i.value().namedValues().p_val_.size())
                    + " NamedValues)";
        if (!i.value().isVisible())
            r += " (off)";
        r += "\n";
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

#if 0
QRectF Properties::align(const QRectF &rect, const QRectF &parent,
                         int alignment, qreal margin, bool outside)
{
    QRectF r(rect);

    // h&v center (ignore outside flag and margin)
    if ((alignment & A_CENTER) == A_CENTER)
    {
        r.moveLeft( parent.left() + (parent.width() - rect.width()) / 2.);
        r.moveTop( parent.top() + (parent.height() - rect.height()) / 2.);
        return r;
    }

    if (outside)
        margin = -margin;

    r.moveTopLeft(parent.topLeft() + QPointF(
                      margin - (outside ? rect.width() : 0),
                      margin - (outside ? rect.height() : 0)));

    // hcenter or right (it's already left)
    if ((alignment & A_HCENTER) == A_HCENTER)
        r.moveLeft( parent.left() + (parent.width() - rect.width()) / 2.);
    else if (alignment & A_RIGHT)
        r.moveRight( parent.right() - margin
                     + (outside ? rect.width() : 0));

    // vcenter or bottom (it's already top)
    if ((alignment & A_VCENTER) == A_VCENTER)
        r.moveTop( parent.top() + (parent.height() - rect.height()) / 2.);
    else if (alignment & A_BOTTOM)
        r.moveBottom( parent.bottom() - margin
                      + (outside ? rect.height() : 0));

    return r;
}
#endif


} // namespace Sonot
