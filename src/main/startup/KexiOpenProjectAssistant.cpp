/* This file is part of the KDE project
   Copyright (C) 2011-2013 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2012 Dimitrios T. Tanis <dimitrios.tanis@kdemail.net>

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

#include "KexiOpenProjectAssistant.h"
#include "KexiStartup.h"
#include "KexiPasswordPage.h"
#include "ui_KexiProjectStorageTypeSelectionPage.h"
#include <widget/KexiProjectSelectorWidget.h>
#include <widget/KexiConnectionSelectorWidget.h>
#include <kexiutils/KexiLinkWidget.h>
#include <kexiprojectset.h>
#include <kexiprojectdata.h>
#include <kexi.h>
#include <KexiIcon.h>

#include <KDbUtils>
#include <KDbDriverManager>

#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

KexiMainOpenProjectPage::KexiMainOpenProjectPage(QWidget* parent)
 : KexiAssistantPage(xi18nc("@title:window", "Open Project"),
                     xi18nc("@info", "Select project to open. "
                          "You can choose project stored in file or on database server."),
                     parent)
 , connSelector(0)
 , m_errorMessagePopup(0)
{
    setNextButtonVisible(true);

    tabWidget = new QTabWidget;
    tabWidget->setElideMode(Qt::ElideNone);
    //! @todo KEXI3 tabWidget->setAutomaticResizeTabs(true);
    tabWidget->setDocumentMode(true);

    m_fileSelectorWidget = new QWidget;
    tabWidget->addTab(m_fileSelectorWidget, Kexi::defaultFileBasedDriverIcon(),
                      xi18nc("@title:tab", "Projects Stored in File"));
    fileSelector = new KexiConnectionSelectorWidget(
        &Kexi::connset(),
        QUrl("kfiledialog:///OpenExistingOrCreateNewProject"),
        KexiConnectionSelectorWidget::Opening);
    fileSelector->hide(); // delayed opening
    fileSelector->showSimpleConnection();
    fileSelector->hideHelpers();
    fileSelector->hideDescription();
    fileSelector->setFileWidgetFrameVisible(false);
    connect(fileSelector, &KexiConnectionSelectorWidget::fileSelected, this, [=]() { next(); });

    m_connSelectorWidget = new QWidget;
    tabWidget->addTab(m_connSelectorWidget, Kexi::serverIcon(),
                      xi18nc("@title:tab", "Projects Stored on Database Server"));

    setRecentFocusWidget(tabWidget);
    setContents(tabWidget);

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    // delayed opening:
    QTimer::singleShot(500, this, SLOT(init()));
}

void KexiMainOpenProjectPage::init()
{
    // file-based:
    QVBoxLayout* fileSelectorLayout = new QVBoxLayout(m_fileSelectorWidget);
    fileSelectorLayout->setContentsMargins(0, KexiUtils::marginHint() * 2, 0, 0);
    fileSelectorLayout->addWidget(fileSelector);
    fileSelector->show();
}

void KexiMainOpenProjectPage::tabChanged(int index)
{
    QVBoxLayout* connSelectorLayout;
    if (!m_connSelectorWidget->layout()) {
        connSelectorLayout = new QVBoxLayout(m_connSelectorWidget);
    }
    else {
        connSelectorLayout = dynamic_cast<QVBoxLayout*>(m_connSelectorWidget->layout());
    }

    if (index == 1) {
        if (KDbDriverManager().hasDatabaseServerDrivers()) {
            if (!connSelector) {
                // server-based:
                connSelectorLayout->setContentsMargins(0, KexiUtils::marginHint() * 2, 0, 0);
                QLabel* connSelectorLabel = new QLabel(
                    xi18nc("@info",
                          "<para>Select database server's connection with project you wish to open.</para>"
                          "<para>Here you may also add, edit or delete connections from the list.</para>"));
                connSelectorLayout->addWidget(connSelectorLabel);
                connSelectorLayout->addSpacing(KexiUtils::marginHint());
                connSelector = new KexiConnectionSelectorWidget(
                    &Kexi::connset(),
                    QUrl("kfiledialog:///OpenExistingOrCreateNewProject"),
                    KexiConnectionSelectorWidget::Opening);
                connSelectorLayout->addWidget(connSelector);

                connSelector->showAdvancedConnection();
                connSelector->layout()->setContentsMargins(0, 0, 0, 0);
                connSelector->hideHelpers();
                connSelector->hideDescription();
                connect(connSelector, SIGNAL(connectionItemExecuted(ConnectionDataLVItem*)),
                        this, SLOT(next()));
            }
        }
        else {
            if (!m_errorMessagePopup) {
                setNextButtonVisible(false);
                setDescription(QString());
                m_errorMessagePopup = new KexiServerDriverNotFoundMessage(m_connSelectorWidget);
                connSelectorLayout->addSpacing(KexiUtils::marginHint());
                connSelectorLayout->addWidget(m_errorMessagePopup);
                connSelectorLayout->setAlignment(m_errorMessagePopup, Qt::AlignTop);
                m_errorMessagePopup->setAutoDelete(false);
                m_errorMessagePopup->animatedShow();
            }
        }
    }
}

KexiMainOpenProjectPage::~KexiMainOpenProjectPage()
{
}

// ----

KexiProjectDatabaseSelectionPage::KexiProjectDatabaseSelectionPage(
   KexiOpenProjectAssistant* parent)
 : KexiAssistantPage(xi18nc("@title:window", "Open Project on Database Server"), QString(), parent)
 , conndataToShow(0)
 , m_assistant(parent)
{
    setBackButtonVisible(true);
    setNextButtonVisible(true);
    nextButton()->setLinkText(xi18n("Open"));

    projectSelector = new KexiProjectSelectorWidget(
        this, 0,
        true, // showProjectNameColumn
        false // showConnectionColumns
    );
    projectSelector->label()->hide();
    connect(projectSelector, SIGNAL(projectExecuted(KexiProjectData*)),
            m_assistant, SLOT(slotOpenProject(KexiProjectData*)));

    setRecentFocusWidget(projectSelector);
    setContents(projectSelector);
}

KexiProjectDatabaseSelectionPage::~KexiProjectDatabaseSelectionPage()
{
}

bool KexiProjectDatabaseSelectionPage::setConnection(KDbConnectionData* data)
{
    projectSelector->setProjectSet(0);
    conndataToShow = 0;
    if (data) {
        KexiProjectSet *projectSetToShow = new KexiProjectSet(m_assistant);
        if (!projectSetToShow->setConnectionData(data)) {
            //! @todo show error
            delete projectSetToShow;
            return false;
        }
        conndataToShow = data;
        //-refresh projects list
        projectSelector->setProjectSet(projectSetToShow);
    }
    if (conndataToShow) {
        setDescription(
            xi18nc("@info", "Select project on database server <resource>%1 (%2)</resource> to open.",
                  conndataToShow->caption(), conndataToShow->toUserVisibleString()));
    }
    return true;
}

// ----

class Q_DECL_HIDDEN KexiOpenProjectAssistant::Private
{
public:
    explicit Private(KexiOpenProjectAssistant *qq)
     : q(qq)
    {
    }

    ~Private()
    {
    }

    KexiMainOpenProjectPage* projectOpenPage() {
        return page<KexiMainOpenProjectPage>(&m_projectOpenPage);
    }
    KexiProjectDatabaseSelectionPage* projectDatabaseSelectionPage() {
        return page<KexiProjectDatabaseSelectionPage>(&m_projectDatabaseSelectionPage, q);
    }
    KexiPasswordPage* passwordPage() {
        return page<KexiPasswordPage>(&m_passwordPage, q);
    }

    template <class C>
    C* page(QPointer<C>* p, KexiOpenProjectAssistant *parent = 0) {
        if (p->isNull()) {
            *p = new C(parent);
            q->addPage(*p);
        }
        return *p;
    }

    QPointer<KexiMainOpenProjectPage> m_projectOpenPage;
    QPointer<KexiProjectDatabaseSelectionPage> m_projectDatabaseSelectionPage;
    QPointer<KexiPasswordPage> m_passwordPage;

    KexiOpenProjectAssistant *q;
};

// ----

KexiOpenProjectAssistant::KexiOpenProjectAssistant(QWidget* parent)
 : KexiAssistantWidget(parent)
 , d(new Private(this))
{
    setCurrentPage(d->projectOpenPage());
    setFocusProxy(d->projectOpenPage());
}

KexiOpenProjectAssistant::~KexiOpenProjectAssistant()
{
    delete d;
}

void KexiOpenProjectAssistant::nextPageRequested(KexiAssistantPage* page)
{
    if (page == d->m_projectOpenPage) {
        if (d->m_projectOpenPage->tabWidget->currentIndex() == 0) {
            // file-based
            if (!d->m_projectOpenPage->fileSelector->checkSelectedFile())
                return;
            emit openProject(d->m_projectOpenPage->fileSelector->selectedFile());
        }
        else { // server-based
            KDbConnectionData *cdata
                = d->m_projectOpenPage->connSelector->selectedConnectionData();
            if (cdata) {
                if (cdata->isPasswordNeeded()) {
                    d->passwordPage()->setConnectionData(*cdata);
                    setCurrentPage(d->passwordPage());
                    return;
                }
                if (d->projectDatabaseSelectionPage()->setConnection(cdata)) {
                    setCurrentPage(d->projectDatabaseSelectionPage());
                }
            }
        }
    }
    else if (page == d->m_passwordPage) {
        KDbConnectionData *cdata
            = d->projectOpenPage()->connSelector->selectedConnectionData();
        d->passwordPage()->updateConnectionData(cdata);
        if (cdata && d->projectDatabaseSelectionPage()->setConnection(cdata)) {
            setCurrentPage(d->projectDatabaseSelectionPage());
        }
    }
    else if (page == d->m_projectDatabaseSelectionPage) {
        slotOpenProject(
            d->m_projectDatabaseSelectionPage->projectSelector->selectedProjectData());
    }
}

void KexiOpenProjectAssistant::cancelRequested(KexiAssistantPage* page)
{
    Q_UNUSED(page);
    //! @todo
}

void KexiOpenProjectAssistant::slotOpenProject(KexiProjectData* data)
{
    if (data)
        emit openProject(*data);
}

void KexiOpenProjectAssistant::tryAgainActionTriggered()
{
    messageWidget()->animatedHide();
    currentPage()->next();
}

void KexiOpenProjectAssistant::cancelActionTriggered()
{
    if (currentPage() == d->m_passwordPage) {
        d->passwordPage()->focusRecentFocusWidget();
    }
}

const QWidget* KexiOpenProjectAssistant::calloutWidget() const
{
    return currentPage()->nextButton();
}
