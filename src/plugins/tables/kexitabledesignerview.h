/* This file is part of the KDE project
   Copyright (C) 2004-2006 Jaroslaw Staniek <js@iidea.pl>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIALTERTABLEDIALOG_H
#define KEXIALTERTABLEDIALOG_H

#include <koproperty/property.h>
#include <kexidb/alter.h>

#include <kexidatatable.h>
#include "kexitablepart.h"

class KexiTableItem;
class KexiTableDesignerViewPrivate;
class KCommand;
class CommandGroup;

namespace KoProperty {
	class Set;
}

//! Design view of the Table Designer
/*! Contains a spreadsheet-like space for entering field definitions.
 Property editor is provided for altering field definitions.
 
 The view also supports Undo and Redo operations.
 These are connected to a factility creating a list of actions used 
 by AlterTableHandler to perform required operation of altering the table.

 Altering itself is performed upon design saving (storeData()). 
 Saving unstored designs just creates a new table.
 Saving changes made to empty (not filled with data) table is performed 
 by physically deleting the previous table schema and recreating it
 TODO: this will be not quite when we have db relationships supported.

 Saving changes made to table containing data requires use of the AlterTableHandler
 functionality.
*/
class KexiTableDesignerView : public KexiDataTable
{
	Q_OBJECT

	public:
		/*! Creates a new alter table dialog. */
		KexiTableDesignerView(KexiMainWindow *win, QWidget *parent);

		virtual ~KexiTableDesignerView();

		KexiTablePart::TempData* tempData() const;

		/*! Clears field informaton entered for row. 
		 This is performed by removing values from caption and data type columns.
		 Used by InsertFieldCommand to undo inserting a new field. */
		void clearRow(int row);

		/*! Inserts a new \a field for \a row. 
		 Property set is also created. If \a set is not 0 (the default), 
		 it will be copied into the new set.
		 Used by InsertFieldCommand to insert a new field. */
		void insertField( int row/*, const KexiDB::Field& field*/, KoProperty::Set& set);

		/*! Inserts a new empty row at position \a row. 
		 Used by RemoveFieldCommand as a part of undo inserting a new field;
		 also used by InsertEmptyRowCommand. */
		void insertEmptyRow( int row );

		/*! Deleted \a row from the table view; used for undoing InsertEmptyRowCommand. */
		void deleteRow( int row );

		/*! Inserts a field for \a row. All the subsequent fields are moved up.
		 Property set is also deleted.
		 Used by RemoveFieldCommand to remove a field. */
		void deleteField( int row );

		/*! Changes property \a propertyName to \a newValue for a field pointed by \a fieldUID.
		 If \a listData is not NULL and not empty, a deep copy of it is passed to Property::setListData().
		 If \a listData \a nlist if not NULL but empty, Property::setListData(0) is called.
		 Used by ChangeFieldPropertyCommand to change field's property. */
		void changeFieldProperty( int fieldUID, const QCString& propertyName, 
			const QVariant& newValue, KoProperty::Property::ListData* const listData = 0 );

		/*! Changes visibility of property \a propertyName to \a visible for a field pointed by \a fieldUID.
		 Used by ChangePropertyVisibilityCommand. */
		void changePropertyVisibility( int fieldUID, const QCString& propertyName, bool visible );

		/*! Builds table field's schema by looking at the \a set. */
		KexiDB::Field * buildField( const KoProperty::Set &set );

	protected slots:
		/*! Equivalent to updateActions(false). Called on row insert/delete
		 in a KexiDataAwarePropertySet. */
		void updateActions();

		virtual void slotUpdateRowActions(int row);

		//! Called before cell change in tableview.
		void slotBeforeCellChanged(KexiTableItem *item, int colnum,
			QVariant& newValue, KexiDB::ResultInfo* result);

		//! Called on row change in a tableview.
		void slotRowUpdated(KexiTableItem *item);

		//! Called before row inserting in tableview.
		void slotRowInserted();
//		void slotAboutToInsertRow(KexiTableItem* item, KexiDB::ResultInfo* result, bool repaint);

		//! Called before row deleting in tableview.
		void slotAboutToDeleteRow(KexiTableItem& item, KexiDB::ResultInfo* result, bool repaint);

		/*! Called after any property has been changed in the current property set,
		 to perform some actions (like updating other dependent properties) */
		void slotPropertyChanged(KoProperty::Set& set, KoProperty::Property& property);

		/*! Toggles primary key for currently selected field.
		 Does nothing for empty row. */
		void slotTogglePrimaryKey();

		/*! Undoes the recently performed action. */
		void slotUndo();

		/*! Redoes the recently undoed action. */
		void slotRedo();

		/*! Reaction on command execution from the command history */
		void slotCommandExecuted(KCommand *command);

		/*! Simulates real execution of the Alter Table. For debugging. */
		void slotSimulateAlterTableExecution();

	protected:
		virtual void updateActions(bool activated);

		//! called whenever data should be reloaded (on switching to this view mode)
		void initData();

		/*! Creates a new property set for \a field.
		 The property set will be asigned to \a row, and owned by this dialog.
		 If \a newOne is true, the property set will be marked as newly created.
		 \return newly created property set. */
		KoProperty::Set* createPropertySet( int row, const KexiDB::Field& field, bool newOne = false );

		virtual tristate beforeSwitchTo(int mode, bool &dontStore);

		virtual tristate afterSwitchFrom(int mode);

		/*! \return property set associated with currently selected row (i.e. field)
		 or 0 if current row is empty. */
		virtual KoProperty::Set *propertySet();

		void removeCurrentPropertySet();

		/*! Reimplemented from KexiViewBase, because tables creation is more complex. 
		 No table schema altering is required, so just buildSchema() is used to create a new schema.
		*/
		virtual KexiDB::SchemaData* storeNewData(const KexiDB::SchemaData& sdata, bool &cancel);

		/*! Reimplemented from KexiViewBase, because table storage is more complex. 
		 Table schema altering may be required, so just buildSchema() is used to create a new schema.
		*/
		virtual tristate storeData(bool dontAsk = false);

		/*! Builds table schema by looking at the current design. Used in storeNewData() */
		tristate buildSchema(KexiDB::TableSchema &schema);

		/*! Builds action list usable for KexiDB::AlterTableHandler by looking at undo buffer 
		 of commands' history. Used in storeData() */
		tristate buildAlterTableActions(KexiDB::AlterTableHandler::ActionList &actions);

		/*! Helper, used for slotTogglePrimaryKey() and slotPropertyChanged().
		 Assigns primary key icon and value for property set \a propertySet, 
		 and deselects it from previous pkey's row. 
		 \a aWasPKey is internal. 
		 If \a commandGroup is not 0, it is used as parent group for storing actions' history. */
		void switchPrimaryKey(KoProperty::Set &propertySet, bool set, bool aWasPKey = false,
			CommandGroup* commandGroup = 0);

		//! Gets subtype strings and names for type \a fieldType.
		void getSubTypeListData(KexiDB::Field::TypeGroup fieldTypeGroup, 
			QStringList& stringsList, QStringList& namesList);

		void addHistoryCommand( KCommand* command, bool execute );

		//! Updates undo/redo shared actions availability by looking at command history's action
		void updateUndoRedoActions();

#ifdef KEXI_DEBUG_GUI
		void debugCommand( KCommand* command, int nestingLevel );
#endif

	private:
		KexiTableDesignerViewPrivate *d;
};

#endif
