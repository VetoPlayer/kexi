/* This file is part of the KDE project
   Copyright (C) 2003 by Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2009-2014 Jarosław Staniek <staniek@kde.org>

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

#include "KexiStandardFormWidgets.h"
#include "KexiMainFormWidgetsFactory.h"
#include <KexiIcon.h>

#include <KTextEdit>
#include <KLocalizedString>

#include <QStackedWidget>

KexiLineWidget::KexiLineWidget(Qt::Orientation o, QWidget *parent)
        : QFrame(parent)
{
    setFrameShadow(Sunken);
    if (o == Qt::Horizontal)
        setFrameShape(HLine);
    else
        setFrameShape(VLine);
}

KexiLineWidget::~KexiLineWidget()
{
}

void KexiLineWidget::setOrientation(Qt::Orientation o)
{
    if (o == Qt::Horizontal)
        setFrameShape(HLine);
    else
        setFrameShape(VLine);
}

Qt::Orientation KexiLineWidget::orientation() const
{
    if (frameShape() == HLine)
        return Qt::Horizontal;
    else
        return Qt::Vertical;
}

// ---

EditRichTextAction::EditRichTextAction(KFormDesigner::Container *container,
                                       QWidget *receiver, QObject *parent,
                                       KexiMainFormWidgetsFactory *factory)
    : QAction(koIcon("document-edit"),
              xi18nc("Edit rich text for a widget", "Edit Rich Text"),
              parent)
    , m_container(container)
    , m_receiver(receiver)
    , m_factory(factory)
{
    connect(this, SIGNAL(triggered()), this, SLOT(slotTriggered()));
}

void EditRichTextAction::slotTriggered()
{
    const QByteArray classname( m_receiver->metaObject()->className() );
    QString text;
    if (classname == "KTextEdit") {
        KTextEdit* te = qobject_cast<KTextEdit*>(m_receiver);
        if (te->acceptRichText()) {
            text = te->toHtml();
        }
        else {
            text = te->toPlainText();
        }
    }
    else if (classname == "QLabel") {
        text = qobject_cast<QLabel*>(m_receiver)->text();
    }

    if (m_factory->editRichText(m_receiver, text)) {
//! @todo ok?
        m_factory->changeProperty(m_container->form(), m_receiver, "acceptRichText", true);
        m_factory->changeProperty(m_container->form(), m_receiver, "text", text);
    }

    if (classname == "QLabel") {
        m_receiver->resize(m_receiver->sizeHint());
    }
}
