/* This file is part of the KDE project
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kexidb/table.h>
#include <kexidb/driver.h>
#include <kexidb/connection.h>

#include <assert.h>

#include <kdebug.h>

using namespace KexiDB;

Table::Table(const QString& name)
	: FieldList(name)
	, m_id(0)
	, m_conn(0)
{
	m_indices.setAutoDelete( true );
}

Table::Table()
	: FieldList()
	, m_id(0)
	, m_conn(0)
{
}

// used by Conenction
Table::Table(const QString & name, Connection *conn)
	: FieldList(name)
	, m_conn( conn )
{
	assert(conn);
}

Table::~Table()
{
}

void Table::addField(KexiDB::Field* field)
{
	FieldList::addField(field);
	//Check for auto-generated indices:

	// this field auto-generated unique indice
	if (field->isUniqueKey()) {
		Index *idx = new Index();
		idx->addField( field );
		idx->setAutoGenerated(true);
		m_indices.append(idx);
	}
}

void Table::clear()
{
	m_indices.clear();
	FieldList::clear();
	m_conn = 0;
}

/*
void Table::addPrimaryKey(const QString& key)
{
	m_primaryKeys.append(key);
}*/

/*QStringList Table::primaryKeys() const
{
	return m_primaryKeys;
}

bool Table::hasPrimaryKeys() const
{
	return !m_primaryKeys.isEmpty();
}
*/

//const QString& Table::name() const
//{
//	return m_name;
//}

//void Table::setName(const QString& name)
//{
//	m_name=name;
/*	ListIterator it( m_fields );
	Field *field;
	for (; (field = it.current())!=0; ++it) {
	
	int fcnt=m_fields.count();
	for (int i=0;i<fcnt;i++) {
		m_fields[i].setTable(name);
	}*/
//}

/*KexiDB::Field Table::field(unsigned int id) const
{
	if (id<m_fields.count()) return m_fields[id];
	return KexiDB::Field();
}

unsigned int Table::fieldCount() const
{
	return m_fields.count();
}*/

Connection* Table::connection()
{
	return m_conn;
}

void Table::debug()
{
	kdDebug() << "TABLE " << m_name << endl;
	FieldList::debug();

/*	QString dbg = "TABLE " + m_name + "\n";
	Field::ListIterator it( m_fields );
	Field *field;
	bool start = true;
	for (; (field = it.current())!=0; ++it) {
		if (!start)
			dbg += ",\n";
		else
			start = false;
		dbg += "  ";
		dbg += field->debugString();
	}
	kdDebug() << dbg << endl;*/
}

//----------------------------------------------------

/*
TableDef::TableDef(const QString& name)
	: Table(name)
{
}

TableDef::TableDef()
	: Table()
{
}

TableDef::~TableDef()
{
}

KexiDB::FieldDef
TableDef::field(unsigned int id) const
{
	if (id<m_fields.count()) return m_fields[id];
	return KexiDB::FieldDef();
}

void TableDef::addField(KexiDB::FieldDef field)
{
	field.setTable(m_name);
	m_fields.append(field);
}
*/

