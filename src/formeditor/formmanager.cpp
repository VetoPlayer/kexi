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

#include <kdebug.h>

#include <qworkspace.h>
#include <qcursor.h>
#include <qstring.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpopupmenu.h>

#include "kexipropertyeditor.h"
#include "objpropbuffer.h"
#include "objecttree.h"
#include "widgetlibrary.h"
#include "form.h"
#include "container.h"
#include "formIO.h"
#include "objecttreeview.h"

#include "formmanager.h"

namespace KFormDesigner
{

FormManager::FormManager(QWidget *container, QObject *parent=0, const char *name=0)
   : QObject(parent, name)
{
	m_lib = new WidgetLibrary(this);
	m_buffer = new ObjectPropertyBuffer(this, this, "buffer");
	m_parent = container;

	m_editor = 0;
	m_active = 0;
	m_inserting = false;
	m_count = 0;

	m_domDoc.appendChild(m_domDoc.createElement("UI"));

	m_popup = new KPopupMenu();
	m_popup->insertItem(i18n("Copy"), this, SLOT(copyWidget()));
	m_popup->insertItem(i18n("Cut"), this, SLOT(cutWidget()));
	m_popup->insertItem(i18n("Paste"), this, SLOT(pasteWidget()));
	m_popup->insertItem(i18n("Remove Item"), this, SLOT(deleteWidget()));
	m_treeview = 0;
	m_editor = 0;
}

void
FormManager::setEditors(KexiPropertyEditor *editor, ObjectTreeView *treeview)
{
	m_editor = editor;
	m_treeview = treeview;

	if(editor)
		editor->setBuffer(m_buffer);

	connect(treeview, SIGNAL(selectionChanged(QWidget*)), m_buffer, SLOT(setWidget(QWidget *)));
	connect(m_treeview, SIGNAL(selectionChanged(QWidget*)), this, SLOT(setSelWidget(QWidget*)));
}

Actions
FormManager::createActions(KActionCollection *parent)
{
	return m_lib->createActions(parent, this, SLOT(insertWidget(const QString &)));
}

void
FormManager::insertWidget(const QString &classname)
{
	Form *form;
	for(form = m_forms.first(); form; form = m_forms.next())
	{
		form->toplevelContainer()->widget()->setCursor(QCursor(CrossCursor));
	}

	m_inserting = true;
	m_insertClass = classname;
}

void
FormManager::stopInsert()
{
	Form *form;
	for(form = m_forms.first(); form; form = m_forms.next())
	{
		form->toplevelContainer()->widget()->unsetCursor();
	}
	m_inserting = false;
}

void
FormManager::windowChanged(QWidget *w)
{
	Form *form;
	for(form = m_forms.first(); form; form = m_forms.next())
	{
		if(form->toplevelContainer()->widget() == w)
		{
			m_active = form;
			m_treeview->setForm(form);
			kdDebug() << "active form is " << form->objectTree()->name() << endl;
			return;
		}
	}
	//m_active = 0;
}

Form*
FormManager::activeForm() const
{
	return m_active;
}

/*
Form*
FormManager::activeForm()
{
	QWidget *wid = m_workspace->focusWidget();
	Form *form;
	for(form = m_forms.first(); form; form = m_forms.next())
	{
		if((form->toplevelContainer()->widget()->child(wid->name()))
		 || wid == form->toplevelContainer()->widget())
		 	kdDebug() << "active widget is " << form->objectTree()->name() << endl;
	}

	QWidget *w = m_workspace->activeWindow();

	for(form = m_forms.first(); form; form = m_forms.next())
	{
		if(form->toplevelContainer()->widget() == w)
		{
			m_active = form;
			return form;
		}
	}
	m_active = m_forms.first();
	return m_forms.first();
}*/

void
FormManager::deleteForm(Form *form)
{
	kdDebug() << "removing form " << form->objectTree()->name() << endl;
	m_forms.remove(form);
}

void
FormManager::setSelWidget(QWidget *w)
{
	if(activeForm())
		activeForm()->setSelWidget(w);
}
/*
void
FormManager::updateTreeView(QWidget *w)
{
	if(!m_treeview)
		return;

	Form *form;
	for(form = m_forms.first(); form; form = m_forms.next())
	{
		if(form->toplevelContainer()->widget() == w)
		{
			m_treeview->setForm(form);
			return;
		}
	}
}*/

void
FormManager::createBlankForm()
{
	createBlankForm("QWidget",0);
}

QWidget *
FormManager::createBlankForm(const QString &classname, const char *name, QWidget *parent)
{
	if(!parent)
		parent = m_parent;

	Form *form = new Form(this, name);

	QWidget *w=0;
	QString n;

	n = "Form" + QString::number(m_count + 1);
	w = new QWidget(parent, n.latin1());

	form->createToplevel(w, classname);
	w->setCaption(n);
	w->setIcon(SmallIcon("kexi"));
	w->resize(350, 300);
	w->show();
	w->setFocus();
	initForm(form);

	return 0;
}

void
FormManager::importForm(QWidget *w)
{
	Form *form = new Form(this, w->name());
	//form->createToplevel(w, w->className());
	form->createToplevel(w, w->name());
	w->setCaption(w->name());
	w->setIcon(SmallIcon("kexi"));
	w->resize(350, 300);
	w->show();
	w->setFocus();

	initForm(form);
}

void
FormManager::loadForm()
{
//	QString n = "Form" + QString::number(m_count + 1);
	Form *form = new Form(this);//, n.latin1());
	QWidget *w = new QWidget(m_parent);
	form->createToplevel(w);
	if(!FormIO::loadForm(form, w))
	{
		delete form;
		return;
	}
	w->show();
	initForm(form);
}

void
FormManager::initForm(Form *form)
{
	m_forms.append(form);

	if(m_treeview)
		m_treeview->setForm(form);

	m_active = form;
	m_count++;

	m_buffer->setWidget(form->toplevelContainer()->widget());

	connect(form, SIGNAL(selectionChanged(QWidget*)), m_buffer, SLOT(setWidget(QWidget*)));
	connect(form, SIGNAL(addedSelectedWidget(QWidget*)), m_buffer, SLOT(addWidget(QWidget*)));
	connect(form, SIGNAL(selectionChanged(QWidget*)), m_treeview, SLOT(setSelWidget(QWidget*)));
	connect(form, SIGNAL(childAdded(ObjectTreeItem* )), m_treeview, SLOT(addItem(ObjectTreeItem*)));
	connect(form, SIGNAL(childRemoved(ObjectTreeItem* )), m_treeview, SLOT(removeItem(ObjectTreeItem*)));
	connect(m_buffer, SIGNAL(nameChanged(const QString&, const QString&)), form, SLOT(changeName(const QString&, const QString&)));
	connect(m_treeview, SIGNAL(selectionChanged(QWidget*)), m_buffer, SLOT(setWidget(QWidget*)));
}


void
FormManager::saveForm()
{
	m_buffer->checkModifiedProp();
	if (activeForm())
	{
		if(!activeForm()->filename().isNull())
			FormIO::saveForm(activeForm(), activeForm()->filename());
		else
			FormIO::saveForm(activeForm());
	}
}

void
FormManager::saveFormAs()
{
	m_buffer->checkModifiedProp();
	if (activeForm())
		FormIO::saveForm(activeForm());
}

void
FormManager::previewForm(Form *form, QWidget *container)
{
	QDomDocument domDoc;
	FormIO::saveFormToDom(form, domDoc);

	Form *myform = new Form(this, form->objectTree()->name().latin1());
	myform->createToplevel(container);
	FormIO::loadFormFromDom(myform, container, domDoc);

	container->show();
}

bool
FormManager::isTopLevel(QWidget *w)
{
	kdDebug() << "FormManager::isTopLevel(): for: " << w->name() << " = " << activeForm()->objectTree()->lookup(w->name())<< endl;
	ObjectTreeItem *item = activeForm()->objectTree()->lookup(w->name());
	if(!item)
		return true;

	return (!item->parent());
}

void
FormManager::deleteWidget()
{
	//if (activeForm() && activeForm()->parentContainer())
	//	activeForm()->parentContainer()->deleteItem();
	if(!activeForm())
		return;

	QPtrList<QWidget> *list = activeForm()->selectedWidgets();
	if(list->isEmpty())
		return;

	QWidget *w;
	for(w = list->first(); w; w= list->next())
	{
		if(w)
			activeForm()->parentContainer(w)->deleteItem();
	}
}

void
FormManager::copyWidget()
{
	if (!activeForm() || !activeForm()->objectTree())
		return;

	QPtrList<QWidget> *list = activeForm()->selectedWidgets();
	if(list->isEmpty())
		return;

	QDomElement parent = m_domDoc.namedItem("UI").toElement();
	if(parent.hasChildNodes())
	{
		parent.clear();
		parent = m_domDoc.createElement("UI");
		m_domDoc.appendChild(parent);
	}

	QWidget *w;
	for(w = list->first(); w; w= list->next())
	{
		ObjectTreeItem *it = activeForm()->objectTree()->lookup(w->name());
		if (!it)
			return;
		FormIO::saveWidget(it, parent, m_domDoc);
	}
}

void
FormManager::cutWidget()
{
	copyWidget();
	deleteWidget();
}

void
FormManager::pasteWidget()
{
	if(!m_domDoc.namedItem("UI").hasChildNodes())
		return;
	if(!activeForm())
		return;

	if(m_domDoc.namedItem("UI").firstChild().nextSibling().isNull())
	{
		QDomElement widg = m_domDoc.namedItem("UI").firstChild().toElement();
		if(m_insertPoint.isNull())
			activeForm()->pasteWidget(widg);
		else
			activeForm()->pasteWidget(widg, m_insertPoint);
	}
	else for(QDomNode n = m_domDoc.namedItem("UI").firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement widg = n.toElement();
		activeForm()->pasteWidget(widg);
	}
}

void
FormManager::setInsertPoint(const QPoint &p)
{
	m_insertPoint = p;
}

void
FormManager::debugTree()
{
	if (activeForm() && activeForm()->objectTree())
		activeForm()->objectTree()->debug();
}

void
FormManager::showPropertyBuffer(ObjectPropertyBuffer *buff)
{
	if(m_editor)
		m_editor->setBuffer(buff);

	emit bufferSwitched(buff);
}

}

#include "formmanager.moc"

