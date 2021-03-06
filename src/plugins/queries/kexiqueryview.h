/* This file is part of the KDE project
   Copyright (C) 2004 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004, 2006 Jarosław Staniek <staniek@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIQUERYVIEW_H
#define KEXIQUERYVIEW_H

#include <widget/tableview/KexiDataTableView.h>

class KDbQuerySchema;

class KexiQueryView : public KexiDataTableView
{
    Q_OBJECT

public:
    explicit KexiQueryView(QWidget *parent);
    virtual ~KexiQueryView();

    /*! \return curent parameters for parametrized query */
    virtual QList<QVariant> currentParameters() const;

protected:
    virtual tristate afterSwitchFrom(Kexi::ViewMode mode);

    virtual KDbObject* storeNewData(const KDbObject& object,
                                             KexiView::StoreNewDataOptions options,
                                             bool *cancel);

    virtual tristate storeData(bool dontAsk = false);

    /**
     * Assigns query @a query to this view
     *
     * - executes it
     * - fills the table view with results
     *
     * @return @c true on success, @c false on failure and @c cancelled when user has cancelled
     * execution, for example when she pressed the Cancel button in the "Enter Query Parameter"
     * input dialog.
     *
     * In successful execution of the query of if @a query is @c nullptr previous query (if any)
     * is unassigned and data is removed.
     */
    tristate setQuery(KDbQuerySchema *query);

    /**
     * @brief Query that was assigned to this view
     *
     * @note It is not owned.
     */
    KDbQuerySchema *query();

    friend class KexiQueryPartTempData;

    class Private;
    Private * const d;
};

#endif

