/***************************************************************************
                          kexibrowser.cpp  -  description
                             -------------------
    begin                : Sun Jun 9 2002
    copyright            : (C) 2002 by lucijan busch
    email                : lucijan@gmx.at
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#include "kexiformbase.h"
#include "kexiworkspace.h"
#include "kexibrowseritem.h"
#include "kexidatatable.h"
#include "kexialtertable.h"
#include "kexiquerydesigner.h"
//#include "kexikugarwrapper.h"
#include "kexiproject.h"
#include "kexiprojectpart.h"
#include "kexiprojectpartitem.h"
#include "kexipartpopupmenu.h"

KexiBrowser::KexiBrowser(QWidget *parent, QString mime, KexiProjectPart *part, const char *name )
 : KListView(parent,name)
{
//	iconLoader = KGlobal::iconLoader();
	m_mime = mime;

	header()->hide();

	addColumn("");
	setResizeMode(QListView::LastColumn);

	connect(this, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
		SLOT(slotContextMenu(KListView*, QListViewItem *, const QPoint&)));

	if(part)
	{
		connect(part, SIGNAL(itemListChanged(KexiProjectPart *)), this, SLOT(slotItemListChanged(KexiProjectPart *)));
		
//		if(mime != "kexi/db")
//		{
			slotItemListChanged(part);
//		}
	}

}

void
KexiBrowser::addGroup(KexiProjectPart *part)
{
	KexiBrowserItem *item = new KexiBrowserItem(this, part);
	
	item->setPixmap(0, part->groupPixmap());
	m_baseItems.insert(part->mime(), item);
	slotItemListChanged(part);
}

void
KexiBrowser::addItem(KexiProjectPartItem *item)
{
	kdDebug() << "KexiBrowser::addItem() looking for " << item->mime() << endl;
	if(m_mime == "kexi/db" && m_baseItems.find(item->mime()))
	{
		KexiBrowserItem *parent = m_baseItems.find(item->mime());
		kdDebug() << "KexiBrowser::addItem() found " << item->mime() << " @ " << parent << endl;	
		KexiBrowserItem *iview = new KexiBrowserItem(parent, item);
		/*parent->*/insertItem(iview);
	
	}
	else if(m_baseItems.find(item->mime()))
	{
		kdDebug() << "KexiBrowser::addItem() adding as parent" << endl;
		KexiBrowserItem *iview = new KexiBrowserItem(this, item);
	}
}

void
KexiBrowser::slotItemListChanged(KexiProjectPart *parent)
{
	ItemList *plist = parent->items();

	kdDebug() << "KexiBrowser::slotItemListChanged() " << plist->count() << " items" << endl;

	
	for(KexiProjectPartItem *item = plist->first(); item; item = plist->next())
	{
		kdDebug() << "KexiBrowser::slotItemListChanged() adding " << item->mime() << endl;
		addItem(item);
	}
}

void
KexiBrowser::slotContextMenu(KListView *, QListViewItem *item, const QPoint &pos)
{
	KexiBrowserItem *it = static_cast<KexiBrowserItem*>(item);
	if(!it)
		return;

	if(it->part() || it->item())
	{
		KexiPartPopupMenu *pg = 0;
		if(it->identifier() == QString::null)
		{
			pg = it->part()->groupContext();
		}
		else
		{
			kdDebug() << "KexiBrowser::slotContextMenu() item @ " << it->item() << endl;
			pg = it->part()->itemContext();
		}
	
		pg->setIdentifier(it->identifier());
		pg->exec(pos);
	}
}

KexiBrowser::~KexiBrowser()
{
}

#include "kexibrowser.moc"
