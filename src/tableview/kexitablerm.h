/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Daniel Molkentin <molkentin@kde.org>

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

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#ifndef KEXITABLERM_H
#define KEXITABLERM_H

#include <qwidget.h>
#include <qimage.h>

/**record marker (at the left side of a table)
  *@author till busch
  */

class KEXIDATATABLE_EXPORT KexiTableRM : public QWidget
{
Q_OBJECT

public:
	KexiTableRM(QWidget *parent);
	~KexiTableRM();

	void paintEvent(QPaintEvent *e);

public slots:
	void setOffset(int offset);
	void setCellHeight(int cellHeight);
	void setCurrentRow(int row);

	void setInsertRow(int row);
	void setColor(const QColor &color);

	void addLabel();
	void removeLabel();

	/*! Adds \num labels */
	void addLabels(int num);

	void clear();

protected:
	int	m_rowHeight;
	int	m_offset;
	int	m_currentRow;
	int	m_insertRow;
	int	m_max;

	QColor	m_pointerColor;
	QImage m_penImg;
};

#endif
