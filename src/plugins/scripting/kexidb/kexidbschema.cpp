/***************************************************************************
 * kexidbschema.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "kexidbschema.h"
#include "kexidbfieldlist.h"

#include <QRegularExpression>
#include <QDebug>

#include <KDbExpression>

using namespace Scripting;

/***************************************************************************
 *KexiDBSchema
 */

KexiDBSchema::KexiDBSchema(QObject* parent, const QString& name, KDbObject* schema, KDbFieldList* fieldlist, bool owner)
        : QObject(parent)
        , m_schema(schema)
        , m_fieldlist(fieldlist)
        , m_owner(owner)
{
    setObjectName(name);
}

KexiDBSchema::~KexiDBSchema()
{
}

QString KexiDBSchema::name() const
{
    return m_schema->name();
}
void KexiDBSchema::setName(const QString& name)
{
    m_schema->setName(name);
}
QString KexiDBSchema::caption() const
{
    return m_schema->caption();
}
void KexiDBSchema::setCaption(const QString& caption)
{
    m_schema->setCaption(caption);
}
QString KexiDBSchema::description() const
{
    return m_schema->description();
}
void KexiDBSchema::setDescription(const QString& description)
{
    m_schema->setDescription(description);
}
QObject* KexiDBSchema::fieldlist()
{
    return new KexiDBFieldList(this, m_fieldlist, false);
}

/***************************************************************************
 * KexiDBTableSchema
 */

KexiDBTableSchema::KexiDBTableSchema(QObject* parent, KDbTableSchema* tableschema, bool owner)
        : KexiDBSchema(parent, "KexiDBTableSchema", tableschema, tableschema, owner)
{
}

KexiDBTableSchema::~KexiDBTableSchema()
{
    if (m_owner)
        delete tableschema();
}

KDbTableSchema* KexiDBTableSchema::tableschema()
{
    return static_cast< KDbTableSchema* >(m_schema);
}

QObject* KexiDBTableSchema::query()
{
    return new KexiDBQuerySchema(this, tableschema()->query(), false);
}

/***************************************************************************
 * KexiDBQuerySchema
 */

KexiDBQuerySchema::KexiDBQuerySchema(QObject* parent, KDbQuerySchema* queryschema, bool owner)
        : KexiDBSchema(parent, "KexiDBQuerySchema", queryschema, queryschema, owner)
{
}

KexiDBQuerySchema::~KexiDBQuerySchema()
{
    if (m_owner)
        delete queryschema();
}

KDbQuerySchema* KexiDBQuerySchema::queryschema()
{
    return static_cast< KDbQuerySchema* >(m_schema);
}

QString KexiDBQuerySchema::statement() const
{
    return static_cast< KDbQuerySchema* >(m_schema)->statement().toString();
}

void KexiDBQuerySchema::setStatement(const QString& statement)
{
    static_cast< KDbQuerySchema* >(m_schema)->setStatement(KDbEscapedString(statement));
}

bool KexiDBQuerySchema::setWhereExpression(const QString& whereexpression)
{
    ///@todo use KDbParser for such kind of parser-functionality.
    QString s = whereexpression;
    QRegularExpression re("[\"',]{1,1}");
    while (true) {
        s.remove(QRegularExpression("^[\\s,]+"));
        int pos = s.indexOf('=');
        if (pos < 0) break;
        QString key = s.left(pos).trimmed();
        s = s.mid(pos + 1).trimmed();

        QString value;
        QRegularExpressionMatch match = re.match(s);
        int sp = s.indexOf(re);
        
        if (match.hasMatch()) {
            if (match.captured(1) == ",") {
                value = s.left(sp).trimmed();
                s = s.mid(sp + 1).trimmed();
            } else {
                int ep = s.indexOf(match.captured(1), sp + 1);
                value = s.mid(sp + 1, ep - 1);
                s = s.mid(ep + 1);
            }
        } else {
            value = s;
            s.clear();
        }

        KDbField* field = static_cast< KDbQuerySchema* >(m_schema)->field(key);
        if (! field) {
            qWarning() << QString("Invalid WHERE-expression: Field \"%1\" does not exists in tableschema \"%2\".").arg(key).arg(m_schema->name());
            return false;
        }

        QVariant v(value);
        if (! v.convert(field->variantType())) {
            qWarning() << QString("Invalid WHERE-expression: The for Field \"%1\" defined value is of type \"%2\" rather then the expected type \"%3\"").arg(key).arg(v.typeName()).arg(field->variantType());
            return false;
        }

        QString errorMessage;
        QString errorDescription;
        if (!static_cast<KDbQuerySchema *>(m_schema)->addToWhereExpression(field, v, KDbToken('='), &errorMessage, &errorDescription))
        {
            qWarning() << "addToWhereExpression() failed, message=" << errorMessage
                       << "description=" << errorDescription;
            return false;
        }
    }
    return true;
}

