/* This file is part of the KDE project
   Copyright (C) 2003   Lucijan Busch <lucijan@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KEXIQUERYPART_H
#define KEXIQUERYPART_H

#include <kexipart.h>
#include <kexipartitem.h>

class KexiMainWin;

class KexiQueryPart : public KexiPart::Part
{
	Q_OBJECT

	public:
		KexiQueryPart(QObject *parent, const char *name, const QStringList &);
		~KexiQueryPart();

		virtual bool remove(KexiMainWindow *win, const KexiPart::Item &item);

	protected:
		virtual KexiDialogBase* createInstance(KexiMainWindow *win, const KexiPart::Item &item, bool design);
//		virtual QString instanceName() const;
};

#endif

