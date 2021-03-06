/* This file is part of the KDE project
   Copyright (C) 2009 Adam Pigg <adam@piggz.co.uk>

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

#include "AlterSchemaWidget.h"
#include <KexiIcon.h>
#include <KexiMainWindowIface.h>
#include <kexiproject.h>
#include <KexiWindow.h>
#include <widget/KexiNameWidget.h>

#include <KDbConnection>

#include <KLocalizedString>

#include <QGridLayout>
#include <QTableView>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

using namespace KexiMigration;

AlterSchemaWidget::AlterSchemaWidget(QWidget *parent) : QWidget(parent)
{
    m_schema = 0;
    m_selectedColumn = -1;

    m_layout = new QGridLayout();
    m_table = new QTableView(this);
    m_columnType = new QComboBox(this);
    m_columnPKey = new QCheckBox(this);
    m_tableNameWidget = new KexiNameWidget("",this);

    m_columnNumLabel = new QLabel(xi18n("Column %1", 1), this);
    m_columnTypeLabel = new QLabel(xi18n("Type"), this);
    m_columnPKeyLabel = new QLabel(xi18n("Primary Key"), this);

    m_types = KDbField::typeNames();
    m_types.removeFirst(); //Remove InvalidTypes

    for (unsigned int i = KDbField::FirstType; i <= KDbField::LastType; ++i) {
        m_columnType->addItem(KDbField::typeName(KDb::intToFieldType(i)), i);
    }

    m_layout->addWidget(m_tableNameWidget, 0, 0, 2, 3);
    m_layout->addWidget(m_columnNumLabel, 2, 0, 1, 3);
    m_layout->addWidget(m_columnTypeLabel, 3, 0, 1, 1);
    m_layout->addWidget(m_columnPKeyLabel, 3, 1, 1, 2);
    m_layout->addWidget(m_columnType, 4, 0, 1, 1);
    m_layout->addWidget(m_columnPKey, 4, 1, 1, 2);
    m_layout->addWidget(m_table, 5, 0, 1, 3);

    setLayout(m_layout);

    connect(m_table, SIGNAL(clicked(QModelIndex)), this, SLOT(tableClicked(QModelIndex)));
    connect(m_columnType, SIGNAL(activated(int)), this, SLOT(typeActivated(int)));
    connect(m_columnPKey, SIGNAL(clicked(bool)), this, SLOT(pkeyClicked(bool)));

    m_model = new AlterSchemaTableModel();
    m_table->setModel(m_model);
}

AlterSchemaWidget::~AlterSchemaWidget()
{
    delete m_table;
    delete m_model;
    delete m_schema;
}

void AlterSchemaWidget::setTableSchema(KDbTableSchema* ts)
{
    if (ts == m_schema) {
        return;
    }
    m_model->setSchema(ts);
    delete m_schema;
    m_schema = ts;

    m_tableNameWidget->setCaptionText(ts->captionOrName());
    m_tableNameWidget->captionLineEdit()->selectAll();
    m_tableNameWidget->captionLineEdit()->setFocus();

    m_model->setRowCount(3); // default
    tableClicked(m_model->index(0,0));
}

void AlterSchemaWidget::setData(QList<KDbRecordData*>* data)
{
    m_model->setData(data);
}

void AlterSchemaWidget::tableClicked(const QModelIndex& idx)
{
    m_selectedColumn = idx.column();
    m_columnNumLabel->setText(xi18n("Column %1", m_selectedColumn + 1));
    if (m_schema && m_selectedColumn < int(m_schema->fieldCount()) && m_schema->field(m_selectedColumn)) {
        //qDebug() << m_schema->field(m_selectedColumn)->typeName() << m_types.indexOf(m_schema->field(m_selectedColumn)->typeName());
        m_columnType->setCurrentIndex(m_types.indexOf(m_schema->field(m_selectedColumn)->typeName()));

        //Only set the pkey check enabled if the field type is integer
        m_columnPKey->setEnabled(KDbField::isIntegerType(KDb::intToFieldType(m_columnType->itemData(m_types.indexOf(m_schema->field(m_selectedColumn)->typeName())).toInt())));

        m_columnPKey->setChecked(m_schema->field(m_selectedColumn)->isPrimaryKey());
    }
}

void AlterSchemaWidget::typeActivated(int typ)
{
    if (!m_schema) {
        return;
    }
    m_schema->field(m_selectedColumn)->setType(KDb::intToFieldType(m_columnType->itemData(typ).toInt()));

    //Only set the pkey check enabled if the field type is integer
    const bool isInteger = KDbField::isIntegerType(
                               KDb::intToFieldType(m_columnType->itemData(typ).toInt()));
    m_columnPKey->setEnabled(isInteger);

    //If the field type is not integer, then the field cannot be a pkey
    if (!isInteger) {
        m_schema->field(m_selectedColumn)->setPrimaryKey(false);
    }
}

void AlterSchemaWidget::pkeyClicked(bool pkey)
{
    if (!m_schema) {
        return;
    }
    m_schema->field(m_selectedColumn)->setAutoIncrement(pkey);
    m_schema->field(m_selectedColumn)->setPrimaryKey(pkey);
}

KDbTableSchema* AlterSchemaWidget::newSchema()
{
    return m_schema;
}

KDbTableSchema* AlterSchemaWidget::takeTableSchema()
{
    KDbTableSchema *schema = m_schema;
    m_schema = 0;
    return schema;
}


KexiNameWidget* AlterSchemaWidget::nameWidget()
{
    return m_tableNameWidget;
}

AlterSchemaTableModel* AlterSchemaWidget::model()
{
    return m_model;
}

QString AlterSchemaWidget::suggestedItemCaption(const QString& baseCaption)
{
    unsigned int n = 0;
    QString newCaption;
    do {
        newCaption = baseCaption;
        if (n >= 1) {
            newCaption = baseCaption + QString::number(n);
        }

        if (nameExists(KDb::stringToIdentifier(newCaption))) {
            n++;
            continue; //stored exists!
        } else {
            break;
        }
    } while (n < 1000/*sanity*/);

    if (n == 1000) {
        newCaption = QString(""); //unable to find a usable name
    }

    return newCaption;
}

bool AlterSchemaWidget::nameExists(const QString & name) const
{
    KexiPart::ItemDict* list
      = KexiMainWindowIface::global()->project()->itemsForPluginId("org.kexi-project.table");
    if (!list) {
        return false;
    }

    QHash<int, KexiPart::Item*>::const_iterator it = list->constBegin();
    while (it != list->constEnd()) {
        if (QString::compare(name, it.value()->name(), Qt::CaseInsensitive) == 0) {
            return true;
        }
        ++it;
    }

    return false;
}
