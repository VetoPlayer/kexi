/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

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

#include <qpixmap.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include <qdom.h>

#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kcombobox.h>
#include <kicondialog.h>
#include <klineedit.h>
#include <kicontheme.h>
#include <kpixmapio.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "pixmapcollection.h"

/// Pixmap Collection
PixmapCollection::PixmapCollection(const QString &collectionName, QObject *parent, const char *name)
 : QObject(parent, name)
{
	m_name = collectionName;
}

QString
PixmapCollection::addPixmapPath(const KURL &url)
{
	QString name = url.filename();
	while(m_pixmaps.contains(name))
	{
		bool ok;
		int num = name.right(1).toInt(&ok, 10);
		if(ok)
			name = name.left(name.length()-1) + QString::number(num+1);
		else
			name += "2";
	}

	m_pixmaps.insert(name, qMakePair(url.path(), 0));
	return name;
}

QString
PixmapCollection::addPixmapName(const QString &icon, int size)
{
	QString name = icon;
	while(m_pixmaps.contains(name))
	{
		bool ok;
		int num = name.right(1).toInt(&ok, 10);
		if(ok)
			name = name.left(name.length()-1) + QString::number(num+1);
		else
			name += "2";
	}

	m_pixmaps.insert(name, qMakePair(icon, size));
	return name;
}

void
PixmapCollection::removePixmap(const QString &name)
{
	m_pixmaps.remove(name);
}

QPixmap
PixmapCollection::getPixmap(const QString &name)
{
	if(!m_pixmaps.contains(name))
	{
		kdDebug() << " The icon " << name << " you requested is not in the collection" << endl;
		return QPixmap();
	}

	if(m_pixmaps[name].second != 0)
	{
		return kapp->iconLoader()->loadIcon(m_pixmaps[name].first, KIcon::NoGroup, m_pixmaps[name].second);
	}
	else
		return QPixmap(m_pixmaps[name].first);
}

bool
PixmapCollection::contains(const QString &name)
{
	return m_pixmaps.contains(name);
}

void
PixmapCollection::save(QDomNode parentNode)
{
	if(m_pixmaps.isEmpty())
		return;

	QDomDocument domDoc = parentNode.ownerDocument();
	QDomElement collection = domDoc.createElement("collection");
	parentNode.appendChild(collection);

	PixmapMap::Iterator it;
	PixmapMap::Iterator endIt = m_pixmaps.end();
	for(it = m_pixmaps.begin(); it != endIt; ++it)
	{
		QDomElement item = domDoc.createElement("pixmap");
		collection.appendChild(item);
		item.setAttribute("name", it.key());
		if(it.data().second != 0)
			item.setAttribute("size", QString::number(it.data().second));

		QString text = it.data().first;
		QDomText textNode = domDoc.createTextNode(text);
		item.appendChild(textNode);
	}
}

void
PixmapCollection::load(QDomNode node)
{
	QDomDocument domDoc = node.ownerDocument();
	for(QDomNode n = node.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement el = n.toElement();
		QPair<QString, int> pair = qMakePair(el.text(), el.attribute("size").toInt());
		m_pixmaps[el.attribute("name")] = pair;
	}
}

//// A dialog to load a KDE icon by its name
LoadIconDialog::LoadIconDialog(QWidget *parent)
: KDialogBase(parent, "loadicon_dialog", true, i18n("Load KDE Icon by Name"), Ok|Cancel, Ok, false)
{
	QFrame *frame = makeMainWidget();
	QGridLayout *l = new QGridLayout(frame, 2, 3, 0, 6);

	// Name input
	QLabel *name = new QLabel(i18n("&Name:"), frame);
	l->addWidget(name, 0, 0);
	name->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_nameInput = new KLineEdit("kexi", frame);
	l->addWidget(m_nameInput, 0, 1);
	name->setBuddy(m_nameInput);

	// Choose size
	QLabel *size = new QLabel(i18n("&Size:"), frame);
	l->addWidget(size, 1, 0);
	size->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	KComboBox *combo = new KComboBox(frame);
	l->addWidget(combo, 1, 1);
	size->setBuddy(combo);
	QStringList list;
	list << i18n("Small") << i18n("Medium") << i18n("Large") << i18n("Huge");
	combo->insertStringList(list);
	combo->setCurrentItem(2);
	connect(combo, SIGNAL(activated(int)), this, SLOT(changeIconSize(int)));


	// Icon chooser button
	m_button = new KIconButton(frame);
	m_button->setIcon("kexi");
	m_button->setIconSize(KIcon::SizeMedium);
	l->addMultiCellWidget(m_button, 0, 1, 2, 2);
	connect(m_button, SIGNAL(iconChanged(QString)), this, SLOT(updateIconName(QString)));
	connect(m_nameInput, SIGNAL(textChanged(const QString &)), this, SLOT(setIcon(const QString &)));
}

void
LoadIconDialog::updateIconName(QString icon)
{
	m_nameInput->setText(icon);
}

void
LoadIconDialog::setIcon(const QString &icon)
{
	m_button->setIcon(icon);
}

void
LoadIconDialog::changeIconSize(int index)
{
	int size = KIcon::SizeMedium;
	switch(index)
	{
		case 0: size = KIcon::SizeSmall; break;
		//case 1: size = KIcon::SizeSmallMedium; break;
		case 1: size = KIcon::SizeMedium; break;
		case 2: size = KIcon::SizeLarge; break;
#if !defined(Q_WS_WIN) && KDE_IS_VERSION(3,1,9)
		case 3: size = KIcon::SizeHuge; break;
#endif
		default:;
	}

	m_button->setIconSize(size);
}

int LoadIconDialog::iconSize()
{
	return m_button->iconSize();
}

QString LoadIconDialog::iconName()
{
	return m_button->icon();
}

/// Pixmap Collection Editor Dialog
PixmapCollectionEditor::PixmapCollectionEditor(PixmapCollection *collection, QWidget *parent)
: KDialogBase(parent, "pixcollection_dialog", true,
	i18n("Edit Pixmap Collection: %1").arg(collection->collectionName()), Close, Close, false)
{
	m_collection = collection;
	QFrame *frame = makeMainWidget();
	QHBoxLayout *l = new QHBoxLayout(frame, 0, 6);
	setInitialSize(QSize(400, 200), true);

	//// Setup the icon toolbar /////////////////
	QVBoxLayout *vlayout = new QVBoxLayout(l, 3);
	QToolButton *newItemPath = new QToolButton(frame);
	newItemPath->setIconSet(BarIconSet("fileopen"));
	newItemPath->setTextLabel(i18n("&Add File"), true);
	vlayout->addWidget(newItemPath);
	m_buttons.insert(BNewItemPath, newItemPath);
	connect(newItemPath, SIGNAL(clicked()), this, SLOT(newItemByPath()));

	QToolButton *newItemName = new QToolButton(frame);
	newItemName->setIconSet(BarIconSet("icons"));
	newItemName->setTextLabel(i18n("&Add an icon"), true);
	vlayout->addWidget(newItemName);
	m_buttons.insert(BNewItemName, newItemName);
	connect(newItemName, SIGNAL(clicked()), this, SLOT(newItemByName()));

	QToolButton *delItem = new QToolButton(frame);
	delItem->setIconSet(BarIconSet("edit_remove"));
	delItem->setTextLabel(i18n("&Remove selected item"), true);
	vlayout->addWidget(delItem);
	m_buttons.insert(BDelItem, delItem);
	connect(delItem, SIGNAL(clicked()), this, SLOT(removeItem()));
	vlayout->addStretch();

	// Setup the iconView
	m_iconView = new KIconView(frame, "pixcollection_iconView");
	m_iconView->resize(100,100);
	m_iconView->setArrangement(QIconView::LeftToRight);
	m_iconView->setAutoArrange(true);
	m_iconView->setMode(KIconView::Select);
	l->addWidget(m_iconView);
	connect(m_iconView, SIGNAL(contextMenuRequested(QIconViewItem*, const QPoint&)), this, SLOT(displayMenu(QIconViewItem*, const QPoint&)));
	connect(m_iconView, SIGNAL(itemRenamed(QIconViewItem*, const QString &)), this, SLOT(renameCollectionItem(QIconViewItem*, const QString&)));

	PixmapMap::Iterator it;
	PixmapMap::Iterator endIt = collection->m_pixmaps.end();
	for(it = collection->m_pixmaps.begin(); it != endIt; ++it)
		createIconViewItem(it.key());
}

void
PixmapCollectionEditor::newItemByName()
{
	LoadIconDialog d(parentWidget());
	if(d.exec()== QDialog::Accepted)
	{
		if(d.iconName().isEmpty())
			return;

		QString name = m_collection->addPixmapName(d.iconName(), d.iconSize());
		createIconViewItem(name);
	}
}

void
PixmapCollectionEditor::newItemByPath()
{
	KURL url = KFileDialog::getImageOpenURL("::kexi", parentWidget());
	if(url.isEmpty())
		return;
	QString name = m_collection->addPixmapPath(url);
	createIconViewItem(name);
}

void
PixmapCollectionEditor::removeItem()
{
	QIconViewItem *item = m_iconView->currentItem();

	int confirm = KMessageBox::questionYesNo(parentWidget(), QString("<qt>")+
		i18n("Do you want to remove item \"%1\" from collection \"%2\"?")
		.arg(item->text()).arg(m_collection->collectionName()) + "</qt>");
	if(confirm == KMessageBox::No)
		return;

	m_collection->removePixmap(item->text());
	delete item;
}

void
PixmapCollectionEditor::renameItem()
{
	m_iconView->currentItem()->rename();
}

void
PixmapCollectionEditor::createIconViewItem(const QString &name)
{
	PixmapIconViewItem *item = new PixmapIconViewItem(m_iconView, name, getPixmap(name));
	item->setRenameEnabled(true);
}

QPixmap
PixmapCollectionEditor::getPixmap(const QString &name)
{
	QPixmap pixmap = m_collection->getPixmap(name);
	if((pixmap.width() <= 48) && (pixmap.height() <= 48))
		return pixmap;

	KPixmapIO io;
	QImage image = io.convertToImage(pixmap);
	pixmap = io.convertToPixmap(image.scale(48, 48, QImage::ScaleMin));
	return pixmap;
}

void
PixmapCollectionEditor::renameCollectionItem(QIconViewItem *it, const QString &name)
{
	PixmapIconViewItem *item = static_cast<PixmapIconViewItem*>(it);
	if(!m_collection->m_pixmaps.contains(item->name()))
		return;

	// We just rename the collection item
	QPair<QString, int> pair = m_collection->m_pixmaps[item->name()];
	m_collection->m_pixmaps.remove(item->name());
	m_collection->m_pixmaps[name] = pair;
	item->setName(name);
}

void
PixmapCollectionEditor::displayMenu(QIconViewItem *it, const QPoint &p)
{
	if(!it) return;
	KPopupMenu *menu = new KPopupMenu();
	menu->insertItem(SmallIconSet("edit"), i18n("Rename Item"), this, SLOT(renameItem()));
	menu->insertItem(SmallIconSet("remove"), i18n("Remove Item"), this, SLOT(removeItem()));
	menu->exec(p);
}

//// A Dialog to choose a pixmap from the PixmapCollection
PixmapCollectionChooser::PixmapCollectionChooser(PixmapCollection *collection, const QString &selectedItem, QWidget *parent)
: KDialogBase(parent, "pixchoose_dialog", true, i18n("Select a pixmap from %1").arg(collection->collectionName()),
   User1|Ok|Cancel, Ok, false, KGuiItem(i18n("Edit Collection...")))
{
	m_collection = collection;
	setInitialSize(QSize(400, 200), true);

	m_iconView = new KIconView(this, "pixchooser_iconView");
	setMainWidget(m_iconView);
	m_iconView->setArrangement(QIconView::LeftToRight);
	m_iconView->setAutoArrange(true);
	m_iconView->setMode(KIconView::Select);

	PixmapMap::Iterator it;
	PixmapMap::Iterator endIt = collection->m_pixmaps.end();
	for(it = collection->m_pixmaps.begin(); it != endIt; ++it)
		new PixmapIconViewItem(m_iconView, it.key(), getPixmap(it.key()));

	QIconViewItem *item = m_iconView->findItem(selectedItem, Qt::ExactMatch);
	if(item && !selectedItem.isEmpty())
		m_iconView->setCurrentItem(item);
}

QPixmap
PixmapCollectionChooser::pixmap()
{
	QString name = m_iconView->currentItem()->text();
	return m_collection->getPixmap(name);
}

QString
PixmapCollectionChooser::pixmapName()
{
	return m_iconView->currentItem()->text();
}

QPixmap
PixmapCollectionChooser::getPixmap(const QString &name)
{
	QPixmap pixmap = m_collection->getPixmap(name);
	if((pixmap.width() <= 48) && (pixmap.height() <= 48))
		return pixmap;

	// We scale the pixmap down to 48x48 to fit in the iconView
	KPixmapIO io;
	QImage image = io.convertToImage(pixmap);
	pixmap = io.convertToPixmap(image.scale(48, 48, QImage::ScaleMin));
	return pixmap;
}

void
PixmapCollectionChooser::slotUser1()
{
	PixmapCollectionEditor dialog(m_collection, parentWidget());
	dialog.exec();

	m_iconView->clear();
	PixmapMap::Iterator it;
	PixmapMap::Iterator endIt = m_collection->m_pixmaps.end();
	for(it = m_collection->m_pixmaps.begin(); it != endIt; ++it)
		 new PixmapIconViewItem(m_iconView, it.key(), getPixmap(it.key()));
}

#include "pixmapcollection.moc"
