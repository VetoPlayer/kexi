/* This file is part of the KDE project
   Copyright (C) 2005-2017 Jarosław Staniek <staniek@kde.org>

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
#ifndef KEXIDATASOURCEPAGE_H
#define KEXIDATASOURCEPAGE_H

#include "kexiformutils_export.h"
#include <config-kexi.h>
#include <KDbField>
#include <KDbUtils>
#include <KDbTableOrQuerySchema>

#include <KPropertySet>

#include <QWidget>

class KexiDataSourceComboBox;
class KexiFieldComboBox;
class KexiFieldListView;
class KexiProject;
class QToolButton;
class QLabel;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;

//! A page within form designer's property tabbed pane, providing data source editor
class KEXIFORMUTILS_EXPORT KexiDataSourcePage : public QWidget
{
    Q_OBJECT

public:
    explicit KexiDataSourcePage(QWidget *parent = 0);
    virtual ~KexiDataSourcePage();

    //QSize sizeHint() const Q_DECL_OVERRIDE { return QSize(); }

    enum AssignFlag {
        NoFlags = 0,
        ForceAssign = 1
    };
    Q_DECLARE_FLAGS(AssignFlags, AssignFlag)

    //! @return name plugin ID of selected item (usually a table or a query). Can return an empty string.
    QString selectedPluginId() const;

    //! @return name of selected table or query.
    QString selectedName() const;

public Q_SLOTS:
    void setProject(KexiProject *prj);
    void clearFormDataSourceSelection(bool alsoClearComboBox = true);
    void clearWidgetDataSourceSelection();

    //! Sets data source of a currently selected form.
    //! This is performed on form initialization and on activating.
    void setFormDataSource(const QString& pluginId, const QString& name);

    //! Receives a pointer to a new property \a set (from KexiFormView::managerPropertyChanged())
    void assignPropertySet(KPropertySet* propertySet, AssignFlags flags = NoFlags);

Q_SIGNALS:
    //! Signal emitted when helper button 'go to selected data source' is clicked.
    void jumpToObjectRequested(const QString& mime, const QString& name);

    //! Signal emitted when form's data source has been changed. It's connected to the Form Manager.
    void formDataSourceChanged(const QString& mime, const QString& name);

    /*! Signal emitted when current widget's data source (field/expression)
     has been changed. It's connected to the Form Manager.
     \a caption for this field is also provided (e.g. AutoField form widget use it) */
    void dataSourceFieldOrExpressionChanged(const QString& string, const QString& caption,
                                            KDbField::Type type);

    /*! Signal emitted when 'insert fields' button has been clicked */
    void insertAutoFields(const QString& sourcePartClass, const QString& sourceName,
                          const QStringList& fields);

protected Q_SLOTS:
    void slotWidgetDataSourceTextChanged(const QString &text);
    void slotFormDataSourceTextChanged(const QString &text);
    void slotFormDataSourceChanged();
    void slotFieldSelected();
    void slotGotoSelected();
    void slotInsertSelectedFields();
    void slotFieldListViewSelectionChanged();
    void slotFieldDoubleClicked(const QString& sourcePluginId, const QString& sourceName,
                                const QString& fieldName);

protected:
    void updateSourceFieldWidgetsAvailability();

    QVBoxLayout *m_mainLyr;
    QGridLayout *m_formLyr;
    KexiFieldComboBox *m_widgetDataSourceCombo;
    QWidget *m_widgetDataSourceContainer;
    KexiDataSourceComboBox* m_formDataSourceCombo;
    QWidget *m_formDataSourceComboSpacer;
    QLabel *m_noDataSourceAvailableLabel;
    QToolButton *m_gotoButton;
    QString m_noDataSourceAvailableSingleText;
    QString m_noDataSourceAvailableMultiText;
    bool m_insideClearFormDataSourceSelection;
    bool m_slotWidgetDataSourceTextChangedEnabled;
#ifdef KEXI_AUTOFIELD_FORM_WIDGET_SUPPORT
    KexiFieldListView* m_availableFieldsLabel;
    KexiFieldListView* m_fieldListView;
    QLabel *m_mousePointerLabel;
    QLabel *m_availableFieldsDescriptionLabel;
    QToolButton *m_addField;
#else
    KDbTableOrQuerySchema *m_tableOrQuerySchema; //!< temp.
#endif

    //! Used only in assignPropertySet() to check whether we already have the set assigned
    QString m_currentObjectName;
};

#endif
