/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2004-2009 Jarosław Staniek <staniek@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "widgetfactory.h"

#include "WidgetInfo.h"
#include "libactionwidget.h"

#include <KexiIcon.h>

using namespace KFormDesigner;

class Q_DECL_HIDDEN LibActionWidget::Private
{
public:
    explicit Private(WidgetInfo *w);
    ~Private();

    QByteArray className;
};

LibActionWidget::Private::Private(WidgetInfo *w) : className(w->className())
{
}

LibActionWidget::Private::~Private()
{
}

LibActionWidget::LibActionWidget(ActionGroup *group, WidgetInfo *w)
    : KToggleAction(QIcon::fromTheme(w->iconName()), w->name(), group), d(new Private(w))
{
    setObjectName(QLatin1String("library_widget_") + w->className());
    group->addAction(this);
    setToolTip(w->name());
    setWhatsThis(w->description());
}

LibActionWidget::~LibActionWidget()
{
    delete d;
}

void
LibActionWidget::slotToggled(bool checked)
{
    KToggleAction::slotToggled(checked);
    if (checked)
        emit toggled(d->className);
}

