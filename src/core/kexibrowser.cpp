/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Lucijan Busch <lucijan@gmx.at>

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

#include <qlayout.h>
#include <qheader.h>
#include <qpoint.h>
#include <qlistview.h>
#include <qdom.h>

#include <klocale.h>
#include <kpushbutton.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>

#include "kexiDB/kexidb.h"

#include "kexiview.h"
#include "kexibrowser.h"
#include "kexiworkspace.h"
#include "kexibrowseritem.h"
#include "kexiproject.h"
#include "kexiprojecthandler.h"
#include "kexiprojecthandlerproxy.h"
#include "kexiprojecthandleritem.h"
#include "kexihandlerpopupmenu.h"
#include "kexidialogbase.h"

KexiBrowser::KexiBrowser(QWidget *parent, QString mime, KexiProjectHandler *part, const char *name )
 : KListView(parent,name)
{
//	iconLoader = KGlobal::iconLoader();
	m_mime = mime;
	m_part = part;

	header()->hide();

	addColumn("");
	setShowToolTips(true);
	setRootIsDecorated(true);
	setSorting(0);
	sort();
//	setResizeMode(QListView::LastColumn);

	connect(this, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
		SLOT(slotContextMenu(KListView*, QListViewItem *, const QPoint&)));
	connect(this, SIGNAL(executed(QListViewItem*)), SLOT(slotExecuteItem(QListViewItem*)));

	if(part)
	{
		connect(part, SIGNAL(itemListChanged(KexiProjectHandler *)), this, SLOT(slotItemListChanged(KexiProjectHandler *)));
		slotItemListChanged(part);
	}

}

void
KexiBrowser::addGroup(KexiProjectHandler *part)
{
	//A littlebit hacky at the moment
	KexiBrowserItem *item = new KexiBrowserItem(this,
		part->proxy(static_cast<KexiDialogBase*>(parent()->parent())->kexiView())
	);

	item->setPixmap(0, part->groupPixmap());
	item->setOpen(true);
	item->setSelectable( false );
	m_baseItems.insert(part->mime(), item);
	slotItemListChanged(part);
}

void
KexiBrowser::addItem(KexiProjectHandlerItem *item)
{
	if(m_mime == "kexi/db" && m_baseItems.find(item->mime()))
	{
		//part object
		KexiBrowserItem *parent = m_baseItems.find(item->mime());
		KexiBrowserItem *bitem = new KexiBrowserItem(parent, item);
		bitem->setPixmap(0, parent->proxy()->part()->itemPixmap());
	}
	else if(m_mime == item->mime())
	{
		//part objects group
		KexiBrowserItem *bitem = new KexiBrowserItem(this, item);
		if(m_part)
			bitem->setPixmap(0, m_part->itemPixmap());
	}
}

void
KexiBrowser::slotItemListChanged(KexiProjectHandler *parent)
{

	kdDebug() << "KexiBrowser::slotItemListChanged() " << parent->items()->count() << " items" << endl;
	kdDebug() << "KexiBrowser::slotItemListChanged() " << m_mime << endl;

	if(m_mime == "kexi/db")
	{
		KexiBrowserItem *group = m_baseItems.find(parent->mime());
		group->clearChildren();
	}
	else
	{
		clear();
	}


	for(KexiProjectHandler::ItemIterator it(*(parent->items()));it.current();++it)
	//KexiProjectHandlerItem *item = plist->first(); item; item = plist->next())
	{
		kdDebug() << "KexiBrowser::slotItemListChanged() adding " << it.current()->mime() << endl;
		kdDebug() << "KexiBrowser::slotItemListChanged()  named " << it.current()->fullIdentifier() << endl;
		addItem(it.current());
	}
}

void
KexiBrowser::slotContextMenu(KListView *, QListViewItem *item, const QPoint &pos)
{
	KexiBrowserItem *it = static_cast<KexiBrowserItem*>(item);
	if(!it)
		return;

	if(it->proxy() || it->item())
	{
		KexiPartPopupMenu *pg = 0;
		if(it->identifier().isNull())
		{
			// FIXME: Make this less hacky please :)
			pg = it->proxy()->groupContextMenu();
		}
		else
		{
			kdDebug() << "KexiBrowser::slotContextMenu() item @ " << it->item() << endl;
			//a littlebit hacky
			pg = it->item()->handler()->proxy(
			static_cast<KexiDialogBase*>(parent()->parent())->kexiView()
			)->itemContextMenu(it->identifier());
		}

		pg->setPartItemId(it->identifier());
		pg->exec(pos);
//		delete pg;
	}
}

void
KexiBrowser::slotExecuteItem(QListViewItem *item)
{
	KexiBrowserItem *it = static_cast<KexiBrowserItem*>(item);
	if(!it)
		return;

	if(it->proxy() || it->item())
	{
		if(!it->identifier().isNull())
		{
			if (it->proxy())
			{
				it->proxy()->executeItem(it->identifier());
			}
			else
			{
				it->item()->handler()->proxy(
					static_cast<KexiDialogBase*>(parent()->parent())->kexiView()
				)->executeItem(it->item()->fullIdentifier());
			}
		}
	}
}

#include "kexibrowser.moc"
