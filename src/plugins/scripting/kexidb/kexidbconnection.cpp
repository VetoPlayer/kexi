/***************************************************************************
 * kexidbconnection.cpp
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

#include "kexidbconnection.h"
#include "kexidbconnectiondata.h"
#include "kexidbdriver.h"
#include "kexidbcursor.h"
#include "kexidbfieldlist.h"
#include "kexidbschema.h"
#include "kexidbparser.h"
#include "KexiScriptingDebug.h"

#include <KDbTransaction>
#include <KDbConnectionOptions>

#include <QDebug>

using namespace Scripting;

KexiDBConnection::KexiDBConnection(KDbConnection* connection, KexiDBConnectionData* connectiondata, KexiDBDriver* driver)
        : QObject()
        , m_connection(connection)
        , m_driver(driver ? driver : new KexiDBDriver(this, connection->driver()))
{
    m_connectiondata = connectiondata;
    setObjectName("KexiDBConnection");
}

KexiDBConnection::KexiDBConnection(KDbConnection* connection, KexiDBDriver* driver)
        : QObject()
        , m_connection(connection)
        , m_driver(driver ? driver : new KexiDBDriver(this, connection->driver()))
{
    setObjectName("KexiDBConnection");
}

KexiDBConnection::~KexiDBConnection()
{
}

bool KexiDBConnection::hadError() const
{
    return m_connection->result().isError();
}
QString KexiDBConnection::lastError() const
{
    return m_connection->result().message();
}

QObject* KexiDBConnection::data()
{
    return m_connectiondata;
}
QObject* KexiDBConnection::driver()
{
    return m_driver;
}

bool KexiDBConnection::connect()
{
    return m_connection->connect();
}
bool KexiDBConnection::isConnected()
{
    return m_connection->isConnected();
}
bool KexiDBConnection::disconnect()
{
    return m_connection->disconnect();
}

bool KexiDBConnection::isReadOnly() const
{
    return m_connection->options()->isReadOnly();
}

bool KexiDBConnection::databaseExists(const QString& dbname)
{
    return m_connection->databaseExists(dbname);
}
QString KexiDBConnection::currentDatabase() const
{
    return m_connection->currentDatabase();
}
const QStringList KexiDBConnection::databaseNames() const
{
    return m_connection->databaseNames();
}
bool KexiDBConnection::isDatabaseUsed() const
{
    return m_connection->isDatabaseUsed();
}
bool KexiDBConnection::useDatabase(const QString& dbname)
{
    return m_connection->databaseExists(dbname) && m_connection->useDatabase(dbname);
}
bool KexiDBConnection::closeDatabase()
{
    return m_connection->closeDatabase();
}

const QStringList KexiDBConnection::allTableNames() const
{
    return m_connection->tableNames(true);
}
const QStringList KexiDBConnection::tableNames() const
{
    return m_connection->tableNames(false);
}

const QStringList KexiDBConnection::queryNames() const
{
    bool ok = true;
    QStringList queries = m_connection->objectNames(KDb::QueryObjectType, &ok);
    if (! ok) {
        KexiScriptingWarning() << "Failed to determinate query names.";
        return QStringList();
    }
    return queries;
}

QObject* KexiDBConnection::executeQueryString(const QString& sqlquery)
{
    // The KDbConnection::executeQuery() method does not check if we pass a valid SELECT-statement
    // or e.g. a DROP TABLE operation. So, let's check for such dangerous operations right now.
    KDbParser parser(m_connection);
    if (! parser.parse(KDbEscapedString(sqlquery))) {
        KexiScriptingWarning() << "Failed to parse query: "<< parser.error().type() << parser.error().message();
        return 0;
    }
    if (parser.query() == 0 || parser.statementType() != KDbParser::Select) {
        KexiScriptingWarning() << "Invalid query operation " << parser.statementTypeString();
        return 0;
    }
    KDbCursor* cursor = m_connection->executeQuery(KDbEscapedString(sqlquery));
    return cursor ? new KexiDBCursor(this, cursor, true) : 0;
}

QObject* KexiDBConnection::executeQuerySchema(KexiDBQuerySchema* queryschema)
{
    KDbCursor* cursor = m_connection->executeQuery(queryschema->queryschema());
    return cursor ? new KexiDBCursor(this, cursor, true) : 0;
}

bool KexiDBConnection::insertRecord(QObject* obj, const QVariantList& values)
{
    KexiDBFieldList* fieldlist = dynamic_cast< KexiDBFieldList* >(obj);
    if (fieldlist)
        return m_connection->insertRecord(fieldlist->fieldlist(), values);
    KexiDBTableSchema* tableschema = dynamic_cast< KexiDBTableSchema* >(obj);
    if (tableschema)
        return m_connection->insertRecord(tableschema->tableschema(), values);
    return false;
}

bool KexiDBConnection::createDatabase(const QString& dbname)
{
    return m_connection->createDatabase(dbname);
}
bool KexiDBConnection::dropDatabase(const QString& dbname)
{
    return m_connection->dropDatabase(dbname);
}

bool KexiDBConnection::createTable(KexiDBTableSchema* tableschema)
{
    return m_connection->createTable(tableschema->tableschema());
}
bool KexiDBConnection::dropTable(const QString& tablename)
{
    return true == m_connection->dropTable(tablename);
}
bool KexiDBConnection::alterTable(KexiDBTableSchema* fromschema, KexiDBTableSchema* toschema)
{
    return true == m_connection->alterTable(fromschema->tableschema(), toschema->tableschema());
}
bool KexiDBConnection::alterTableName(KexiDBTableSchema* tableschema, const QString& newtablename)
{
    return m_connection->alterTableName(tableschema->tableschema(), newtablename);
}

QObject* KexiDBConnection::tableSchema(const QString& tablename)
{
    KDbTableSchema* tableschema = m_connection->tableSchema(tablename);
    return tableschema ? new KexiDBTableSchema(this, tableschema, false) : 0;
}

bool KexiDBConnection::isEmptyTable(KexiDBTableSchema* tableschema) const
{
    return m_connection->isEmpty(tableschema->tableschema()) == true;
}

QObject* KexiDBConnection::querySchema(const QString& queryname)
{
    KDbQuerySchema* queryschema = m_connection->querySchema(queryname);
    return queryschema ? new KexiDBQuerySchema(this, queryschema, false) : 0;
}

bool KexiDBConnection::autoCommit() const
{
    return m_connection->autoCommit();
}
bool KexiDBConnection::setAutoCommit(bool enabled)
{
    return m_connection->setAutoCommit(enabled);
}

QObject* KexiDBConnection::parser()
{
    return new KexiDBParser(this, new KDbParser(m_connection), true);
}

