/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004-2015 Jarosław Staniek <staniek@kde.org>

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

#ifndef KFORMDESIGNERWIDGETLIBRARY_H
#define KFORMDESIGNERWIDGETLIBRARY_H

#include <QObject>
#include <QList>

#include <KDbResult>

#include "widgetfactory.h"

template<class type> class QList;
template<class type> class QVector;
class KActionCollection;
class QAction;
class QMenu;
class QWidget;
class QVariant;
class QDomDocument;
class QDomElement;
class KexiFormWidgetsPluginMetaData;

namespace KFormDesigner
{

class Container;
class ObjectTreeItem;
class ActionGroup;
class WidgetInfo;

typedef QList<QAction *> ActionList;

/**
 * A manager class for searching and loading form widget factories
 *
 * It also provides actions for widget creation.
 * Every widget can be located using this library.
 * You call WidgetLibrary functions instead of calling directly factories.
 * See WidgetFactory for a description of the functions.
 */
class KFORMDESIGNER_EXPORT WidgetLibrary : public QObject, public KDbResultable
{
    Q_OBJECT

public:
    /*! Constructs WidgetLibrary object.
     In \a supportedFactoryGroups you can provide
     factory group list to be supported. Factory groups are defined by
     "X-Kexi-FormWidgetsFactoryGroup" field in every factory serviece's .desktop file.
     By default (when supportedFactoryGroups is empty) only factories having empty
     "X-Kexi-FormWidgetsFactoryGroup" field will be loaded.
     Factory group names are case-insensitive. */
    explicit WidgetLibrary(QObject *parent = 0,
                           const QStringList& supportedFactoryGroups = QStringList());

    virtual ~WidgetLibrary();

    /**
     * creates actions for widget creating
     */
    void createWidgetActions(ActionGroup *group);

    void addCustomWidgetActions(KActionCollection *col);

    /**
     * searches the right factory and creates a widget.
     * \return the widget or 0 if something falid
     */
    QWidget *createWidget(const QByteArray &classname, QWidget *parent, const char *name,
                          Container *c,
                          WidgetFactory::CreateWidgetOptions options = WidgetFactory::DefaultOptions) Q_REQUIRED_RESULT;

    bool createMenuActions(const QByteArray &c, QWidget *w, QMenu *menu,
                           KFormDesigner::Container *container);

    /**
     * Shows orientation selection popup.
     * \return one of the following values:
     * - WidgetFactory::AnyOrientation (means no selection has been made, i.e. it was cancelled)
     * - WidgetFactory::HorizontalOrientation
     * - WidgetFactory::VerticalOrientation
     */
    WidgetFactory::CreateWidgetOption showOrientationSelectionPopup(
        const QByteArray &classname, QWidget* parent, const QPoint& pos);

    QVariant internalProperty(const QByteArray& classname, const QByteArray& property);

    QString displayName(const QByteArray &classname);
    QString namePrefix(const QByteArray &classname);
    QString textForWidgetName(const QByteArray &name, const QByteArray &className);

    /*! Checks if the \a classname is an alternate classname,
     and returns the good classname.
     If \a classname is not alternate, \a classname is returned. */
    QByteArray classNameForAlternate(const QByteArray &classname);
    QString iconName(const QByteArray &classname);
    QString includeFileName(const QByteArray &classname);
    QString savingName(const QByteArray &classname);

    bool startInlineEditing(const QByteArray &classname, QWidget *w, Container *container);
    bool previewWidget(const QByteArray &classname, QWidget *widget, Container *container);
    bool clearWidgetContent(const QByteArray &classname, QWidget *w);

    bool saveSpecialProperty(const QByteArray &classname, const QString &name,
                             const QVariant &value, QWidget *w, QDomElement &parentNode, QDomDocument &parent);
    bool readSpecialProperty(const QByteArray &classname, QDomElement &node, QWidget *w,
                             ObjectTreeItem *item);
    bool isPropertyVisible(const QByteArray &classname, QWidget *w,
                           const QByteArray &property, bool multiple = false, bool isTopLevel = false);

    /*! @return list of the properties that should automatically be saved
    for a widget of @a classname class.
    Examples are: custom properties "text" for label or button, "contents" for combobox... */
    QList<QByteArray> autoSaveProperties(const QByteArray &classname);

    WidgetInfo* widgetInfoForClassName(const char* classname);

    WidgetFactory* factoryForClassName(const char* className);

    WidgetFactory* factory(const char* factoryName) const;

    /*! \return true if advanced properties such as "mouseTracking" should
     be visible to the user. True by default but Kexi sets it to false.
     See WidgetLibrary::Private() for a complete list of advanced properties (regardless of class)
     @see WidgetFactory::setAdvancedPropertiesVisible() WidgetLibrary::isPropertyVisible() */
    bool advancedPropertiesVisible() const;

    /*! Sets advanced properties to be visible or not. */
    void setAdvancedPropertiesVisible(bool set);

    /*! \return The i18n'ed name of the property \a propertyName
     for a class described by \a winfo. The name can be displayed in
     PropertyEditor. The name is retrieved from class' widget library.
     If this library doesn't define description for such property,
     and there is a parent library for \a winfo defined, parent library
     is asked for returning description string.
     Eventually, if even this failed, empty string is returned.
     @see WidgetFactory::propertyDescForName() */
    QString propertyDescForName(WidgetInfo *winfo, const QByteArray& propertyName);

    /*! \return The i18n'ed name of the property's value whose name is \a name.
     Works in the same way as propertyDescForName(): if actual library
     does not define a description we are looking for, parent factory is asked
     to return such description.
     Eventually, if even this failed, empty string is returned.
     @see WidgetFactory::propertyDescForValue() */
    QString propertyDescForValue(WidgetInfo *winfo, const QByteArray& name);

    /*! Used by Form::createPropertiesForWidget() after creating properties. */
    void setPropertyOptions(KPropertySet& set, const WidgetInfo& winfo, QWidget* w);

    /*! \return true if property sets should be reloaded for \a property property,
     \a classname class and widget \a w when a given property value changed. */
    bool propertySetShouldBeReloadedAfterPropertyChange(const QByteArray& classname, QWidget *w,
            const QByteArray& property);

    //! @return selectable item for @a item item.
    //! In most cases it is equal to @a item.
    //! See WidgetFactory::selectableItem(ObjectTreeItem*).
    ObjectTreeItem* selectableItem(ObjectTreeItem* item);

Q_SIGNALS:
    void widgetActionToggled(const QByteArray& _class);

    //! Received by KexiFormPart::slotWidgetCreatedByFormsLibrary() so we can add drag/drop
    //! connection for the new widget
    void widgetCreated(QWidget *widget);

protected:
    /**
     * Adds a factory to the library, creates actions for widgets in the added factory.
     * This function is not called directly but by the factory locater.
     */
    void loadFactoryWidgets(WidgetFactory *f);

    /**
     * Lookups widget factories list (note that this function get called once in ctor)
     */
    bool lookup();

#if 0
    /**
     * Loads widget factories found in lookupFactories(). This is called once.
     */
    void loadFactories();
#endif

    void setErrorMessage(KexiFormWidgetsPluginMetaData *pluginMetaData, const QString& errorMessage);

    class Private;
    Private * const d;
};

}
#endif
