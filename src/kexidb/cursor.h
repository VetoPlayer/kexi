/* This file is part of the KDE project
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KEXIDB_CURSOR_H
#define KEXIDB_CURSOR_H

#include <qstring.h>
#include <qvariant.h>
#include <qptrvector.h>

#include <kexidb/connection.h>
#include <kexidb/object.h>

namespace KexiDB {

//class CursorData;

/*! Provides database cursor functionality.

	Cursor can be defined in two ways:

	-# by passing QuerySchema object to Connection::executeQuery() or Connction::prepareQuery();
	   then query is defined for in engine-independent way -- this is recommended usage

	-# by passing raw query statement string to Connection::executeQuery() or Connction::prepareQuery();
	   then query may be defined for in engine-dependent way -- this is not recommended usage,
	   but convenient when we can't or do not want to allocate QuerySchema object, while we
	   know that the query statement is syntactically and logically ok in our context.

	You can move cursor to next record with moveNext() and move back with movePrev().
	The cursor is always positioned on record, not between records, with exception that 
	ofter open() it is positioned before first record (if any) -- then bof() equals true,
	and can be positioned after the last record (if any) with moveNext() -- then eof() equals true,
	For example, if you have four records 1, 2, 3, 4, then after calling moveNext(), 
	moveNext(), moveNext(), movePrev() you are going through records: 1, 2, 3, 2.

	Cursor can be buffered or unbuferred.
	Buffering in this class is not related to any SQL engine capatibilities for server-side cursors 
	(eg. like 'DECLARE CURSOR' statement) - buffered data is at client (application) side.
	Any record retrieved in buffered cursor will be stored inside an internal buffer
	and reused when needed. Unbuffered cursor always requires one record fetching from
	db connection at every step done with moveNext(), movePrev(), etc.

	Notes:
	- Do not use delete operator for Cursor objects - this will fail; use Connection::deleteCursor()
	instead.
	- QuerySchema object is not owned by Cursor object that uses it.
*/
class KEXI_DB_EXPORT Cursor: public Object
{
	public:
		//! Cursor options that describes its behaviour
		enum Options {
			NoOptions = 0,
			Buffered = 1
		};

		virtual ~Cursor();

		/*! \return connection used for the cursor */
		Connection* connection() { return m_conn; }

		/*! Opens the cursor using data provided on creation. 
		 The data might be either QuerySchema or raw sql statement. */
		bool open();

		/*! Closes and then opens again the same cursor. 
		 If the cursor is not opened it is just opened and result of this open is returned.
		 Otherwise, true is returned if cursor is successfully closed and then opened. */
		bool reopen();

//		/*! Opens the cursor using \a statement. 
//		 Omit \a statement if cursor is already initialized with statement 
//		 at creation time. If \a statement is not empty, existing statement
//		 (if any) is overwritten. */
//		bool open( const QString& statement = QString::null );

		/*! Closes previously opened cursor. 
			If the cursor is closed, nothing happens. */
		virtual bool close();

		/*! \return query schema used to define this cursor
		 or NULL if query schema is undefined but raw statement instead. */
		QuerySchema *query() const { return m_query; }

		/*! \return raw query statement used to define this cursor
		 or null string if raw statement instead (but QuerySchema is defined instead). */
		QString rawStatement() const { return m_rawStatement; }

		/*! \return logically or'd cursor's options, 
			selected from Cursor::Options enum. */
		uint options() const { return m_options; }

		/*! \returns true if the cursor is opened. */
		bool isOpened() const { return m_opened; }

		/*! \returns true if the cursor is buffered. */
		bool isBuffered() const;

		/*! Sets this cursor to buffered type or not. See description 
			of buffered and nonbuffered cursors in class description.
			This method only works if cursor is not opened (isOpened()==false).
			You can close already opened cursor and then switch this option on/off.
		*/
		void setBuffered(bool buffered);

		/*! Moves current position to the first record and retrieves it.
			\return true if the first record was retrieved.
			False could mean that there was an error or there is no record available. */
		bool moveFirst();

		/*! Moves current position to the last record and retrieves it. 
			\return true if the last record was retrieved.
			False could mean that there was an error or there is no record available. */
		virtual bool moveLast();

		/*! Moves current position to the next record and retrieves it. */
		virtual bool moveNext();

		/*! Moves current position to the next record and retrieves it. */
		virtual bool movePrev();

		/*! \return true if current position is after last record. */
		bool eof() const;

		/*! \return true if current position is before first record. */
		bool bof() const;

		/*! \return current internal position of the cursor's query. 
		 We are counting records from 0.
		 Value -1 means that cursor does not point to any valid record
		 (this happens eg. after open(), close(), 
		 and after moving after last record or before first one. */
		Q_LLONG at() const;

		/*! \return number of fields available for this cursor. */
		uint fieldCount() const { return m_fieldCount; }

		virtual QVariant value(int i) const = 0;

		/*! [PROTOTYPE] \return current record data or NULL if there is no current records. */
		virtual const char ** recordData() const = 0;
		
		/*! Puts current record's data into \a data (makes a deep copy).
		 This have unspecified behaviour if the cursor is not at valid record.
		 Note: For reimplementation in driver's code. */
		virtual void storeCurrentRecord(RecordData &data) const = 0;

		void debug();

	protected:
		/*! Cursor will operate on \a conn, raw \a statement will be used to execute query. */
		Cursor(Connection* conn, const QString& statement, uint options = NoOptions );

		/*! Cursor will operate on \a conn, \a query schema will be used to execute query. */
		Cursor(Connection* conn, QuerySchema& query, uint options = NoOptions );

		void init();

		/* Note for driver developers: this method should initialize engine-specific cursor's
		 resources using \a statement. It is not required to store \a statement somewhere
		 in your Cursor subclass (it is already stored in m_query or m_rawStatement, 
		 depending query type) - only pass it to proper engine's function. */
		virtual bool drv_open(const QString& statement) = 0;

		virtual bool drv_close() = 0;
//		virtual bool drv_moveFirst() = 0;
		virtual bool drv_getNextRecord() = 0;
		virtual bool drv_getPrevRecord() = 0;

		/*DISABLED: ! This is called only once in open(), after successful drv_open().
			Reimplement this if you need (or not) to do get the first record after drv_open(),
			eg. to know if there are any records in table. Value returned by this method
			will be assigned to m_readAhead.
			Default implementation just calls drv_getNextRecord(). */

		/*! Clears cursor's buffer if this was allocated (only for buffered cursor type).
			Otherwise do nothing. For reimplementing. Default implementation does nothing. */
		virtual void drv_clearBuffer() {}

		//! Internal: clears buffer with reimplemented drv_clearBuffer(). */
		void clearBuffer();

		Connection *m_conn;
		QuerySchema *m_query;
//		CursorData *m_data;
		QString m_rawStatement;
		bool m_opened : 1;
		bool m_beforeFirst : 1;
		bool m_atLast : 1;
		bool m_afterLast : 1;
//		bool m_atLast;
		bool m_validRecord : 1; //! true if valid record is currently retrieved @ current position
		bool m_readAhead : 1;
		Q_LLONG m_at;
		int m_fieldCount; //! cached field count information
		uint m_options; //! cursor options that describes its behaviour

		//<members related to buffering>
		uint m_cols_pointers_mem_size; //! size of record's array of pointers to values
		int m_records_in_buf;          //! number of records currently stored in the buffer
		QPtrVector<const char*> m_records;//! buffer data
		bool m_buffering_completed : 1;   //! true if we already have all records stored in the buffer
		bool m_at_buffer : 1;             //! true if we already point to the buffer with curr_coldata
		//</members related to buffering>
	
	private:
		class Private;
		Private *d;
};

} //namespace KexiDB

#endif


