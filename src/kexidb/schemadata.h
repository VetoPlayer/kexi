/* This file is part of the KDE project
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

#ifndef KEXIDB_SCHEMADATA_H
#define KEXIDB_SCHEMADATA_H

#include <qvaluelist.h>
#include <qstring.h>

#include <kexidb/global.h>
#include <kexidb/field.h>

namespace KexiDB {

/*! Container class that stores common kexi object schema's properties like
 id, name, caption, help text.
 By kexi object we mean in-db storable object like table schema or query schema.
*/

class KEXI_DB_EXPORT SchemaData
{
	public:
		const int type() const;
		const int id() const;
		const QString& name() const;
		void setName(const QString& n) { m_name=n; }
		const QString& caption() const;
		void setCaption(const QString& c) { m_caption=c; }
		const QString& helpText() const;
		void setHelpText(const QString& ht) { m_helpText=ht; }

		//! \return debug string useful for debugging
		virtual QString schemaDataDebugString() const;
		
		/*! \return true if this is schema of native database object,
		 like, for example like, native table. This flag 
		 is set when object schema (currently -- database table) 
		 is not retrieved using kexi__* schema storage system,
		 but just based on the information about native table.
		 
		 By native object we mean the one that has no additional
		 data like caption, helpText, etc. properties (no kexidb extensions).
		 
		 Native objects schemas are used mostly for representing 
		 kexi system (kexi__*) tables in memory for later reference;
		 see Connection::tableNames().
		 
		 By default (on allocation) SchemaData objects are not native.
		*/
		virtual bool isNative() const { return m_native; }
		
		/* Sets native flag */
		virtual void setNative(bool set) { m_native=set; }
	protected:
		SchemaData(int obj_type = KexiDB::UnknownObjectType);
		virtual ~SchemaData();

		//! Clears all properties excpet 'type'.
		void clear();

	//js	QStringList m_primaryKeys;
		int m_type;
		int m_id;
		QString m_name;
		QString m_caption;
		QString m_helpText;
		bool m_native : 1;

	friend class Connection;
};

} //namespace KexiDB

#endif
