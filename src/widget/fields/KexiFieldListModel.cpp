/*
    This file is part of the KDE project
    Copyright (C) 2010 Adam Pigg <adam@piggz.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "KexiFieldListModel.h"
#include "KexiFieldListModelItem.h"

#include <KDbTableSchema>
#include <KDbQuerySchema>
#include <KDbTableOrQuerySchema>
#include <KDbUtils>

#include <KLocalizedString>

#include <QDebug>
#include <QMimeData>

class Q_DECL_HIDDEN KexiFieldListModel::Private
{
public:
    Private();
    ~Private();
    KDbTableOrQuerySchema* schema;
    KexiFieldListOptions options;
    KexiFieldListModelItem *allColumnsItem;
    QList<KexiFieldListModelItem*> items;
};

KexiFieldListModel::Private::Private() : schema(0), allColumnsItem(0)
{

}

KexiFieldListModel::Private::~Private()
{
    qDeleteAll(items);
}

KexiFieldListModel::KexiFieldListModel(QObject* parent, KexiFieldListOptions options): QAbstractTableModel(parent)
                                      , d(new Private())
{
    d->options = options;
}

KexiFieldListModel::~KexiFieldListModel()
{
    delete d;
}

void KexiFieldListModel::setSchema(KDbConnection *conn, KDbTableOrQuerySchema* schema)
{
    if (schema && d->schema == schema)
        return;

    delete d->schema;
    d->schema = schema;
    if (!d->schema)
        return;

    qDeleteAll(d->items);
    d->items.clear();
    KexiFieldListModelItem *item = 0;
    KDbQueryColumnInfo::Vector columns = d->schema->columns(conn, KDbTableOrQuerySchema::ColumnsMode::Unique);
    const int count = columns.count();
    for (int i = -2; i < count; i++) {
        KDbQueryColumnInfo *colinfo = 0;
        if (i == -2) {
            if (!(d->options & ShowEmptyItem))
                continue;
            item = new KexiFieldListModelItem(QString(), QString(), false);
        } else if (i == -1) {
            if (!(d->options & ShowAsterisk))
                continue;
            item = new KexiFieldListModelItem("*", "", false);
            d->allColumnsItem = item;
        } else {
            colinfo = columns[i];
            item = new KexiFieldListModelItem(
                        colinfo->aliasOrName(), colinfo->field()->typeName(),
                        (colinfo->field()->isPrimaryKey() || colinfo->field()->isUniqueKey()));
            item->setCaption(colinfo->captionOrAliasOrName());
        }
        d->items.append(item);
        //qDebug() << item->data(0);
    }
}

QVariant KexiFieldListModel::data(const QModelIndex& index, int role) const
{
    KexiFieldListModelItem *item = 0;

    if (index.isValid() && index.row() < d->items.count()) {
        item = d->items[index.row()];
    }

    if (item) {
        if (role == Qt::DisplayRole) {
            return item->data(index.column());
        } else if (role == Qt::DecorationRole) {
            if (index.column() == 0) { // icon for field name only
                return item->icon();
            } else {
                return QVariant();
            }
        } else if (role == Qt::UserRole) {
            //qDebug() << item->caption();
            return item->caption();
        }
    }
    return QVariant();
}

int KexiFieldListModel::columnCount(const QModelIndex& /*parent*/) const
{
    return (d->options & ShowDataTypes) ? 2 : 1;
}

int KexiFieldListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return d->items.count();
}

QVariant KexiFieldListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section == 0) {
            return xi18n("Field Name");
        } else if (section == 1) {
            return xi18n("Data Type");
        }
    }
    return QVariant();
}

QStringList KexiFieldListModel::mimeTypes() const
{
    QStringList types;
    types << "kexi/fields";
    return types;
}

QMimeData* KexiFieldListModel::mimeData(const QModelIndexList& indexes) const
{
    if (!d->schema) {
        return new QMimeData();
    }

    QString sourceMimeType;
    QString sourceName;
    QStringList fields;

    QMimeData *mimedata = new QMimeData();
    QByteArray fielddata;
    QDataStream stream1(&fielddata, QIODevice::WriteOnly);

    if (d->schema->table()) {
        sourceMimeType = "kexi/table";
    } else if (d->schema->query()) {
        sourceMimeType = "kexi/query";
    }

    sourceName = d->schema->name();

    foreach (const QModelIndex &idx, indexes) {
        fields << data(idx, Qt::DisplayRole).toString();
    }

    stream1 << sourceMimeType << sourceName << fields;
    mimedata->setData("kexi/fields", fielddata);

    return mimedata;
}

Qt::ItemFlags KexiFieldListModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (index.isValid())
        return d->items[index.row()]->flags()| defaultFlags;
    else
        return defaultFlags;
}
