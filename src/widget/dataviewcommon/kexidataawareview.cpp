/* This file is part of the KDE project
   Copyright (C) 2005-2014 Jarosław Staniek <staniek@kde.org>

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

#include "kexidataawareview.h"
#include <kexidataawareobjectiface.h>
#include <utils/kexisharedactionclient.h>
#include <core/KexiMainWindowIface.h>
#include <core/KexiStandardAction.h>
#include <core/KexiWindow.h>

#include <KDbRecordEditBuffer>
#include <KDbTableViewColumn>

#include <KActionCollection>

#include <QDebug>
#include <QMenu>

class Q_DECL_HIDDEN KexiDataAwareView::Private
{
public:
    Private()
      : internalView(0)
      , actionClient(0)
      , dataAwareObject(0)
    {
    }

    QWidget* internalView;
    KexiSharedActionClient* actionClient;
    KexiDataAwareObjectInterface* dataAwareObject;
};

KexiDataAwareView::KexiDataAwareView(QWidget *parent)
        : KexiView(parent)
        , KexiSearchAndReplaceViewInterface()
        , d(new Private)
{
}

KexiDataAwareView::~KexiDataAwareView()
{
    delete d;
}

void KexiDataAwareView::init(QWidget* viewWidget, KexiSharedActionClient* actionClient,
                             KexiDataAwareObjectInterface* dataAwareObject, bool noDataAware)
{
    d->internalView = viewWidget;
    d->actionClient = actionClient;
    d->dataAwareObject = dataAwareObject;
    setViewWidget(d->internalView, true);

    if (!noDataAware) {
        d->dataAwareObject->connectCellSelectedSignal(this, SLOT(slotCellSelected(int,int)));

        //! before closing - we'are accepting editing
        connect(this, SIGNAL(closing(bool*)), this, SLOT(slotClosing(bool*)));

        //! updating actions on start/stop editing
        d->dataAwareObject->connectRecordEditingStartedSignal(
            this, SLOT(slotUpdateRecordActions(int)));
        d->dataAwareObject->connectRecordEditingTerminatedSignal(
            this, SLOT(slotUpdateRecordActions(int)));
        d->dataAwareObject->connectUpdateSaveCancelActionsSignal(
            this, SLOT(slotUpdateSaveCancelActions()));
        d->dataAwareObject->connectReloadActionsSignal(
            this, SLOT(reloadActions()));
    }
    setMinimumSize(d->internalView->minimumSizeHint().width(),
                   d->internalView->minimumSizeHint().height());
    resize(preferredSizeHint(d->internalView->sizeHint()));
    setFocusProxy(d->internalView);

    if (!noDataAware) {
        initActions();
        reloadActions();
    }
}

void KexiDataAwareView::initActions()
{
    // - setup local actions
    QList<QAction*> viewActions;
    KActionCollection *ac = KexiMainWindowIface::global()->actionCollection();
    viewActions
        << ac->action("data_save_row")
        << ac->action("data_cancel_row_changes");

    QAction *a = new QAction(this);
    a->setSeparator(true);
    viewActions << a;

    if (d->dataAwareObject->isSortingEnabled()) {
        viewActions
        << KexiStandardAction::sortAscending(this, SLOT(sortAscending()), this)
        << KexiStandardAction::sortDescending(this, SLOT(sortDescending()), this);
    }
    viewActions
        << ac->action("edit_find");
    setViewActions(viewActions);

    plugSharedAction("edit_delete_row", this, SLOT(deleteCurrentRecord()));
    d->actionClient->plugSharedAction(sharedAction("edit_delete_row")); //for proper shortcut

    plugSharedAction("edit_delete", this, SLOT(deleteAndStartEditCurrentCell()));
    d->actionClient->plugSharedAction(sharedAction("edit_delete")); //for proper shortcut

    plugSharedAction("edit_edititem", this, SLOT(startEditOrToggleValue()));
    d->actionClient->plugSharedAction(sharedAction("edit_edititem")); //for proper shortcut

    plugSharedAction("data_save_row", this, SLOT(acceptRecordEditing()));
    d->actionClient->plugSharedAction(sharedAction("data_save_row")); //for proper shortcut

    plugSharedAction("data_cancel_row_changes", this, SLOT(cancelRecordEditing()));
    d->actionClient->plugSharedAction(sharedAction("data_cancel_row_changes")); //for proper shortcut
    d->actionClient->plugSharedAction(sharedAction("edit_insert_empty_row")); //for proper shortcut

    setAvailable("data_sort_az", d->dataAwareObject->isSortingEnabled());
    setAvailable("data_sort_za", d->dataAwareObject->isSortingEnabled());
//! \todo  plugSharedAction("data_filter", this, SLOT(???()));

    plugSharedAction("data_go_to_first_record", this, SLOT(slotGoToFirstRecord()));
    plugSharedAction("data_go_to_previous_record", this, SLOT(slotGoToPreviusRecord()));
    plugSharedAction("data_go_to_next_record", this, SLOT(slotGoToNextRecord()));
    plugSharedAction("data_go_to_last_record", this, SLOT(slotGoToLastRecord()));
    plugSharedAction("data_go_to_new_record", this, SLOT(slotGoToNewRecord()));

//! \todo update availability
    setAvailable("data_go_to_first_record", true);
    setAvailable("data_go_to_previous_record", true);
    setAvailable("data_go_to_next_record", true);
    setAvailable("data_go_to_last_record", true);
    setAvailable("data_go_to_new_record", true);

    plugSharedAction("edit_copy", this, SLOT(copySelection()));
    d->actionClient->plugSharedAction(sharedAction("edit_copy")); //for proper shortcut

    plugSharedAction("edit_cut", this, SLOT(cutSelection()));
    d->actionClient->plugSharedAction(sharedAction("edit_cut")); //for proper shortcut

    plugSharedAction("edit_paste", this, SLOT(paste()));
    d->actionClient->plugSharedAction(sharedAction("edit_paste")); //for proper shortcut

// plugSharedAction("edit_find", this, SLOT(editFind()));
// d->actionClient->plugSharedAction(sharedAction("edit_find")); //for proper shortcut

// plugSharedAction("edit_findnext", this, SLOT(editFindNext()));
// d->actionClient->plugSharedAction(sharedAction("edit_findnext")); //for proper shortcut

// plugSharedAction("edit_findprevious", this, SLOT(editFindPrevious()));
// d->actionClient->plugSharedAction(sharedAction("edit_findprev")); //for proper shortcut

//! @todo plugSharedAction("edit_replace", this, SLOT(editReplace()));
//! @todo d->actionClient->plugSharedAction(sharedAction("edit_replace")); //for proper shortcut

// setAvailable("edit_find", true);
// setAvailable("edit_findnext", true);
// setAvailable("edit_findprevious", true);
//! @todo setAvailable("edit_replace", true);
}

void KexiDataAwareView::slotUpdateRecordActions(int row)
{
    const bool ro = d->dataAwareObject->isReadOnly();
// const bool inserting = d->dataAwareObject->isInsertingEnabled();
    const bool deleting = d->dataAwareObject->isDeleteEnabled();
    const bool emptyInserting = d->dataAwareObject->isEmptyRecordInsertingEnabled();
    const bool editing = isDataEditingInProgress();
    const bool sorting = d->dataAwareObject->isSortingEnabled();
    const int rows = d->dataAwareObject->recordCount();
    const bool insertRowFocusedWithoutEditing = !editing && row == rows;

    setAvailable("edit_cut", !ro && !insertRowFocusedWithoutEditing);
    setAvailable("edit_copy", !insertRowFocusedWithoutEditing);
    setAvailable("edit_paste", !ro);
    setAvailable("edit_delete", !ro && !insertRowFocusedWithoutEditing);
    setAvailable("edit_delete_row", !ro && !(deleting && row == rows));
    setAvailable("edit_insert_empty_row", !ro && emptyInserting);
    setAvailable("edit_clear_table", !ro && deleting && rows > 0);
    setAvailable("data_sort_az", sorting);
    setAvailable("data_sort_za", sorting);
    slotUpdateSaveCancelActions();
}

void KexiDataAwareView::slotUpdateSaveCancelActions()
{
    // 'save row' enabled when editing and there's anything to save
    //const bool editing = isDataEditingInProgress();
    setAvailable("data_save_row", d->dataAwareObject->recordEditing() >= 0);
    // 'cancel row changes' enabled when editing
    setAvailable("data_cancel_row_changes", d->dataAwareObject->recordEditing() >= 0);
}

QWidget* KexiDataAwareView::mainWidget() const
{
    return d->internalView;
}

QSize KexiDataAwareView::minimumSizeHint() const
{
    return d->internalView ? d->internalView->minimumSizeHint() : QSize(0, 0);
}

QSize KexiDataAwareView::sizeHint() const
{
    return d->internalView ? d->internalView->sizeHint() : QSize(0, 0);
}

KexiDataAwareObjectInterface* KexiDataAwareView::dataAwareObject() const
{
    return d->dataAwareObject;
}

void KexiDataAwareView::updateActions(bool activated)
{
    setAvailable("data_sort_az", d->dataAwareObject->isSortingEnabled());
    setAvailable("data_sort_za", d->dataAwareObject->isSortingEnabled());
    KexiView::updateActions(activated);
}

QWidget* KexiDataAwareView::internalView() const
{
    return d->internalView;
}

QAction* KexiDataAwareView::sharedActionRequested(QKeyEvent *ke, const char *actionName)
{
    QAction *a = sharedAction(actionName);
    return a && QKeySequence(ke->key() | ke->modifiers()) == a->shortcut()
            ? a : 0;
}

bool KexiDataAwareView::eventFilter(QObject *o, QEvent *e)
{
    // qDebug() << "***" << o << e << window()->selectedView() << this;
//    if (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) {
//        qDebug() << "F O C U S" << e << o;
//    }
    if (o == this) {
        switch (e->type()) {
        case QEvent::ShortcutOverride: {
            QKeyEvent *ke = static_cast<QKeyEvent*>(e);
            QAction *a = sharedActionRequested(ke, "data_cancel_row_changes");
            if (!a) {
                break;
            }
            KexiDataItemInterface *editor = d->dataAwareObject->editor();
            if (editor) {
                d->dataAwareObject->cancelEditor();
                editor->moveCursorToEnd();
                editor->selectAll();
            }
            else {
                a->trigger();
            }
            return true;
        }
        case QEvent::KeyPress: {
            QKeyEvent *ke = static_cast<QKeyEvent*>(e);
            QAction *a = sharedActionRequested(ke, "data_save_row");
            if (a) {
                a->trigger();
                KexiDataItemInterface *editor = d->dataAwareObject->editor();
                if (editor) {
                    editor->moveCursorToEnd();
                    editor->selectAll();
                }
                return true;
            }
            foreach (const QByteArray& actionName,
                     QList<QByteArray>() << "edit_copy" << "edit_cut" << "edit_paste")
            {
                a = sharedActionRequested(ke, actionName);
                if (a) {
                    a->trigger();
                    return true;
                }
            }
            break;
        }
        default:;
        }
    }
    return KexiView::eventFilter(o, e);
}

void KexiDataAwareView::reloadActions()
{
//! @todo Move this to the table part
    d->dataAwareObject->contextMenu()->clear();
    if (!d->dataAwareObject->contextMenuTitleText().isEmpty()) {
        d->dataAwareObject->contextMenu()->addSection(
            d->dataAwareObject->contextMenuTitleIcon(),
            d->dataAwareObject->contextMenuTitleText());
    }

    plugSharedAction("edit_cut", d->dataAwareObject->contextMenu());
    plugSharedAction("edit_copy", d->dataAwareObject->contextMenu());
    plugSharedAction("edit_paste", d->dataAwareObject->contextMenu());

    bool separatorNeeded = true;

    unplugSharedAction("edit_clear_table");
    plugSharedAction("edit_clear_table", this, SLOT(deleteAllRecords()));

    if (d->dataAwareObject->isEmptyRecordInsertingEnabled()) {
        unplugSharedAction("edit_insert_empty_row");
        plugSharedAction("edit_insert_empty_row", d->internalView, SLOT(insertEmptyRecord()));
        if (separatorNeeded)
            d->dataAwareObject->contextMenu()->addSeparator();
        plugSharedAction("edit_insert_empty_row", d->dataAwareObject->contextMenu());
    } else {
        unplugSharedAction("edit_insert_empty_row");
        unplugSharedAction("edit_insert_empty_row", d->dataAwareObject->contextMenu());
    }

    if (d->dataAwareObject->isDeleteEnabled()) {
        if (separatorNeeded)
            d->dataAwareObject->contextMenu()->addSeparator();
        plugSharedAction("edit_delete", d->dataAwareObject->contextMenu());
        plugSharedAction("edit_delete_row", d->dataAwareObject->contextMenu());
    } else {
        unplugSharedAction("edit_delete_row", d->dataAwareObject->contextMenu());
        unplugSharedAction("edit_delete_row", d->dataAwareObject->contextMenu());
    }
    setAvailable("data_sort_az", d->dataAwareObject->isSortingEnabled());
    setAvailable("data_sort_za", d->dataAwareObject->isSortingEnabled());

    slotCellSelected(d->dataAwareObject->currentRecord(), d->dataAwareObject->currentColumn());
}

void KexiDataAwareView::slotCellSelected(int row, int col)
{
    Q_UNUSED(col);
    slotUpdateRecordActions(row);
}

void KexiDataAwareView::deleteAllRecords()
{
    d->dataAwareObject->deleteAllRecords(true/*ask*/, true/*repaint*/);
}

void KexiDataAwareView::deleteCurrentRecord()
{
    d->dataAwareObject->deleteCurrentRecord();
}

void KexiDataAwareView::deleteAndStartEditCurrentCell()
{
    d->dataAwareObject->deleteAndStartEditCurrentCell();
}

void KexiDataAwareView::startEditOrToggleValue()
{
    d->dataAwareObject->startEditOrToggleValue();
}

bool KexiDataAwareView::acceptRecordEditing()
{
    return d->dataAwareObject->acceptRecordEditing();
}

void KexiDataAwareView::slotClosing(bool* cancel)
{
    if (!acceptRecordEditing())
        *cancel = true;
}

bool KexiDataAwareView::cancelRecordEditing()
{
    return d->dataAwareObject->cancelRecordEditing();
}

void KexiDataAwareView::sortAscending()
{
    d->dataAwareObject->sortAscending();
}

void KexiDataAwareView::sortDescending()
{
    d->dataAwareObject->sortDescending();
}

void KexiDataAwareView::copySelection()
{
    d->dataAwareObject->copySelection();
}

void KexiDataAwareView::cutSelection()
{
    d->dataAwareObject->cutSelection();
}

void KexiDataAwareView::paste()
{
    d->dataAwareObject->paste();
}

void KexiDataAwareView::slotGoToFirstRecord()
{
    d->dataAwareObject->selectFirstRecord();
}
void KexiDataAwareView::slotGoToPreviusRecord()
{
    d->dataAwareObject->selectPreviousRecord();
}
void KexiDataAwareView::slotGoToNextRecord()
{
    d->dataAwareObject->selectNextRecord();
}
void KexiDataAwareView::slotGoToLastRecord()
{
    d->dataAwareObject->selectLastRecord();
}
void KexiDataAwareView::slotGoToNewRecord()
{
    d->dataAwareObject->addNewRecordRequested();
}

bool KexiDataAwareView::setupFindAndReplace(QStringList& columnNames, QStringList& columnCaptions,
        QString& currentColumnName)
{
    if (!dataAwareObject() || !dataAwareObject()->data())
        return false;
    const QList<KDbTableViewColumn*> *columns = dataAwareObject()->data()->columns();
    foreach(const KDbTableViewColumn *col, *columns) {
        if (!col->isVisible())
            continue;
        columnNames.append(col->field()->name());
        columnCaptions.append(col->captionAliasOrName());
    }

    //update "look in" selection if there was any
    const int currentColumnNumber = dataAwareObject()->currentColumn();
    if (currentColumnNumber >= 0 && currentColumnNumber < columns->count()) {
        KDbTableViewColumn *col = columns->at(currentColumnNumber);
        if (col && col->field())
            currentColumnName = col->field()->name();
    }
    return true;
}

tristate KexiDataAwareView::find(const QVariant& valueToFind,
                                 const KexiSearchAndReplaceViewInterface::Options& options, bool next)
{
    if (!dataAwareObject() || !dataAwareObject()->data())
        return cancelled;

    return dataAwareObject()->find(valueToFind, options, next);
    //! @todo result...
    //! @todo else if (res == KexiFindDialog::Replace) {
    //! @todo else if (res == KexiFindDialog::ReplaceAll) {
}

tristate KexiDataAwareView::findNextAndReplace(const QVariant& valueToFind,
        const QVariant& replacement,
        const KexiSearchAndReplaceViewInterface::Options& options, bool replaceAll)
{
    if (!dataAwareObject() || !dataAwareObject()->data())
        return cancelled;

    return dataAwareObject()->findNextAndReplace(valueToFind, replacement, options, replaceAll);
}

bool KexiDataAwareView::isDataEditingInProgress() const
{
    if (!d->dataAwareObject->recordEditing()
        || !d->dataAwareObject->data()
        || !d->dataAwareObject->data()->recordEditBuffer())
    {
        return false;
    }
    // true if edit buffer is not empty or at least there is editor with changed value
    return !d->dataAwareObject->data()->recordEditBuffer()->isEmpty()
           || (d->dataAwareObject->editor() && d->dataAwareObject->editor()->valueChanged());
}

tristate KexiDataAwareView::saveDataChanges()
{
    return acceptRecordEditing();
}

tristate KexiDataAwareView::cancelDataChanges()
{
    return cancelRecordEditing();
}

