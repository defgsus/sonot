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

#include <QList>
#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>

#include "Score.h"
#include "NoteStream.h"
#include "io/error.h"

namespace Sonot {

struct Score::Private
{
    Private(Score* p)
        : p     (p)
    { }

    Score* p;

    QList<NoteStream> streams;
    QMap<QString, QVariant> properties;
};

Score::Score()
    : p_        (new Private(this))
{

}

Score::~Score()
{
    delete p_;
}

QJsonObject Score::toJson() const
{
    QJsonArray jstreams;
    for (const NoteStream& s : p_->streams)
        jstreams.append(QJsonValue(s.toJson()));

    QJsonObject jprops;
    for (auto i = p_->properties.begin(); i!=p_->properties.end(); ++i)
        jprops.insert(i.key(), QJsonValue::fromVariant(i.value()));

    QJsonObject o;
    if (!jstreams.isEmpty())
        o.insert("streams", jstreams);
    if (!jprops.isEmpty())
        o.insert("properties", jprops);

    return o;
}

void Score::fromJson(const QJsonObject& o)
{
    QList<NoteStream> streams;
    QMap<QString, QVariant> props;

    JsonHelper json("Score");

    if (o.contains("streams"))
    {
        auto jstreams = json.expectArray(json.expectChildValue(o, "streams"));
        for (int i=0; i<jstreams.size(); ++i)
        {
            NoteStream s;
            s.fromJson(json.expectObject(jstreams.at(i)));
            streams.append( s );
        }
    }

    if (o.contains("properties"))
    {
        auto jprops = json.expectObject(json.expectChildValue(o, "properties"));
        QStringList keys = jprops.keys();
        for (const QString& key : keys)
        {
            props.insert(key, jprops.value(key).toVariant());
        }
    }

    p_->streams.swap(streams);
    p_->properties.swap(props);
}

QVariant Score::property(const QString &key) const
{
    auto i = p_->properties.find(key);
    if (i == p_->properties.end())
        return QVariant();
    return i.value();
}

size_t Score::numNoteStreams() const
{
    return p_->streams.size();
}

const NoteStream& Score::noteStream(size_t idx) const
{
    SONOT_ASSERT_LT(idx, size_t(p_->streams.size()), "in Score::noteStream()");
    return p_->streams.at(idx);
}

const QList<NoteStream>& Score::noteStreams() const
{
    return p_->streams;
}


void Score::setProperty(const QString &key, const QVariant &prop)
{
    p_->properties.insert(key, prop);
}

void Score::clear()
{
    clearProperties();
    clearScore();
}

void Score::clearProperties()
{
    p_->properties.clear();
}

void Score::clearScore()
{
    p_->streams.clear();
}

void Score::appendNoteStream(const NoteStream& s)
{
    p_->streams.append(s);
}

void Score::insertNoteStream(size_t idx, const NoteStream& s)
{
    if (idx >= size_t(p_->streams.size()))
        p_->streams.append(s);
    else
        p_->streams.insert(idx, s);
}

void Score::removeNoteStream(size_t idx)
{
    SONOT_ASSERT_LT(idx, size_t(p_->streams.size()),
                    "in Score::removeNoteStream()");
    p_->streams.removeAt(idx);
}

} // namespace Sonot
