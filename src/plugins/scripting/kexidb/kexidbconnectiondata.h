/***************************************************************************
 * kexidbconnectiondata.h
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

#ifndef SCRIPTING_KEXIDBCONNECTIONDATA_H
#define SCRIPTING_KEXIDBCONNECTIONDATA_H

#include <QString>
#include <QObject>

#include <KDbConnection>
#include <KDbConnectionData>

namespace Scripting
{

/**
 * A KexiDBConnectionData is used to store the details needed for
 * a connection with a database.
 */
class KexiDBConnectionData : public QObject
{
    Q_OBJECT
public:
    KexiDBConnectionData();
    KexiDBConnectionData(QObject* parent, KDbConnectionData* data);
    virtual ~KexiDBConnectionData();
    KDbConnectionData* data() {
        return m_data;
    }

public Q_SLOTS:

    /** Return the connection name. */
    const QString caption() const;
    /** Set the connection name. */
    void setCaption(const QString& name);

    /** Return the description. */
    const QString description() const;
    /** Set the description. */
    void setDescription(const QString& desc);

    /** Return drivername. */
    const QString driverName() const;
    /** Set the drivername. */
    void setDriverName(const QString& driver);

    /** Return true if a local socket file is used else false. */
    bool localSocketFileUsed() const;
    /** Set if the local socket file should be used. */
    void setLocalSocketFileUsed(bool used);
    /** Return the local socket filename. */
    const QString localSocketFileName() const;
    /** Set the local socket filename. */
    void setLocalSocketFileName(const QString& socketfilename);

    // For serverbased drivers

    /** Return the database name. */
    const QString databaseName() const;
    /** Set the database name. */
    void setDatabaseName(const QString& dbname);

    /** Return the hostname. */
    const QString hostName() const;
    /** Set the hostname. */
    void setHostName(const QString& hostname);

    /** Return the port number. */
    int port() const;
    /** Set the port number. */
    void setPort(int p);

    /** Return the password. */
    const QString password() const;
    /** Set the password. */
    void setPassword(const QString& passwd);

    /** Return the username. */
    const QString userName() const;
    /** Set the username. */
    void setUserName(const QString& username);
    
    /** Return a user-friendly string representation. */
    const QString toUserVisibleString() const;

private:
    KDbConnectionData *m_data = nullptr;
};

}

#endif

