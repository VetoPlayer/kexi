/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

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

#include "kexiprojecthandlerproxy.h"
#include "kexiprojecthandler.h"
#include "kexiview.h"
#include "kexihandlerpopupmenu.h"

KexiProjectHandlerProxy::KexiProjectHandlerProxy(KexiProjectHandler *part,KexiView *view)
 : QObject(part)
	,m_view(view)
	,m_part(part)
	,m_createAction(0)
	,m_openAction(0)
	,m_editAction(0)
	,m_deleteAction(0)
{
	m_group_pmenu = new KexiPartPopupMenu(this);
	m_item_pmenu = new KexiPartPopupMenu(this);

}

KexiProjectHandler *KexiProjectHandlerProxy::part()const
{
	return m_part;
}

KexiView *KexiProjectHandlerProxy::kexiView()const
{
	return m_view;
}

#include "kexiprojecthandlerproxy.moc"
