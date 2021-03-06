/* This file is part of the KDE project
   Copyright (C) 2010 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2010-2014 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KexiProjectModel.h"
#include "KexiProjectModelItem.h"
#include <kexipart.h>
#include <kexipartinfo.h>
#include <kexipartitem.h>
#include <kexiproject.h>
#include <kexipartmanager.h>

#include <KDbUtils>

#include <QDebug>

class Q_DECL_HIDDEN KexiProjectModel::Private {
public:
    Private();
    ~Private();

    //!Part class to display
    QString itemsPartClass;
    KexiProjectModelItem *rootItem;
    QPersistentModelIndex searchHighlight;
    QPointer<KexiProject> project;
    int objectsCount;
};

KexiProjectModel::Private::Private() : rootItem(0), objectsCount(0)
{

}

KexiProjectModel::Private::~Private()
{
    delete rootItem;
}


KexiProjectModel::KexiProjectModel(QObject* parent): QAbstractItemModel(parent) , d(new Private())
{
    //qDebug();
    d->rootItem = new KexiProjectModelItem(QString());
}

KexiProject* KexiProjectModel::project() const
{
    return d->project;
}

void KexiProjectModel::setProject(KexiProject* prj, const QString& itemsPartClass, QString* partManagerErrorMessages)
{
    d->project = prj;
    //qDebug() << itemsPartClass << ".";
    clear();
    d->itemsPartClass = itemsPartClass;

    d->rootItem = new KexiProjectModelItem(prj ? prj->data()->databaseName() : QString());

    KexiPart::PartInfoList* plist = Kexi::partManager().infoList();
    if (!plist)
        return;

    foreach(KexiPart::Info *info, *plist) {
        if (!info->isVisibleInNavigator())
            continue;

        if (!d->itemsPartClass.isEmpty() && info->pluginId() != d->itemsPartClass)
            continue;

        //load part - we need this to have GUI merged with part's actions
//! @todo FUTURE - don't do that when DESIGN MODE is OFF
        //qDebug() << info->pluginId() << info->objectName();
        KexiProjectModelItem *groupItem = 0;
        if (d->itemsPartClass.isEmpty() /*|| m_itemsPartClass == info->pluginId()*/) {
            groupItem = addGroup(info, d->rootItem);
            if (!groupItem) {
                continue;
            } else {
                d->rootItem->appendChild(groupItem);
            }

        } else {
            groupItem = d->rootItem;
        }

        //lookup project's objects (part items)
//! @todo FUTURE - don't do that when DESIGN MODE is OFF
        KexiPart::ItemDict *item_dict = prj ? prj->items(info) : 0;
        if (!item_dict) {
            continue;
        }

        foreach(KexiPart::Item *item, *item_dict) {
            addItem(info, item, groupItem);
        }

        groupItem->sortChildren();
        if (!d->itemsPartClass.isEmpty()) {
            break; //the only group added, so our work is completed
        }
    }
    if (partManagerErrorMessages && !partManagerErrorMessages->isEmpty())
        partManagerErrorMessages->append("</ul></p>");
}



KexiProjectModel::~KexiProjectModel()
{
    delete d;
}

QVariant KexiProjectModel::data(const QModelIndex& index, int role) const
{
    KexiProjectModelItem *item = static_cast<KexiProjectModelItem*>(index.internalPointer());
    if (!item)
        return QVariant();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::WhatsThisRole:
        return item->data(index.column());
    case Qt::DecorationRole:
        return item->icon();
    default:;
    }

    return QVariant();
}

int KexiProjectModel::columnCount(const QModelIndex& parent) const
{
     if (parent.isValid())
         return static_cast<KexiProjectModelItem*>(parent.internalPointer())->columnCount();
     else
         return d->rootItem->columnCount();
}

int KexiProjectModel::rowCount(const QModelIndex& parent) const
{
     KexiProjectModelItem *parentItem;
     if (parent.column() > 0)
         return 0;

     if (!parent.isValid())
         parentItem = d->rootItem;
     else
         parentItem = static_cast<KexiProjectModelItem*>(parent.internalPointer());

     if (parentItem)
        return parentItem->childCount();
     else
         return 0;
}

QModelIndex KexiProjectModel::parent(const QModelIndex& index) const
{
     if (!index.isValid())
         return QModelIndex();

     KexiProjectModelItem *childItem = static_cast<KexiProjectModelItem*>(index.internalPointer());
     KexiProjectModelItem *parentItem = childItem->parent();

     if (!parentItem)
         return QModelIndex();

     if (parentItem == d->rootItem)
         return QModelIndex();

     return createIndex(parentItem->row(), 0, parentItem);
}

QModelIndex KexiProjectModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    KexiProjectModelItem *parentItem;

    if (!parent.isValid()) {
        parentItem = d->rootItem;
    } else {
        parentItem = static_cast<KexiProjectModelItem*>(parent.internalPointer());
    }

    KexiProjectModelItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

bool KexiProjectModel::hasChildren(const QModelIndex& parent) const
{
    return QAbstractItemModel::hasChildren(parent);
}

bool KexiProjectModel::renameItem(KexiPart::Item *item, const QString& newName)
{
    if (item->name() == newName) { //make sure the new name is different
        return false;
    }
    KexiProjectModelItem *i = modelItemFromItem(*item);
    if (!i) {
        return false;
    }
    QModelIndex origIndex = indexFromItem(i);
    bool ok = true;
    emit renameItem(item, newName, &ok);
    if (ok) {
        emit layoutAboutToBeChanged();
        i->parent()->sortChildren();
        changePersistentIndex(origIndex, indexFromItem(i));
        emit layoutChanged();
    }
    return ok;
}

bool KexiProjectModel::setItemCaption(KexiPart::Item *item, const QString& newCaption)
{
    if (item->caption() == newCaption) { //make sure the new caption is different
        return false;
    }
    bool ok = true;
    emit changeItemCaption(item, newCaption, &ok);
    return ok;
}

Qt::ItemFlags KexiProjectModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
         return QAbstractItemModel::flags(index);

    KexiProjectModelItem *item = static_cast<KexiProjectModelItem*>(index.internalPointer());
    if (item) {
        return item->flags();
    }
    return QAbstractItemModel::flags(index);
}

void KexiProjectModel::clear()
{
    beginResetModel();
    delete(d->rootItem);
    d->rootItem = 0;
    endResetModel();
}

QString KexiProjectModel::itemsPartClass() const
{
    return d->itemsPartClass;
}

KexiProjectModelItem *KexiProjectModel::addGroup(KexiPart::Info *info,
                                                 KexiProjectModelItem *parent) const
{
    if (!info->isVisibleInNavigator())
        return 0;

    KexiProjectModelItem *item = new KexiProjectModelItem(info, parent);
    return item;
}

void KexiProjectModel::slotAddItem(KexiPart::Item *item)
{
    //qDebug() << item.name();
    QModelIndex idx;

    KexiProjectModelItem *parent = modelItemFromName(item->pluginId());

    if (parent) {
        //qDebug() << "Got Parent" << parent->data(0);
        idx = indexFromItem(parent);
        beginInsertRows(idx, 0,0);
        addItem(parent->partInfo(), item, parent);
        parent->sortChildren();
        endInsertRows();
    }
    else {
        //qDebug() << "Unable to find parent item!";
    }
}

KexiProjectModelItem* KexiProjectModel::addItem(KexiPart::Info *info, KexiPart::Item *item,
                                                KexiProjectModelItem *parent) const
{
    d->objectsCount++;
    KexiProjectModelItem *i = new KexiProjectModelItem(info, item, parent);
    parent->appendChild(i);
    return i;
}

void KexiProjectModel::slotRemoveItem(const KexiPart::Item& item)
{
    QModelIndex idx;
    KexiProjectModelItem *mitm = modelItemFromItem(item);
    KexiProjectModelItem *parent =0;

    if (mitm) {
        //qDebug() << "Got model item from item";
        parent = mitm->parent();
    } else {
        //qDebug() << "Unable to get model item from item";
    }

    if (parent) {
        idx = indexFromItem(parent);
        beginRemoveRows(idx, 0,0);
        parent->removeChild(item);
        d->objectsCount--;
        endRemoveRows();
    } else {
        //qDebug() << "Unable to find parent item!";
    }
}

QModelIndex KexiProjectModel::indexFromItem(KexiProjectModelItem* item) const
{
    //qDebug();
    if (item) {
        int row = item->parent() ? item->row() : 0;
        //qDebug() << row;
        return createIndex(row, 0, (void*)item);
    }
    return QModelIndex();
}

KexiProjectModelItem* KexiProjectModel::modelItemFromItem(const KexiPart::Item& item) const
{
    return d->rootItem->modelItemFromItem(item);
}

KexiProjectModelItem* KexiProjectModel::modelItemFromName(const QString& name) const
{
    //qDebug() << name;
    return d->rootItem->modelItemFromName(name);
}

void KexiProjectModel::updateItemName(KexiPart::Item& item, bool dirty)
{
    //qDebug();
    KexiProjectModelItem *bitem = modelItemFromItem(item);
    if (!bitem)
        return;

    QModelIndex idx = indexFromItem(bitem);
    bitem->setDirty(dirty);
    emit dataChanged(idx, idx);
}

QModelIndex KexiProjectModel::firstChildPartItem(const QModelIndex &parentIndex) const
{
    int count = rowCount(parentIndex);
    //qDebug() << "parent:" << data(parentIndex) << parentIndex.isValid() << count;
    KexiProjectModelItem *it = static_cast<KexiProjectModelItem*>(parentIndex.internalPointer());
    if (it) {
        if (it->partItem()) {
            return parentIndex;
        }
    }
    for (int i = 0; i < count; i++) {
        QModelIndex index = parentIndex.child(i, 0);
        //qDebug() << data(index);
        index = firstChildPartItem(index);
        if (index.isValid()) {
            return index;
        }
    }
    return QModelIndex();
}

QModelIndex KexiProjectModel::firstPartItem() const
{
    return firstChildPartItem(indexFromItem(d->rootItem));
}

// Implemented for KexiSearchableModel:

int KexiProjectModel::searchableObjectCount() const
{
    const QModelIndex rootIndex = indexFromItem(d->rootItem);
    const int topLevelCount = rowCount(rootIndex);
    int result = 0;
    for (int i = 0; i < topLevelCount; i++) {
        QModelIndex index = this->index(i, 0, rootIndex);
        result += rowCount(index);
    }
    return result;
}

QModelIndex KexiProjectModel::sourceIndexForSearchableObject(int objectIndex) const
{
    const QModelIndex rootIndex = indexFromItem(d->rootItem);
    const int topLevelCount = rowCount(rootIndex);
    int j = objectIndex;
    for (int i = 0; i < topLevelCount; i++) {
        QModelIndex index = this->index(i, 0, rootIndex);
        const int childCount = rowCount(index);
        if (j < childCount) {
            return this->index(j, 0, index);
        }
        j -= childCount;
    }
    return QModelIndex();
}

QVariant KexiProjectModel::searchableData(const QModelIndex &sourceIndex, int role) const
{
    return data(sourceIndex, role);
}

QString KexiProjectModel::pathFromIndex(const QModelIndex &sourceIndex) const
{
    KexiProjectModelItem *it = static_cast<KexiProjectModelItem*>(sourceIndex.internalPointer());
    return it->partItem()->name();
}

QPersistentModelIndex KexiProjectModel::itemWithSearchHighlight() const
{
    return d->searchHighlight;
}

bool KexiProjectModel::highlightSearchableObject(const QModelIndex &index)
{
    if (d->searchHighlight.isValid() && index != d->searchHighlight) {
        setData(d->searchHighlight, false, SearchHighlight);
    }
    setData(index, true, SearchHighlight);
    emit highlightSearchedItem(index);
    d->searchHighlight = QPersistentModelIndex(index);
    return true;
}

bool KexiProjectModel::activateSearchableObject(const QModelIndex &index)
{
    if (d->searchHighlight.isValid() && index != d->searchHighlight) {
        setData(d->searchHighlight, false, SearchHighlight);
    }
    emit activateSearchedItem(index);
    return true;
}

int KexiProjectModel::objectsCount() const
{
    return d->objectsCount;
}
