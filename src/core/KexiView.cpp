/* This file is part of the KDE project
   Copyright (C) 2004-2016 Jarosław Staniek <staniek@kde.org>

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

#include "KexiView.h"
#include "KexiMainWindowIface.h"
#include "KexiWindow.h"
#include "kexiproject.h"
#include "kexipartinfo.h"
#include <kexiutils/utils.h>
#include <kexiutils/SmallToolButton.h>
#include <kexiutils/FlowLayout.h>
#include <KexiStyle.h>
#include <KexiIcon.h>

#include "KexiGroupButton.h"

#include <KPropertySet>

#include <KDbConnection>

#include <KActionCollection>

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QVBoxLayout>

//! @internal Action for toggling view mode
class KEXICORE_EXPORT KexiToggleViewModeAction : public QAction
{
    Q_OBJECT
public:
    //! Creates action for toggling to view mode @a mode. @a slot should have signature
    //! matching switchedTo(Kexi::ViewMode mode) signal.
    KexiToggleViewModeAction(const QString& pluginId, Kexi::ViewMode mode, QObject* parent)
        : QAction(
            QIcon::fromTheme(Kexi::iconNameForViewMode(pluginId, mode)),
            Kexi::nameForViewMode(mode, true/*withAmpersand*/),
            parent)
    {
        setCheckable(true);
        if (mode == Kexi::DesignViewMode) {
            setObjectName("view_design_mode");
            setToolTip(xi18n("Switch to visual design view"));
            setWhatsThis(xi18n("Switches to visual design view."));
        } else if (mode == Kexi::TextViewMode) {
            setObjectName("view_text_mode");
            setToolTip(xi18n("Switch to text view"));
            setWhatsThis(xi18n("Switches to text view."));
        } else {
            qWarning() << "KexiToggleViewModeAction: invalid mode" << mode;
        }
    }
};

//-------------------------

class Q_DECL_HIDDEN KexiView::Private
{
public:
    explicit Private(KexiView *qq)
            : q(qq)
            , viewWidget(0)
            , parentView(0)
            , newlyAssignedID(-1)
            , viewMode(Kexi::NoViewMode) //unknown!
            , isDirty(false)
            , slotSwitchToViewModeInternalEnabled(true)
            , sortedProperties(false)
            , recentResultOfSwitchToViewModeInternal(true)
            , m_mainMenu(0)
    {
    }

    ~Private() {
    }

    void toggleViewModeButtonBack(Kexi::ViewMode mode) {
        QAction *a = toggleViewModeActions.value(mode);
        if (a) {
            slotSwitchToViewModeInternalEnabled = false;
            toggleViewModeButtons.value(mode)->setChecked(viewMode == mode);
            slotSwitchToViewModeInternalEnabled = true;
        }
    }

    QMenu* mainMenu()
    {
        if (m_mainMenu) {
            return m_mainMenu;
        }
        if (!window) {
            return 0;
        }
        KexiSmallToolButton* menuButton = new KexiSmallToolButton(
             koIcon("application-menu"), topBarHWidget);
        menuButton->setToolTip(xi18nc("@info:tooltip", "Menu for <resource>%1</resource>")
                               .arg(window->partItem()->captionOrName()));
        menuButton->setWhatsThis(xi18n("Shows context menu for the current view."));
        menuButton->setPopupMode(QToolButton::InstantPopup);
        topBarLyr->insertWidget(0, menuButton);

        m_mainMenu = new QMenu(menuButton);
        menuButton->setMenu(m_mainMenu);
        return m_mainMenu;
    }

    KexiGroupButton *addViewButton(KexiGroupButton::GroupPosition pos,
                                 Kexi::ViewMode mode, QWidget *parent, const char *slot,
                                 const QString &text, QHBoxLayout *btnLyr)
    {
        if (!window->supportsViewMode(mode)) {
            return 0;
        }
        QAction *a = new KexiToggleViewModeAction(q->part()->info()->id(), mode, q);
        toggleViewModeActions.insert(mode, a);

        KexiGroupButton *btn = new KexiGroupButton(pos, parent);
        toggleViewModeButtons.insert(mode, btn);
        connect(btn, SIGNAL(toggled(bool)), q, slot);
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        btn->setText(text);
        btn->setIcon(a->icon());
        QFont f(q->font());
        f.setPointSizeF(KexiUtils::smallestReadableFont().pointSizeF());
        btn->setFont(f);
        btn->setToolTip(a->toolTip());
        btn->setWhatsThis(a->whatsThis());
        btn->setCheckable(true);
        btn->setAutoRaise(true);
        btnLyr->addWidget(btn);
        return btn;
    }

    KexiView *q;
    QVBoxLayout* mainLyr;
    QWidget *topBarHWidget;
    KexiFlowLayout *topBarLyr;
    QHash<Kexi::ViewMode, QAction*> toggleViewModeActions;
    QHash<Kexi::ViewMode, KexiGroupButton*> toggleViewModeButtons;

    KexiSmallToolButton* saveDesignButton;

    QString defaultIconName;
    KexiWindow *window;
    QWidget *viewWidget;
    KexiView *parentView;

    QPointer<QWidget> lastFocusedChildBeforeFocusOut;

    /*! Member set to newly assigned object's ID in storeNewData()
     and used in storeDataBlock(). This is needed because usually,
     storeDataBlock() can be called from storeNewData() and in this case
     window has not yet assigned valid identifier (it has just negative temp. number).
     \sa KexiWindow::id()
     */
    int newlyAssignedID;

    /*! Mode for this view. Initialized by KexiWindow::switchToViewMode().
     Can be useful when single class is used for more than one view (e.g. KexiDBForm). */
    Kexi::ViewMode viewMode;

    QList<KexiView*> children;

    /*! View-level actions (not shared), owned by the view. */
    QList<QAction*> viewActions;
    QHash<QByteArray, QAction*> viewActionsHash;

    /*! Main-meny-level actions (not shared), owned by the view. */
    QList<QAction*> mainMenuActions;
    QHash<QByteArray, QAction*> mainMenuActionsHash;

    bool isDirty;

    //! Used in slotSwitchToViewModeInternal() to disabling it
    bool slotSwitchToViewModeInternalEnabled;

    bool sortedProperties;

    //! Used in slotSwitchToViewModeInternal() to disabling d->window->switchToViewModeInternal(mode) call.
    //! Needed because there is another slotSwitchToViewModeInternal() calls if d->window->switchToViewModeInternal(mode)
    //! did not succeed, so for the second time we block this call.
    tristate recentResultOfSwitchToViewModeInternal;

    QString textToDisplayForNullSet;
private:
    QMenu* m_mainMenu;
};

//----------------------------------------------------------

KexiView::KexiView(QWidget *parent)
        : QWidget(parent)
        , KexiActionProxy(this)
        , d(new Private(this))
{
    QWidget *wi = this;
    while ((wi = wi->parentWidget()) && !qobject_cast<KexiWindow*>(wi)) {
    }
    d->window = (wi && qobject_cast<KexiWindow*>(wi)) ? qobject_cast<KexiWindow*>(wi) : nullptr;
    if (d->window) {
        //init view mode number for this view (obtained from window where this view is created)
        if (d->window->supportsViewMode(d->window->creatingViewsMode()))
            d->viewMode = d->window->creatingViewsMode();
    }
    setObjectName(
        QString("%1_for_%2_object")
        .arg(Kexi::nameForViewMode(d->viewMode).replace(' ', '_'))
        .arg(d->window ? d->window->partItem()->name() : QString("??")));

    installEventFilter(this);

    d->mainLyr = new QVBoxLayout(this);
    d->mainLyr->setContentsMargins(0, 0, 0, 0);

    if (qobject_cast<KexiWindow*>(parentWidget())) {
        createTopBar();
    } else {
        d->saveDesignButton = 0;
        d->topBarHWidget = 0;
        d->topBarLyr = 0;
    }
}

void KexiView::createTopBar()
{
    d->topBarHWidget = new QWidget(this);
    d->topBarHWidget->setFont(KexiUtils::smallestReadableFont());
    d->mainLyr->addWidget(d->topBarHWidget);
    QHBoxLayout *topBarHLyr = new QHBoxLayout(d->topBarHWidget); //needed unless KexiFlowLayout properly handles contents margins
    topBarHLyr->setContentsMargins(0, 0, 0, 0);
    topBarHLyr->addSpacing(KexiUtils::spacingHint() / 2);
    d->topBarLyr = new KexiFlowLayout(topBarHLyr, 0, 0);

    bool toggleModePossible = viewMode() != Kexi::DataViewMode && !KexiMainWindowIface::global()->userMode();
    if (   !(d->window->supportedViewModes() & Kexi::DesignViewMode)
        || !(d->window->supportedViewModes() & Kexi::TextViewMode))
    {
        toggleModePossible = false;
    }
    if (toggleModePossible) {
        createViewModeToggleButtons();
    }

    (void)d->mainMenu();

    if (d->viewMode == Kexi::DesignViewMode || d->viewMode == Kexi::TextViewMode) {
        QAction *a = sharedAction("project_save");
        d->saveDesignButton = new KexiSmallToolButton(a, d->topBarHWidget);
        d->saveDesignButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        d->saveDesignButton->setToolTip(xi18n("Save current design"));
        d->saveDesignButton->setWhatsThis(xi18n("Saves changes made to the current design."));
        d->topBarLyr->addWidget(d->saveDesignButton);

        a = sharedAction("project_saveas");
        d->mainMenu()->addAction(a);
    }
    else {
        d->saveDesignButton = 0;
    }
}

KexiView::~KexiView()
{
    delete d;
}

KexiWindow* KexiView::window() const
{
    return d->window;
}

bool KexiView::isDirty() const
{
    return d->isDirty;
}

bool KexiView::isDataEditingInProgress() const
{
    return false;
}

tristate KexiView::saveDataChanges()
{
    return true;
}

tristate KexiView::cancelDataChanges()
{
    return true;
}

Kexi::ViewMode KexiView::viewMode() const
{
    return d->viewMode;
}

KexiPart::Part* KexiView::part() const
{
    return d->window ? d->window->part() : 0;
}

tristate KexiView::beforeSwitchTo(Kexi::ViewMode mode, bool *dontStore)
{
    Q_UNUSED(mode);
    Q_UNUSED(dontStore);
    return true;
}

tristate KexiView::afterSwitchFrom(Kexi::ViewMode mode)
{
    Q_UNUSED(mode);
    return true;
}

QSize KexiView::preferredSizeHint(const QSize& otherSize)
{
    return otherSize;
}

void KexiView::closeEvent(QCloseEvent * e)
{
    bool cancel = false;
    emit closing(&cancel);
    if (cancel) {
        e->ignore();
        return;
    }
    QWidget::closeEvent(e);
}

KPropertySet *KexiView::propertySet()
{
    return 0;
}

void KexiView::propertySetSwitched()
{
    if (window()) {
        KexiMainWindowIface::global()->propertySetSwitched(window(), false/*force*/,
            true/*preservePrevSelection*/, d->sortedProperties);
    }
}

void KexiView::propertySetReloaded(bool preservePrevSelection,
                                   const QByteArray& propertyToSelect)
{
    if (window())
        KexiMainWindowIface::global()->propertySetSwitched(
            window(), true, preservePrevSelection, d->sortedProperties, propertyToSelect);
}

void KexiView::setDirty(bool set)
{
    const bool changed = (d->isDirty != set);
    d->isDirty = set;
    d->isDirty = isDirty();
    if (d->saveDesignButton)
        d->saveDesignButton->setEnabled(d->isDirty);
    if (d->parentView) {
        d->parentView->setDirty(d->isDirty);
    } else {
        if (changed && d->window)
            d->window->dirtyChanged(this);
    }
}

void KexiView::setDirty()
{
    setDirty(true);
}

KDbObject* KexiView::storeNewData(const KDbObject& object,
                                           KexiView::StoreNewDataOptions options,
                                           bool *cancel)
{
    Q_ASSERT(cancel);
    Q_UNUSED(options)
    Q_UNUSED(cancel)
    QScopedPointer<KDbObject> newObject(new KDbObject);
    *newObject = object;

    KDbConnection *conn = KexiMainWindowIface::global()->project()->dbConnection();
    if (!conn->storeNewObjectData(newObject.data())
        || !conn->removeDataBlock(newObject->id()) // for sanity
        || !KexiMainWindowIface::global()->project()->removeUserDataBlock(newObject->id()) // for sanity
       )
    {
        return 0;
    }
    d->newlyAssignedID = newObject->id();
    return newObject.take();
}

KDbObject* KexiView::copyData(const KDbObject& object,
                                        KexiView::StoreNewDataOptions options,
                                        bool *cancel)
{
    Q_ASSERT(cancel);
    Q_UNUSED(options)
    Q_UNUSED(cancel)
    QScopedPointer<KDbObject> newObject(new KDbObject);
    *newObject = object;

    KDbConnection *conn = KexiMainWindowIface::global()->project()->dbConnection();
    if (!conn->storeNewObjectData(newObject.data())
        || !conn->copyDataBlock(d->window->id(), newObject->id())
        || !KexiMainWindowIface::global()->project()->copyUserDataBlock(d->window->id(), newObject->id())
       )
    {
        return 0;
    }
    d->newlyAssignedID = newObject->id();
    return newObject.take();
}

tristate KexiView::storeData(bool dontAsk)
{
    Q_UNUSED(dontAsk);
    if (!d->window || !d->window->schemaObject())
        return false;
    if (!KexiMainWindowIface::global()->project()
            ->dbConnection()->storeObjectData(d->window->schemaObject()))
    {
        return false;
    }
    setDirty(false);
    return true;
}

bool KexiView::loadDataBlock(QString *dataString, const QString& dataID, bool canBeEmpty)
{
    if (!d->window)
        return false;
    const tristate res = KexiMainWindowIface::global()->project()->dbConnection()
                         ->loadDataBlock(d->window->id(), dataString, dataID);
    if (canBeEmpty && ~res) {
        dataString->clear();
        return true;
    }
    return res == true;
}

bool KexiView::storeDataBlock(const QString &dataString, const QString &dataID)
{
    if (!d->window)
        return false;
    int effectiveID;
    if (d->newlyAssignedID > 0) {//ID not yet stored within window, but we've got ID here
        effectiveID = d->newlyAssignedID;
        d->newlyAssignedID = -1;
    } else
        effectiveID = d->window->id();

    return effectiveID > 0
           && KexiMainWindowIface::global()->project()->dbConnection()->storeDataBlock(
               effectiveID, dataString, dataID);
}

bool KexiView::removeDataBlock(const QString& dataID)
{
    if (!d->window)
        return false;
    return KexiMainWindowIface::global()->project()->dbConnection()
           ->removeDataBlock(d->window->id(), dataID);
}

bool KexiView::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) {
//        qDebug() << "this=[" << o->metaObject()->className()
//            << objectName() << "] o=[" << o->metaObject()->className() << o->objectName()
//            << "] focusWidget=[" << (qApp->focusWidget() ? qApp->focusWidget()->metaObject()->className() : QString())
//            << (qApp->focusWidget() ? qApp->focusWidget()->objectName() : QString()) << "] ev.type=" << e->type();
        if (KDbUtils::hasParent(this, o)) {
            if (e->type() == QEvent::FocusOut && qApp->focusWidget()
                    && !KDbUtils::hasParent(this, qApp->focusWidget())) {
                //focus out: when currently focused widget is not a parent of this view
                emit focus(false);
            } else if (e->type() == QEvent::FocusIn) {
                emit focus(true);
            }
            if (e->type() == QEvent::FocusOut) {
//    qDebug() << focusWidget()->className() << focusWidget()->name();
//    qDebug() << o->className() << o->name();
                KexiView *v = KDbUtils::findParent<KexiView*>(o);
                if (v) {
                    while (v->d->parentView)
                        v = v->d->parentView;
                    if (KDbUtils::hasParent(this, static_cast<QWidget*>(v->focusWidget())))
                        v->d->lastFocusedChildBeforeFocusOut = static_cast<QWidget*>(v->focusWidget());
                }
            }

            if (e->type() == QEvent::FocusIn && m_actionProxyParent) {
                m_actionProxyParent->m_focusedChild = this;
            }
        }
    }
    return false;
}

void KexiView::setViewWidget(QWidget* w, bool focusProxy)
{
    if (d->viewWidget == w)
        return;
    if (d->viewWidget) {
        d->viewWidget->removeEventFilter(this);
        d->mainLyr->removeWidget(d->viewWidget);
    }
    d->viewWidget = w;
    if (d->viewWidget) {
        QFrame *frameWidget = qobject_cast<QFrame*>(d->viewWidget);
        KexiStyle::setupFrame(frameWidget);
        d->viewWidget->setParent(this);
        d->mainLyr->addWidget(d->viewWidget, 1);
        d->viewWidget->installEventFilter(this);
        //}
        if (focusProxy)
            setFocusProxy(d->viewWidget); //js: ok?
    }
}

void KexiView::addChildView(KexiView* childView)
{
    d->children.append(childView);
    addActionProxyChild(childView);
    childView->d->parentView = this;
    childView->installEventFilter(this);
}

void KexiView::removeView(Kexi::ViewMode mode)
{
    window()->removeView(mode);
}

void KexiView::setFocus()
{
    if (!d->lastFocusedChildBeforeFocusOut.isNull()) {
//  qDebug() << "FOCUS:" << d->lastFocusedChildBeforeFocusOut->className() << d->lastFocusedChildBeforeFocusOut->name();
        QWidget *w = d->lastFocusedChildBeforeFocusOut;
        d->lastFocusedChildBeforeFocusOut = 0;
        w->setFocus();
    } else {
        setFocusInternal();
    }
    KexiMainWindowIface::global()->invalidateSharedActions(this);
}

QAction* KexiView::sharedAction(const QString& action_name)
{
    if (part()) {
        KActionCollection *ac;
        if ((ac = part()->actionCollectionForMode(viewMode()))) {
            QAction* a = ac->action(action_name);
            if (a)
                return a;
        }
    }
    return KexiActionProxy::sharedAction(action_name);
}

void KexiView::setAvailable(const QString& action_name, bool set)
{
    if (part()) {
        KActionCollection *ac;
        QAction* a;
        if ((ac = part()->actionCollectionForMode(viewMode())) && (a = ac->action(action_name))) {
            a->setEnabled(set);
        }
    }
    KexiActionProxy::setAvailable(action_name, set);
}

void KexiView::updateActions(bool activated)
{
    //do nothing here
    //do the same for children :)
    foreach(KexiView* view, d->children) {
        view->updateActions(activated);
    }
}

void KexiView::setViewActions(const QList<QAction*>& actions)
{
    d->viewActions = actions;
    d->viewActionsHash.clear();
    foreach(QAction* action, d->viewActions) {
        d->viewActionsHash.insert(action->objectName().toLatin1(), action);
    }
}

void KexiView::setMainMenuActions(const QList<QAction*>& actions)
{
    d->mainMenuActions = actions;
    d->mainMenuActionsHash.clear();
    foreach(QAction* action, d->mainMenuActions) {
        d->mainMenuActionsHash.insert(action->objectName().toLatin1(), action);
    }
}

QAction* KexiView::viewAction(const char* name) const
{
    return d->viewActionsHash.value(name);
}

QList<QAction*> KexiView::viewActions() const
{
    return d->viewActions;
}

void KexiView::toggleViewModeButtonBack()
{
    d->toggleViewModeButtonBack(Kexi::DataViewMode);
    d->toggleViewModeButtonBack(Kexi::DesignViewMode);
    d->toggleViewModeButtonBack(Kexi::TextViewMode);
}

void KexiView::createViewModeToggleButtons()
{
    d->topBarLyr->addSpacing(KexiUtils::spacingHint());

    QWidget *btnCont = new QWidget(d->topBarHWidget);
    QHBoxLayout *btnLyr = new QHBoxLayout;
    btnLyr->setSpacing(0);
    btnLyr->setContentsMargins(0, 0, 0, 0);
    btnCont->setLayout(btnLyr);
    d->topBarLyr->addWidget(btnCont);
    d->topBarLyr->addSpacing(KexiUtils::spacingHint());

    d->addViewButton(KexiGroupButton::GroupLeft, Kexi::DesignViewMode, btnCont,
                     SLOT(slotSwitchToDesignViewModeInternal(bool)),
                     xi18nc("@action:button Visual Design", "Visual"), btnLyr);
    KexiGroupButton *btn = d->addViewButton(KexiGroupButton::GroupRight, Kexi::TextViewMode,
                                          btnCont, SLOT(slotSwitchToTextViewModeInternal(bool)),
                                          QString(), btnLyr);
    if (btn) {
        QAction *a = d->toggleViewModeActions.value(Kexi::TextViewMode);
        const QString customTextViewModeCaption(d->window->internalPropertyValue("textViewModeCaption").toString());
        btn->setText(customTextViewModeCaption.isEmpty() ? a->text() : customTextViewModeCaption);
        const QString customTextViewModeToolTip(d->window->internalPropertyValue("textViewModeToolTip").toString());
        btn->setToolTip(customTextViewModeToolTip.isEmpty() ? a->toolTip() : customTextViewModeToolTip);
    }

    toggleViewModeButtonBack();
}

void KexiView::slotSwitchToDesignViewModeInternal(bool)
{
    slotSwitchToViewModeInternal(Kexi::DesignViewMode);
}

void KexiView::slotSwitchToTextViewModeInternal(bool)
{
    slotSwitchToViewModeInternal(Kexi::TextViewMode);
}

void KexiView::slotSwitchToViewModeInternal(Kexi::ViewMode mode)
{
    if (!d->slotSwitchToViewModeInternalEnabled)
        return;
    if (d->recentResultOfSwitchToViewModeInternal != true)
        d->recentResultOfSwitchToViewModeInternal = true;
    else
        d->recentResultOfSwitchToViewModeInternal = d->window->switchToViewModeInternal(mode);

    if (d->viewMode != mode) {
        //switch back visually
        KexiGroupButton *b = d->toggleViewModeButtons.value(mode);
        d->slotSwitchToViewModeInternalEnabled = false;
        b->setChecked(false);
        d->slotSwitchToViewModeInternalEnabled = true;
    }
}

void KexiView::initViewActions()
{
    if (!d->topBarLyr)
        return;
    foreach(QAction* action, d->viewActions) {
        if (action->isSeparator()) {
            d->topBarLyr->addWidget(new KexiToolBarSeparator(d->topBarHWidget));
        } else {
            KexiSmallToolButton *btn = new KexiSmallToolButton(action, d->topBarHWidget);
            btn->setText(action->text());
            btn->setToolTip(action->toolTip());
            btn->setWhatsThis(action->whatsThis());
            //! @todo Local toolbar actions are hardcoded as icons-only, maybe add view option?
#if 0
            if (action->dynamicPropertyNames().contains("iconOnly") && action->property("iconOnly").toBool() ) {
#endif
            btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
            //}
            d->topBarLyr->addWidget(btn);
        }
    }
}

void KexiView::initMainMenuActions()
{
    if (!d->topBarLyr)
        return;
    if (d->mainMenuActions.isEmpty()) {
        return;
    }
    d->mainMenu()->clear();
    foreach(QAction* action, d->mainMenuActions) {
        d->mainMenu()->addAction(action);
    }
}

void KexiView::setSortedProperties(bool set)
{
    d->sortedProperties = set;
}

bool KexiView::saveSettings()
{
    return true;
}

QString KexiView::defaultIconName() const
{
    return d->defaultIconName;
}

void KexiView::setDefaultIconName(const QString& iconName)
{
    d->defaultIconName = iconName;
}

QList<QVariant> KexiView::currentParameters() const
{
    return QList<QVariant>();
}

QString KexiView::textToDisplayForNullSet() const
{
    return d->textToDisplayForNullSet;
}

void KexiView::setTextToDisplayForNullSet(const QString& text)
{
    d->textToDisplayForNullSet = text;
}

#include "KexiView.moc"
