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

#ifndef KEXIACTIONPROXY_H
#define KEXIACTIONPROXY_H

#include <qguardedptr.h>
#include <qasciidict.h>
#include <qobject.h>
#include <qpair.h>

class KexiMainWindow;
class QSignal;
class KAction;

//! Acts as proxy for shared actions within the application.
/*!
 For example, edit->copy action can be reused to copy different types of items.
 Availability and meaning of given action depends on the context, while
 the context changes e.g. when another window is activated.
 This class is mostly used by subclassing in KExiDialogBase or KexiDockBase
 - you can subclass in a similar way.
*/
class KEXICORE_EXPORT KexiActionProxy
{
	public:
		KexiActionProxy(KexiMainWindow *main, QObject *receiver);
		~KexiActionProxy();

		void activateSharedAction(const char *action_name);

	protected:
		/*! Plugs shared action named \a action_name to slot \a slot in \a receiver.
		 \a Receiver is usually a child of _this_ widget. */
		void plugSharedAction(const char *action_name, QObject* receiver, const char *slot);

		/*! Typical version of plugAction() method -- plugs action named \a action_name
		 to slot \a slot in _this_ widget. */
		inline void plugSharedAction(const char *action_name, const char *slot) {
			plugSharedAction(action_name, m_receiver, slot);
		}

		/*! Plugs action named \a action_name to a widget \a w, so the action is visible on this widget 
		 as an item. \a w will typically be a menu, popup menu or a toolbar. 
		 Does nothing if no action found, so generally this is safer than just caling e.g.
		 <code> action("myaction")->plug(myPopup); </code> 
		 \sa action(), KAction::plug(QWidget*, int) */
		void plugSharedAction(const char *action_name, QWidget* w);

		/*! \return action named with \a name or NULL if there is no such action. */
		KAction* sharedAction(const char* name);

		inline QObject *receiver() const { return m_receiver; }

		bool isAvailable(const char* action_name);
		void setAvailable(const char* action_name, bool set);

		QGuardedPtr<KexiMainWindow> m_main;
		QGuardedPtr<QObject> m_receiver;
		QAsciiDict< QPair<QSignal*,bool> > m_signals;

		QObject *m_signal_parent; //!< it's just to have common parent for owned signals
	friend class KexiMainWindow;
};

#endif

