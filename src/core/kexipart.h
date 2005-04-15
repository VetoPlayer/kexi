/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2003-2005 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KEXIPART_H
#define KEXIPART_H

#include <qobject.h>
#include <qmap.h>

#include "kexi.h"
#include "tristate.h"
#include "keximainwindow.h"

class KActionCollection;
class KexiDialogBase;
class KexiDialogTempData;
class KexiViewBase;
class KexiMainWindowImpl;
class KAction;
class KShortcut;

namespace KexiPart
{
	class Info;
	class Item;
	class GUIClient;
	class DataSource;
	class PartPrivate;

/*! Official (registered) type IDs for objects like table, query, form... */
enum ObjectTypes {
	TableObjectType = 1, //like in KexiDB::ObjectTypes
	QueryObjectType = 2, //like in KexiDB::ObjectTypes
	FormObjectType = 3,
	ReportObjectType = 4,
	LastObjectType = 4, //ALWAYS UPDATE THIS

	UserObjectType = 100 //external types
};

/**
 * The main class for kexi frontend parts like tables, queries, relations
 */
class KEXICORE_EXPORT Part : public QObject
{
	Q_OBJECT

	public:
		Part(QObject *parent, const char *name, const QStringList &);
		virtual ~Part();

		/*! \return supported modes for dialogs created by this part, ie. a combination
		 of Kexi::ViewMode enum elements.
		 Set this member in your KexiPart subclass' ctor, if you need to override the default value
		 that equals Kexi::DataViewMode | Kexi::DesignViewMode.
		 This information is used to set supported view modes for every 
		 KexiDialogBase derived object created by this KexiPart.
		 Default flag combination is Kexi::DataViewMode | Kexi::DesignViewMode. */
		inline int supportedViewModes() const { return m_supportedViewModes; }

		/*! "Opens" an instance that the part provides, pointed by \a item in a mode \a viewMode. 
		 \a viewMode is one of Kexi::ViewMode enum. */
		KexiDialogBase* openInstance(KexiMainWindow *win, KexiPart::Item &item, 
			int viewMode = Kexi::DataViewMode);

		/*! Removes any stored data pointed by \a item (example: table is dropped for table part). 
		 From now this data is inaccesible, and \a item disappear.
		 You do not need to remove \a item, or remove object's schema stored in the database,
		 beacuse this will be done automatically by KexiProject after successful 
		 call of this method. All object's data blocks are also automatically removed from database 
		 (from "kexi__objectdata" table).
		 For this, a database connection associated with kexi project owned by \a win can be used.

		 Database transaction is started by KexiProject before calling this method, 
		 and it will be rolled back if you return false here.
		 You shouldn't use by hand transactions here.
		 
		 Default implementation just removes object from kexi__* system structures 
		 at the database backend using KexiDB::Connection::removeObject(). */
		virtual bool remove(KexiMainWindow *win, KexiPart::Item & item);

		/*! Renames stored data pointed by \a item to \a newName 
		 (example: table name is altered in the database). 
		 For this, a database connection associated with kexi project owned by \a win can be used. 
		 You do not need to change \a item, and change object's schema stored in the database,
		 beacuse this is automatically handled by KexiProject. 

		 Database transaction is started by KexiProject before calling this method, 
		 and it will be rolled back if you return false here.
		 You shouldn't use by hand transactions here.

		 Default implementation does nothing and returns true. */
		virtual tristate rename(KexiMainWindow * /*win*/, KexiPart::Item & /*item*/, 
			const QString& /*newName*/)
			{ return true; }

		/*! Creates and returns a new temporary data for a dialog \a dialog. 
		 This method is called on openInstance() once per dialog.
		 Reimplement this to return KexiDialogTempData subclass instance.
		 Default implemention just returns empty KexiDialogTempData object. */
		virtual KexiDialogTempData* createTempData(KexiDialogBase* dialog);

		/*! Creates a new view for mode \a viewMode, \a item and \a parent. The view will be 
		 used inside \a dialog. */
		virtual KexiViewBase* createView(QWidget *parent, KexiDialogBase* dialog, 
			KexiPart::Item &item, int viewMode = Kexi::DataViewMode) = 0;

		/*! i18n'd instance name usable for displaying in gui.
		 @todo move this to Info class when the name could be moved as localized property 
		 to service's .desktop file. */
		inline QString instanceName() const { return m_names["instance"]; }
		
		inline Info *info() const { return m_info; }

		/*! \return part's GUI Client, so you can 
		 create part-wide actions using this client. */
		inline GUIClient *guiClient() const { return m_guiClient; }

		/*! \return part's GUI Client, so you can 
		 create instance-wide actions using this client. */
		inline GUIClient *instanceGuiClient(int mode = 0) const
			{ return m_instanceGuiClients[mode]; }

		/**
		 * @returns the datasource object of this part
		 * reeimplement it to make a part work as dataprovider ;)
		 */
		virtual DataSource *dataSource() { return 0; }

		KActionCollection* actionCollectionForMode(int viewMode) const;

		const Kexi::ObjectStatus& lastOperationStatus() const { return m_status; }

		/*! \return i18n'd message translated from \a englishMessage.
		 This method is useful for messages like: 
		 "<p>Table \"%1\" has been modified.</p><p>Do you want to save changes?</p>",
		 -- first part of messages like that could not be accurately translated.
		 See implementation of this method in KexiTablePart for example how to implement it. 
		 Default implementation returns \a englishMessage. */
		virtual QString i18nMessage(const QCString& englishMessage) const;

	signals: 
		void newObjectRequest( KexiPart::Info *info );

	protected slots:
		void slotCreate();

	protected:
//		virtual KexiDialogBase* createInstance(KexiMainWindow *win, const KexiPart::Item &item, int viewMode = Kexi::DataViewMode) = 0;

		//! Creates GUICLients for this part, attached to \a win
		//! This method is called from KexiMainWindow
		void createGUIClients(KexiMainWindow *win);

#if 0
		/*! For reimplementation. Create here all part actions (KAction or similar). 
		 "Part action" is an action that is bound to given part, not for dialogs 
		 created with this part, eg. "Open external project" action for Form part.
		 Default implementation does nothing.
		*/
		virtual void initPartActions( KActionCollection * ) {};

		/*! For reimplementation. You should here create all instance actions (KAction or similar)
		 for \a mode (this method called for every value given by Kexi::ViewMode enum, 
		 and in special cases, in the future - for user-defined part-specific modes). 
		 Actions should be bound to action collection \a col. 
		 "Instance action" is an action that is bound to given dialog instance (created with a part), 
		 for specific view. \a mo; eg. "Filter data" action for DataViewMode of Table part. 
		 By creating actions here, you can ensure that after switching to other view mode (eg. from
		 Design view to Data view), appropriate actions will be switched/hidden.
		 \a mode equal Kexi::AllViewModes means that given actions will be available for 
		 all supported views.
		 Default implementation does nothing.
		*/
		virtual void initInstanceActions( int mode, KActionCollection *col ) {};
#endif

		virtual void initPartActions();
		virtual void initInstanceActions();

		virtual KexiDB::SchemaData* loadSchemaData(KexiDialogBase *dlg, const KexiDB::SchemaData& sdata, int viewMode);

		bool loadDataBlock( KexiDialogBase *dlg, QString &dataString, const QString& dataID = QString::null);

		/*! Creates shared action for action collection declared 
		 for 'instance actions' of this part.
		 See KexiSharedActionHost::createSharedAction() for details.
		 Pass desired KAction subclass with \a subclassName (e.g. "KToggleAction") to have
		 that subclass allocated instead just KAction (what is the default). */
		KAction* createSharedAction(int mode, const QString &text, 
			const QString &pix_name, const KShortcut &cut, const char *name, 
			const char *subclassName = 0);

		/*! Convenience version of above method - creates shared toggle action. */
		KAction* createSharedToggleAction(int mode, const QString &text,
			const QString &pix_name, const KShortcut &cut, const char *name);

		/*! Creates shared action for action collection declared 
		 for 'part actions' of this part.
		 See KexiSharedActionHost::createSharedAction() for details.
		 Pass desired KAction subclass with \a subclassName (e.g. "KToggleAction") to have
		 that subclass allocated instead just KAction (what is the default). */
		KAction* createSharedPartAction(const QString &text, 
			const QString &pix_name, const KShortcut &cut, const char *name,
			const char *subclassName = 0);

		/*! Convenience version of above method - creates shared toggle action 
		 for 'part actions' of this part. */
		KAction* createSharedPartToggleAction(const QString &text,
			const QString &pix_name, const KShortcut &cut, const char *name);

		void setActionAvailable(const char *action_name, bool avail);

		inline void setInfo(Info *info) { m_info = info; }

		//! Set of i18n'd action names for, initialised on KexiPart::Part subclass ctor
		//! The names are useful because the same action can have other name for each part
		//! E.g. "New table" vs "New query" can have different forms for some languages...
		QMap<QString,QString> m_names;

		/*! Supported modes for dialogs created by this part.
		 @see supportedViewModes() */
		int m_supportedViewModes;

		Info *m_info;
		GUIClient *m_guiClient;
//		GUIClient *m_instanceGuiClient;
		QIntDict<GUIClient> m_instanceGuiClients;
		KexiMainWindow* m_mainWin;

		Kexi::ObjectStatus m_status;

		PartPrivate *d;

	friend class Manager;
	friend class ::KexiMainWindow;
	friend class ::KexiMainWindowImpl;
	friend class GUIClient;
};

}

#endif

