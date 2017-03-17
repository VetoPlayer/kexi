/* This file is part of the KDE project
   Copyright (C) 2004 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2005-2014 Jarosław Staniek <staniek@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QObject>
#include <QPainter>
#include <QCursor>
#include <QFocusEvent>
#include <QList>
#include <QEvent>
#include <QKeyEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDebug>

#include "kexidbform.h"
#include "kexiformpart.h"
#include "kexiformmanager.h"
#include "kexiformscrollview.h"

#include <formeditor/objecttree.h>
#include <formeditor/widgetlibrary.h>
#include <widget/dataviewcommon/kexidataawareobjectiface.h>
#include <kexiutils/utils.h>
#include <kexi_global.h>

//! @internal
class Q_DECL_HIDDEN KexiDBForm::Private
{
public:
    Private()
            : dataAwareObject(0)
            , autoTabStops(true)
            , popupFocused(false)
            , orderedFocusWidgetsIteratorInitialized(false)
    {
    }

    ~Private() {
    }

    //! \return index of data-aware widget \a widget
    int indexOfDataAwareWidget(QWidget *widget) const {
        if (!dynamic_cast<KexiDataItemInterface*>(widget))
            return -1;
        return indexOfDataItem(dynamic_cast<KexiDataItemInterface*>(widget));
    }

    //! \return index of data item \a item, or -1 if not found
    int indexOfDataItem(KexiDataItemInterface* item) const {
        QHash<KexiDataItemInterface*, int>::const_iterator indicesForDataAwareWidgetsIt(
            indicesForDataAwareWidgets.find(item));
        if (indicesForDataAwareWidgetsIt == indicesForDataAwareWidgets.constEnd())
            return -1;
        //qDebug() << "column # for item:" << indicesForDataAwareWidgetsIt.value();
        return indicesForDataAwareWidgetsIt.value();
    }

    //! Sets orderedFocusWidgetsIterator member to a position pointing to \a widget
    void setOrderedFocusWidgetsIteratorTo(QWidget *widget) {
        if (orderedFocusWidgetsIteratorInitialized && *orderedFocusWidgetsIterator == widget)
            return;
        orderedFocusWidgetsIterator = orderedFocusWidgets.begin();
        orderedFocusWidgetsIteratorInitialized = true;
        /*foreach (QWidget *w, orderedFocusWidgets) {
            qDebug() << "orderedFocusWidget:" << w;
        }*/
        //qDebug() << "widget to focus:" << widget;
        while (orderedFocusWidgetsIterator != orderedFocusWidgets.end()
                && *orderedFocusWidgetsIterator != widget) {
            ++orderedFocusWidgetsIterator;
        }
    }

    KexiDataAwareObjectInterface* dataAwareObject;
    //! ordered list of focusable widgets (can be both data-widgets or buttons, etc.)
    QList<QWidget*> orderedFocusWidgets;
    //! ordered list of data-aware widgets
    QList<QWidget*> orderedDataAwareWidgets;
    //! a subset of orderedFocusWidgets mapped to indices
    QHash<KexiDataItemInterface*, int> indicesForDataAwareWidgets;
    QList<QWidget*>::iterator orderedFocusWidgetsIterator;
    QRect prev_rect; //!< previously selected rectangle
    bool autoTabStops;
    bool popupFocused; //!< used in KexiDBForm::eventFilter()
    bool orderedFocusWidgetsIteratorInitialized;
};

//========================

KexiDBForm::KexiDBForm(QWidget *parent, KexiDataAwareObjectInterface* dataAwareObject)
        : QWidget(parent)
        , KexiFormDataItemInterface()
        , d(new Private())
{
    installEventFilter(this);
    editedItem = 0;
    d->dataAwareObject = dataAwareObject;
    KexiDataItemInterface::setHasFocusableWidget(false);
    setCursor(QCursor(Qt::ArrowCursor)); //to avoid keeping Size cursor when moving from form's boundaries
    setAcceptDrops(true);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::NoFocus);
}

KexiDBForm::~KexiDBForm()
{
    //qDebug() << "close";
    delete d;
}

KexiDataAwareObjectInterface* KexiDBForm::dataAwareObject() const
{
    return d->dataAwareObject;
}

void KexiDBForm::setInvalidState(const QString& displayText)
{
    Q_UNUSED(displayText);

    //! @todo draw "invalid data source" text on the surface?
}

bool KexiDBForm::autoTabStops() const
{
    return d->autoTabStops;
}

void KexiDBForm::setAutoTabStops(bool set)
{
    d->autoTabStops = set;
}

QList<QWidget*>* KexiDBForm::orderedFocusWidgets() const
{
    return &d->orderedFocusWidgets;
}

QList<QWidget*>* KexiDBForm::orderedDataAwareWidgets() const
{
    return &d->orderedDataAwareWidgets;
}

void KexiDBForm::updateTabStopsOrder(KFormDesigner::Form* form)
{
    QWidget *fromWidget = 0;
    int numberOfDataAwareWidgets = 0;
    //generate a new list
    foreach (KFormDesigner::ObjectTreeItem* titem, *form->tabStops()) {
        if (titem->widget()->focusPolicy() & Qt::TabFocus) {
            if (fromWidget) {
                //qDebug() << "tab order:"
                //         << fromWidget->objectName() << "->" << titem->widget()->objectName();
            }
            fromWidget = titem->widget();
            d->orderedFocusWidgets.append(titem->widget());
        }

        titem->widget()->installEventFilter(this);
        //also filter events for data-aware children of this widget (i.e. KexiDBAutoField's editors)
        QList<QWidget*> children(titem->widget()->findChildren<QWidget*>());
        foreach(QWidget* widget, children) {
            /*qDebug() << "also adding '"
                << widget->metaObject()->className()
                << widget->objectName()
                << "' child to filtered widgets";*/
            widget->installEventFilter(this);
        }
        KexiFormDataItemInterface* dataItem
            = dynamic_cast<KexiFormDataItemInterface*>(titem->widget());
        if (dataItem && !dataItem->dataSource().isEmpty()) {
            //qDebug() << "#" << numberOfDataAwareWidgets << ": "
            //         << dataItem->dataSource() << "(" << titem->widget()->objectName() << ")";

//! @todo d->indicesForDataAwareWidgets SHOULD NOT BE UPDATED HERE BECAUSE
//! THERE CAN BE ALSO NON-TABSTOP DATA WIDGETS!
            d->indicesForDataAwareWidgets.insert(dataItem, numberOfDataAwareWidgets);
            numberOfDataAwareWidgets++;

            d->orderedDataAwareWidgets.append(titem->widget());
        }
    }//for
    /* else {
        //restore ordering
        for (QPtrListIterator<QWidget> it(d->orderedFocusWidgets); it.current(); ++it) {
          if (fromWidget) {
            qDebug() << "tab order:" << fromWidget->name()
              << "->" << it.current()->name();
            setTabOrder( fromWidget, it.current() );
          }
          fromWidget = it.current();
        }
    //  SET_FOCUS_USING_REASON(focusWidget(), QFocusEvent::Tab);
      }*/
}

void KexiDBForm::updateTabStopsOrder()
{
    for (QList<QWidget*>::iterator it(d->orderedFocusWidgets.begin());
            it != d->orderedFocusWidgets.end();)
    {
        if (!((*it)->focusPolicy() & Qt::TabFocus))
            it = d->orderedFocusWidgets.erase(it);
        else
            ++it;
    }
}

void KexiDBForm::updateReadOnlyFlags()
{
    foreach(QWidget *w, d->orderedDataAwareWidgets) {
        KexiFormDataItemInterface* dataItem = dynamic_cast<KexiFormDataItemInterface*>(w);
        if (dataItem && !dataItem->dataSource().isEmpty()) {
            if (dataAwareObject()->isReadOnly()) {
                dataItem->setReadOnly(true);
            }
        }
    }
}

bool KexiDBForm::eventFilter(QObject * watched, QEvent * e)
{
#if 0
    if (e->type() != QEvent::Paint
            && e->type() != QEvent::Leave
            && e->type() != QEvent::MouseMove
            && e->type() != QEvent::HoverMove
            && e->type() != QEvent::HoverEnter
            && e->type() != QEvent::HoverLeave)
    {
        qDebug() << e << watched;
    }
    if (e->type() == QEvent::Resize && watched == this)
        qDebug() << "RESIZE";
#endif
    if (e->type() == QEvent::KeyPress || e->type() == QEvent::ShortcutOverride) {
        if (isPreviewing()) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(e);
            const int key = ke->key();
            bool tab = ke->modifiers() == Qt::NoModifier && key == Qt::Key_Tab;
            bool backtab = ((ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier) && key == Qt::Key_Backtab)
                           || (ke->modifiers() == Qt::ShiftModifier && key == Qt::Key_Tab);
            QObject *o = watched; //focusWidget();
            QWidget* realWidget = dynamic_cast<QWidget*>(o); //will beused below (for tab/backtab handling)

            if (realWidget && !tab && !backtab) {
                //for buttons, left/up and right/down keys act like tab/backtab (see qbutton.cpp)
                if (realWidget->inherits("QButton")) {
                    if (ke->modifiers() == Qt::NoModifier && (key == Qt::Key_Right || key == Qt::Key_Down))
                        tab = true;
                    else if (ke->modifiers() == Qt::NoModifier && (key == Qt::Key_Left || key == Qt::Key_Up))
                        backtab = true;
                }
            }

            if (!tab && !backtab) {
                // allow the editor widget to grab the key press event
                while (true) {
                    if (!o || o == dynamic_cast<QObject*>(d->dataAwareObject))
                        break;
                    if (dynamic_cast<KexiFormDataItemInterface*>(o)) {
                        realWidget = dynamic_cast<QWidget*>(o); //will be used below
                        if (realWidget == this) //we have encountered 'this' form surface, give up
                            return false;
                        KexiFormDataItemInterface* dataItemIface = dynamic_cast<KexiFormDataItemInterface*>(o);
                        while (dataItemIface) {
                            if (dataItemIface->keyPressed(ke))
                                return false;
                            dataItemIface = dynamic_cast<KexiFormDataItemInterface*>(dataItemIface->parentDataItemInterface()); //try in parent, e.g. in combobox
                        }
                        break;
                    }
                    o = o->parent();
                }
                // try to handle global shortcuts at the KexiDataAwareObjectInterface
                // level (e.g. for "next record" action)
                int curRow = d->dataAwareObject->currentRecord();
                int curCol = d->dataAwareObject->currentColumn();
                bool moveToFirstField; //if true, we'll move focus to the first field (in tab order)
                bool moveToLastField; //if true, we'll move focus to the first field (in tab order)
                if (!(ke->modifiers() == Qt::NoModifier && (key == Qt::Key_Home
                        || key == Qt::Key_End || key == Qt::Key_Down || key == Qt::Key_Up))
                        /* ^^ home/end/down/up are already handled by widgets */
                        && !ke->isAccepted()
                        && d->dataAwareObject->handleKeyPress(
                            ke, &curRow, &curCol, false/*!fullRecordSelection*/, &moveToFirstField, &moveToLastField)) {
                    if (ke->isAccepted())
                        return true;
                    QWidget* widgetToFocus;
                    if (moveToFirstField) {
                        widgetToFocus = d->orderedFocusWidgets.first(); //?
                        curCol = d->indexOfDataAwareWidget(widgetToFocus);
                    } else if (moveToLastField) {
                        widgetToFocus = d->orderedFocusWidgets.last(); //?
                        curCol = d->indexOfDataAwareWidget(widgetToFocus);
                    } else
                        widgetToFocus = d->orderedDataAwareWidgets.at(curCol);   //?

                    d->dataAwareObject->setCursorPosition(curRow, curCol);

                    if (widgetToFocus) {
                        widgetToFocus->setFocus();
                        KexiFormDataItemInterface *formItem = dynamic_cast<KexiFormDataItemInterface*>(widgetToFocus);
                        if (formItem) {
                            formItem->selectAllOnFocusIfNeeded();
                        }
                    }
                    else {
                        qWarning() << "widgetToFocus not found!";
                    }

                    ke->accept();
                    return true;
                }
                if (key == Qt::Key_Delete && ke->modifiers() == Qt::ControlModifier) {
//! @todo remove hardcoded shortcuts: can be reconfigured...
                    d->dataAwareObject->deleteCurrentRecord();
                    return true;
                }
            }
            // handle Esc key
            if (ke->modifiers() == Qt::NoModifier && key == Qt::Key_Escape) {
                //cancel field editing/row editing if possible
                if (d->dataAwareObject->cancelEditor())
                    return true;
                else if (d->dataAwareObject->cancelRecordEditing())
                    return true;
                return false; // canceling not needed - pass the event to the active widget
            }
            // jstaniek: Fix for Qt bug (handling e.g. Alt+2, Ctrl+2 keys on every platform)
            //           It's important because we're using alt+2 short cut by default
            //           Damn! I've reported this to Trolltech in November 2004 - still not fixed.
            if (ke->isAccepted() && (ke->modifiers() & Qt::AltModifier) && ke->text() >= "0" && ke->text() <= "9")
                return true;

            if (tab || backtab) {
                //the watched widget can be a subwidget of a real widget, e.g. a drop down button of image box: find it
                while (realWidget && !KexiFormManager::self()->library()->widgetInfoForClassName(
                            realWidget->metaObject()->className()))
                {
                    realWidget = realWidget->parentWidget();
                }
                if (!realWidget)
                    return true; //ignore
                //the watched widget can be a subwidget of a real widget, e.g. autofield: find it
                //QWidget* realWidget = static_cast<QWidget*>(watched);
                KexiDataItemInterface *iface;
                while ((iface = dynamic_cast<KexiDataItemInterface*>(realWidget))
                       && iface->parentDataItemInterface())
                {
                    realWidget = dynamic_cast<QWidget*>(iface->parentDataItemInterface());
                }

                d->setOrderedFocusWidgetsIteratorTo(realWidget);
                iface = dynamic_cast<KexiDataItemInterface*>(realWidget);
                if (iface) {
                    iface->moveCursorToEnd();
                }

                //qDebug() << realWidget->objectName();

                // find next/prev widget to focus
                QWidget *widgetToFocus = 0;
                bool wasAtFirstWidget = false; //used to protect against infinite loop
                while (true) {
                    if (tab) {
                        if (!d->orderedFocusWidgets.isEmpty() && realWidget == d->orderedFocusWidgets.last()) {
                            if (wasAtFirstWidget)
                                break;
                            d->orderedFocusWidgetsIterator = d->orderedFocusWidgets.begin();
                            wasAtFirstWidget = true;
                        } else if (realWidget == *d->orderedFocusWidgetsIterator) {
                            ++d->orderedFocusWidgetsIterator; //next
                        } else {
                            return true; //ignore
                        }
                    } else {//backtab
                        if (!d->orderedFocusWidgets.isEmpty() && realWidget == d->orderedFocusWidgets.first()) {
                            d->orderedFocusWidgetsIterator
                                = d->orderedFocusWidgets.begin() + (d->orderedFocusWidgets.count() - 1);
                        } else if (realWidget == *d->orderedFocusWidgetsIterator) {
                            --d->orderedFocusWidgetsIterator; //prev
                        } else {
                            return true; //ignore
                        }
                    }

                    widgetToFocus = *d->orderedFocusWidgetsIterator;

                    QObject *pageFor_widgetToFocus = 0;
                    KFormDesigner::TabWidget *tabWidgetFor_widgetToFocus
                        = KFormDesigner::findParent<KFormDesigner::TabWidget>(
                            widgetToFocus,
                            "KFormDesigner::TabWidget",
                            pageFor_widgetToFocus
                        );
                    if (   tabWidgetFor_widgetToFocus
                        && tabWidgetFor_widgetToFocus->currentWidget() != pageFor_widgetToFocus)
                    {
                        realWidget = widgetToFocus;
                        continue; // the new widget to focus is placed on invisible tab page:
                        // move to next widget
                    }
                    break;
                }//while

                QWidget *widgetToSelectAll = widgetToFocus;

                //set focus, but don't use just setFocus() because certain widgets
                //behaves differently (e.g. QLineEdit calls selectAll()) when
                //focus event's reason is Qt::TabFocusReason
                if (widgetToFocus->focusProxy()) {
                    widgetToFocus = widgetToFocus->focusProxy();
                }

                if (widgetToFocus && d->dataAwareObject->acceptEditor()) {
                    if (tab) {
                        widgetToFocus->setFocus();
                        //qDebug() << "focusing" << widgetToFocus->objectName();
                        (*d->orderedFocusWidgetsIterator)->setFocus();
                        //qDebug() << "focusing" << (*d->orderedFocusWidgetsIterator)->objectName();
                    }
                    KexiFormDataItemInterface *formItem = dynamic_cast<KexiFormDataItemInterface*>(widgetToSelectAll);
                    if (formItem) {
                        //qDebug() << "widgetToSelectAll:" << widgetToSelectAll;
                        formItem->selectAllOnFocusIfNeeded();
                    }
                }
                return true;
            }
        }
    } else if (e->type() == QEvent::FocusIn || (e->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent*>(e)->button() == Qt::LeftButton)) {
        bool focusDataWidget = isPreviewing();
        if (static_cast<QFocusEvent*>(e)->reason() == Qt::PopupFocusReason) {
            //qDebug() << "->>> focus IN, popup" << watched;
            focusDataWidget = !d->popupFocused;
            d->popupFocused = false;
        }

        if (focusDataWidget) {
            //qDebug() << "FocusIn:" << watched->metaObject()->className() << watched->objectName();
            if (d->dataAwareObject) {
                QWidget *dataItem = dynamic_cast<QWidget*>(watched);
                while (dataItem) {
                    while (dataItem && !dynamic_cast<KexiDataItemInterface*>(dataItem)) {
                        dataItem = dataItem->parentWidget();
                    }
                    if (!dataItem) {
                        break;
                    }
                    //qDebug() << "FocusIn: FOUND" << dataItem->metaObject()->className() << dataItem->objectName();

                    const int index = d->indexOfDataAwareWidget(dataItem);
                    if (index >= 0) {
                        //qDebug() << "moving cursor to column #" << index;
                        editedItem = 0;
                        if ((int)index != d->dataAwareObject->currentColumn()) {
                            d->dataAwareObject->setCursorPosition(d->dataAwareObject->currentRecord(), index /*column*/);
                        }
                        break;
                    } else {
                        dataItem = dataItem->parentWidget();
                    }

                    dataItem->update();
                }
            }
        }
    } else if (e->type() == QEvent::FocusOut) {
        if (static_cast<QFocusEvent*>(e)->reason() == Qt::PopupFocusReason) {
            d->popupFocused = true;
        } else {
            d->popupFocused = false;
        }
//  qDebug() << "e->type()==QEvent::FocusOut" << watched->className() << watched->name();
    }
    return QWidget::eventFilter(watched, e);
}

bool KexiDBForm::valueIsNull()
{
    return true;
}

bool KexiDBForm::valueIsEmpty()
{
    return true;
}

bool KexiDBForm::isReadOnly() const
{
    if (d->dataAwareObject)
        return d->dataAwareObject->isReadOnly();
//! @todo ?
    return false;
}

void KexiDBForm::setReadOnly(bool readOnly)
{
    if (d->dataAwareObject)
        d->dataAwareObject->setReadOnly(readOnly);   //???
}

QWidget* KexiDBForm::widget()
{
    return this;
}

bool KexiDBForm::cursorAtStart()
{
    return false;
}

bool KexiDBForm::cursorAtEnd()
{
    return false;
}

void KexiDBForm::clear()
{
    //! @todo clear all fields?
}

bool KexiDBForm::isPreviewing() const
{
    const KexiFormScrollView *view = dynamic_cast<KexiFormScrollView*>(d->dataAwareObject);
    return view ? view->isPreviewing() : false;
}

void KexiDBForm::dragMoveEvent(QDragMoveEvent *e)
{
    QWidget::dragMoveEvent(e);
    emit handleDragMoveEvent(e);
}

void KexiDBForm::dropEvent(QDropEvent *e)
{
    QWidget::dropEvent(e);
    emit handleDropEvent(e);
}

void KexiDBForm::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    if (isPreviewing()) {
        // Force background in styles like Oxygen
        QPainter p;
        p.begin(this);
        p.fillRect(e->rect(), palette().brush(backgroundRole()));
        p.end();
    }
}

