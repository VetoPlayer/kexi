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

#ifndef KEXIDB_OBJECT_H
#define KEXIDB_OBJECT_H

#include <kexidb/error.h>

#include <qstring.h>

namespace KexiDB {

/*!

*/
class KEXI_DB_EXPORT Object
{
	public:
		//! \return true if there was error during last operation on the object.
		bool error() { return m_hasError; }

		/*! \return (localized) error message if there was error during last operation on the object,
			else: 0. */
		const QString& errorMsg() { return m_errMsg; }

		/*! \return error number of if there was error during last operation on the object,
			else: 0. */
		int errorNum() { return m_errno; }

		/*! Sends errorMsg() to debug output. */
		void debugError();

		/*! KexiDB library offers detailed error numbers using errorNum()
		 and detailed error i18n'd messages using errorMsg() -
		 these informations are not engine-dependent (almost).
		 Use this in your application to give users more information on what's up.
		 
		 This method returns (non-i18n'd !) engine-specific error message,
		 if there was any error during last server-side operation,
		 otherwise null string. 
		 Reimplement this for your driver 
		 - default implementation just returns null string.
		 \sa serverErrorMsg()
		*/
		virtual QString serverErrorMsg();

		/*! \return engine-specific last server-side operation result number.
		 Use this in your application to give users more information on what's up.
		 
		 Reimplement this for your driver - default implementation just returns 0.
		 Note that this error value is not the same as the one returned 
		 by errorNum() (Object::m_errno member)
		 \sa serverErrorMsg(), drv_clearServerResult()
		*/
		virtual int serverResult();

		/*! \return engine-specific last server-side operation result name,
		 (name for serverResult()).
		 Use this in your application to give users more information on what's up.
		 
		 Reimplement this for your driver - default implementation 
		 just returns null string.
		 Note that this result name is not the same as the error message returned 
		 by serverErorMsg() or erorMsg()
		 \sa serverErrorMsg(), drv_clearServerResult()
		*/
		virtual QString serverResultName();
		
	protected:
		Object();
		virtual ~Object();
		
		/*! Sets the (localized) error code to \a code and message to \a msg. 
		 You have to set at least nonzero error code \a code, 
		 although it is also adviced to set descriptive message \a msg.
		 Eventually, if you omit all parameters, ERR_OTHER code will be set 
		 and default message for this will be set.
		 Use this in KexiDB::Object subclasses to informa the world about your 
		 object's state. */
		void setError(int code = ERR_OTHER,  const QString &msg = QString::null );

		/* \overload void setError(int code,  const QString &msg = QString::null )
		
			Sets error code to ERR_OTHER. Use this if you don't care about 
			setting error code.
		*/
		void setError( const QString &msg );
		
		/*! Copies the (localized) error message and code from other KexiDB::Object. */
		void setError( KexiDB::Object *obj );

		/*! Clears error flag. 
		 Also calls drv_clearServerResult().
		 You can reimplement this method in subclasses to clear even more members,
		 but remember to also call Object::clearError(). */
		virtual void clearError();

		/*! Clears number of last server operation's result stored
		 as a single integer. Formally, this integer should be set to value 
		 that means "NO ERRORS" or "OK". This method is called by clearError().
		 For reimplementation. By default does nothing.
		 \sa serverErrorMsg()
		*/
		virtual void drv_clearServerResult() {};

	private:
		int m_errno;
		bool m_hasError;
		QString m_errMsg;

		class Private;
		Private *d_object; //for future extension
};

} //namespace KexiDB

#endif


