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

#ifndef QPROPS_SRC_PROPERTIES_H
#define QPROPS_SRC_PROPERTIES_H

#include <functional>

#include <QtCore>
#include <QVariant>
#include <QMap>
#include <QRectF>

#include "JsonInterface.h"

class QWidget;

namespace QProps {


/** Generic property container based on QVariant

    <p>The properties are indexed by an id string and
    support name, tooltip, default value, min, max, stepsize.</p>
*/
class Properties : public JsonInterface
{
    Q_DECLARE_TR_FUNCTIONS(Properties)
public:

    // --------------- types --------------------

    enum SubType
    {
        ST_ANY = 0,
        ST_TEXT = 0x1000,
        ST_FILENAME = 0x2000
    };
    static const int subTypeMask;


    /** A class for handling persistent
        choose-one-of-different-values properties */
    class NamedValues
    {
    public:
        struct Value
        {
            bool isValid() const { return v.isValid(); }
            QString id, name, tip;
            QVariant v;
            size_t index;
        };

        NamedValues();

        bool has(const QString &id) const { return p_val_.contains(id); }
        bool hasValue(const QVariant &v) const;
        /** Returns the Value for id, or an invalid Value */
        const Value& get(const QString& id) const;
        /** Returns the Value matching v, or an invalid Value */
        const Value& getByValue(const QVariant& v) const;

        /** When true, values can be OR-ed together */
        bool isFlags() const { return p_isFlags_; }

        void set(const QString& id, const QString& name,
                 const QVariant& v);
        void set(const QString& id, const QString& name,
                 const QString& statusTip, const QVariant& v);

        /** Enable the use of the values as flags.
            When true, values should be OR-able.
            @note values will be treated as qlonglong for
            flag testing and json i/o. */
        void setIsFlags(bool e) { p_isFlags_ = e; }

        /** Access to values, sorted by id */
        QMap<QString, Value>::const_iterator begin() const
            { return p_val_.cbegin(); }
        /** Access to values, sorted by id */
        QMap<QString, Value>::const_iterator end() const
            { return p_val_.cend(); }

    private:
        friend Properties;
        QMap<QString, Value> p_val_;
        size_t p_curIndex_;
        bool p_isFlags_;
    };

    /** Returns NamedValues for Qt::AlignmentFlag */
    static NamedValues namedValuesQtAlignment();

    /** Returns NamedValues for Qt::TextFlag */
    static NamedValues namedValuesQtTextFlag();


    struct Property
    {
        Property() : p_subType_(-1), p_idx_(0), p_vis_(true) { }

        /** @{ */
        /** Compares this and @p o.
            Comparison only compares the actual Property value, not
            the name, description, min/max or anything else.
            Floating point numbers use a fuzzy comparison. */
        bool operator == (const Property& o) const;
        bool operator != (const Property& o) const { return !(*this == o); }
        /* @} */

        bool isValid() const { return !p_val_.isNull(); }

        const QVariant& value() const { return p_val_; }
        const QVariant& defaultValue() const { return p_def_; }
        const QVariant& minimum() const { return p_min_; }
        const QVariant& maximum() const { return p_max_; }
        const QVariant& step() const { return p_step_; }

        const QString& id() const { return p_id_; }
        const QString& name() const { return p_name_; }
        const QString& tip() const { return p_tip_; }

        int subType() const { return p_subType_; }

        bool hasNamedValues() const;
        /** Associated NamedValues class, if any */
        const NamedValues& namedValues() const { return p_nv_; }
        /** Associated NamedValues class, if any, sorted by creation index */
        QList<NamedValues::Value> namedValuesByIndex() const;

        /** Tests if @p flag is contained in the current value.
            The current value is interpreted as qlonglong OR-combination.
            @see NamedValues::setIsFlags() */
        bool testFlag(const QVariant& flag) const;

        /** Returns the value of this Property as string */
        QString toString() const;

        // ---- gui stuff ----

        /** The order of creation */
        int index() const { return p_idx_; }
        bool isVisible() const { return p_vis_; }

        /** A user-callback for the created edit widget */
        const std::function<void(QWidget*)>&
            widgetCallback() const { return p_cb_widget_; }

    private:
        friend class Properties;
        QVariant
            p_val_,
            p_def_,
            p_min_,
            p_max_,
            p_step_;
        QString
            p_id_,
            p_name_,
            p_tip_;
        int p_subType_,
            p_idx_;
        bool
            p_vis_;
        NamedValues
            p_nv_;
        std::function<void(QWidget*)>
            p_cb_widget_;
    };

    /** The default key/value map used for all Properties */
    typedef QMap<QString, Property> Map;

    // -------------- ctor ----------------------

    /**
       @brief Constructs a Properties container.
       @param id The id of the Properties collection.
       @param explicitJsonType Should exact type infos be stored
              for each Property.
              It is highly suggested to leave this value true
              if you want to deserialize Properties and get the
              same type as stored.
              @see JsonHelper::wrap(QVariant, bool)
     */
    Properties(const QString& id, bool explicitJsonType = true);

    void swap(Properties& other);

    /** @{ */
    /** Compares this and @p o.
        Comparison only compares the actual Property values, not
        the name, description, min/max or anything else.
        Floating point numbers use a fuzzy comparison. */
    bool operator != (const Properties& o) const;
    bool operator == (const Properties& o) const { return !((*this) != o); }
    /** @} */

    // ------------------ io --------------------

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;

    // ------------- static helper --------------

    /** Returns the string representation of all QVariant types */
    static QString qvariant_to_string(const QVariant&);

    /** Returns true if the two QVariants match.
        Qt does not support comparison for all QVariant types by default,
        e.g. QVector... */
    static bool qvariant_compare(const QVariant&, const QVariant&);

    // ---------------- getter ------------------

    bool isEmpty() const { return p_map_.isEmpty(); }
    bool isExplicitJsonTypes() const { return p_explicitJsonTypes_; }

    /** Returns the Property for the given id, or an invalid Property */
    const Property& getProperty(const QString& id) const;

    /** Returns a list of all properties in creation order */
    QList<Property*> getSortedList() const;

    /** Returns the given property, or an invalid QVariant */
    QVariant get(const QString& id) const;

    /** Returns the given property, or the given default value. */
    QVariant get(const QString& id, const QVariant& def) const;

    /** Returns the default value for the property, or an invalid QVariant */
    QVariant getDefault(const QString& id) const;
    QVariant getMin(const QString& id) const;
    QVariant getMax(const QString& id) const;
    QVariant getStep(const QString& id) const;
    QString getName(const QString& id) const;
    QString getTip(const QString& id) const;
    /** Returns the subtype of the value.
        Some value types may have a associated sub-type:
        QString:
            ST_TEXT | MO::TextType (in object/object_fwd.h)
            ST_FILENAME | MO::IO::FileType (in io/filetypes.h)
        @returns -1 if not defined.
    */
    int getSubType(const QString& id) const;

    /** Returns true, when there is a property named @p id */
    bool has(const QString& id) const { return p_map_.contains(id); }

    /** Returns the default value for the property, or an invalid QVariant */
    bool hasDefault(const QString& id) const
        { return getProperty(id).defaultValue().isValid(); }
    bool hasMin(const QString& id) const
        { return getProperty(id).minimum().isValid(); }
    bool hasMax(const QString& id) const
        { return getProperty(id).maximum().isValid(); }
    bool hasStep(const QString& id) const
        { return getProperty(id).step().isValid(); }
    bool hasName(const QString& id) const
        { return !getProperty(id).name().isNull(); }
    bool hasTip(const QString& id) const
        { return !getProperty(id).tip().isNull(); }
    bool hasSubType(const QString& id) const
        { return getProperty(id).subType() != -1; }
    bool isVisible(const QString& id) const;

    /** Tests if @p flag is contained in the current value of @p id.
        The current value is interpreted as qlonglong OR-combination.
        @note The value must have NamedValues which must be set
        to be of flags type.
        @see Property::testFlag(), NamedValues::setIsFlags() */
    bool testFlag(const QString& id, const QVariant& flag) const;

    /** Returns a css-style list of all properties */
    QString toString(const QString& indent = "") const;
    /** Returns a single line of all Property string representations */
    QString toCompactString() const;

    // -------------- iterator ------------------

    Map::const_iterator begin() const { return p_map_.constBegin(); }
    Map::const_iterator end() const { return p_map_.constEnd(); }

    // --------------- setter -------------------

    /** Wipes out everything */
    void clear();

    /** Removes a single item */
    void clear(const QString& id);

    /** Sets the Properties::id() */
    void setId(const QString& properties_id) { p_id_ = properties_id; }

    void setExplicitJsonTypes(bool e) { p_explicitJsonTypes_ = e; }

    /** Sets the given property (and default value) */
    void set(const QString& id, const QVariant& v);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue);

    /** @{ */
    /** Initializers for integral or float/double types or
        supported compound types of integral or float/double */

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& step);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& minimum, const T& maximum);

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const T& defaultValue, const T& minimum, const T& maximum,
             const T& step);
    /** @} */


    /** @{ */
    /** NamedValues */

    template <class T>
    void set(const QString& id, const QString& name, const QString& statusTip,
             const NamedValues& names, const T& v);

    /** Clears the flags of Property @p id
        if Property::namedValues().isFlags() is true */
    void clearFlags(const QString& id);

    /** ORs the value of Property @p id with the NamedValues identified
        by @p flagIds, if Property::namedValues().isFlags() is true */
    void setFlags(const QString& id, const QList<QString>& flagIds);
    void setFlags(const QString& id, const std::vector<QString>& flagIds);

    /** @} */


    /** Sets the given default value */
    void setDefault(const QString& id, const QVariant& v);
    void setMin(const QString& id, const QVariant& v);
    void setMax(const QString& id, const QVariant& v);
    void setRange(const QString& id, const QVariant& mi, const QVariant& ma);
    void setStep(const QString& id, const QVariant& v);
    void setName(const QString& id, const QString& name);
    void setTip(const QString& id, const QString& statusTip);
    void setSubType(const QString& id, int t);
    void setNamedValues(const QString& id, const NamedValues& names);
    void setVisible(const QString& id, bool vis);

    /** Sets the given property if existing. */
    bool change(const QString& id, const QVariant& v);

    /** @{ */
    /** Overloads to make sure, that user-extended QVariants get caught. */
    template <class T>
    void set(const QString& id, const T& v)
        { set(id, QVariant::fromValue(v)); }
    template <class T>
    bool change(const QString& id, const T& v)
        { return change(id, QVariant::fromValue(v)); }
    template <class T>
    void setDefault(const QString& id, const T& v)
        { setDefault(id, QVariant::fromValue(v)); }
    template <class T>
    void setMin(const QString& id, const T& v)
        { setMin(id, QVariant::fromValue(v)); }
    template <class T>
    void setMax(const QString& id, const T& v)
        { setMax(id, QVariant::fromValue(v)); }
    template <class T>
    void setRange(const QString& id, const T& mi, const T& ma)
        { setRange(id, QVariant::fromValue(mi), QVariant::fromValue(ma)); }
    template <class T>
    void setStep(const QString& id, const T& v)
        { setStep(id, QVariant::fromValue(v)); }
    /** @} */

    /** Revert the value to it's default value */
    void setDefault(const QString& id);

    /** Revert all values to their default value */
    void setDefault();

    /** Copy all values from @p other.
        This creates or overwrites values for each value
        contained in @p other. */
    void unify(const Properties& other);
    /** Copy values from @p other that are already present in this. */
    void updateFrom(const Properties& other);

    /** Create a union of this and @p other,
        while prefering other's values over own. */
    Properties unified(const Properties& other) const
        { Properties p(*this); p.unify(other); return p; }

    // --------- callbacks -----------

    /** Installs a callback that is called by the gui to update
        the visibility of widgets after a value is changed.
        Changes other than to Property::setVisible() will not be
        reflected in the gui. */
    void setUpdateVisibilityCallback(
            std::function<void(Properties&)> f) { p_cb_vis_ = f; }

    /** Calls the user-callback and returns true if the visibility
        of any widget has changed. */
    bool callUpdateVisibility();

    /** Sets a callback that is called for the particular widget that
        is created for this property. The callback is called once after
        creation of the widget. */
    void setWidgetCallback(
            const QString& id, std::function<void(QWidget*)> f);

    /** Applies the user-callback from setWidgetCallback()
        to the given widget, if any. */
    void callWidgetCallback(const QString& id, QWidget * ) const;


private:

    Map p_map_;
    std::function<void(Properties&)> p_cb_vis_;
    QString p_id_;
    bool p_explicitJsonTypes_;
};

} // namespace QProps


//Q_DECLARE_METATYPE(MO::Properties::NamedValues)



// --------- templ impl. ------------------

namespace QProps {

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue)
{
    set(id, defaultValue);
    setDefault(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue, const T& step)
{
    set(id, defaultValue);
    setDefault(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
    setStep(id, step);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue, const T& minimum, const T& maximum)
{
    set(id, defaultValue);
    setDefault(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
    setRange(id, minimum, maximum);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const T& defaultValue, const T& minimum, const T& maximum, const T& step)
{
    set(id, defaultValue);
    setDefault(id, defaultValue);
    setName(id, name);
    setTip(id, statusTip);
    setRange(id, minimum, maximum);
    setStep(id, step);
}

template <class T>
void Properties::set(
        const QString& id, const QString& name, const QString& statusTip,
        const NamedValues& names, const T& v)
{
    set(id, v);
    setDefault(id, v);
    setName(id, name);
    setTip(id, statusTip);
    setNamedValues(id, names);
}

} // namespace QProps

#endif // QPROPS_SRC_PROPERTIES_H
