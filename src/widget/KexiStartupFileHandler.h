/* This file is part of the KDE project
   Copyright (C) 2003-2014 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXISTARTUPFILEHANDLER_H
#define KEXISTARTUPFILEHANDLER_H

#include "kexiextwidgets_export.h"
#include <KexiFileFilters.h>

#include <QObject>

class QUrl;
class KFileDialog;
class KUrlRequester;
class KexiContextMessage;

//! @short Handler for use with file Kexi-specific opening/saving widgets/dialogs
/*! Make sure this object is destroyed before the handled file dialog
    or URL requester, otherwise recent dirs information will not be updated.
    Alternativelly you can call saveRecentDir(). */
class KEXIEXTWIDGETS_EXPORT KexiStartupFileHandler : public QObject
{
    Q_OBJECT

public:
/* removed in KEXI3
    KexiStartupFileHandler(
        const QUrl &startDirOrVariable, Mode mode, KFileDialog *dialog);*/

    //! @todo KEXI3 add equivalent of kfiledialog:/// for startDirOrVariable
    KexiStartupFileHandler(
        const QUrl &startDirOrVariable, KexiFileFilters::Mode mode, KUrlRequester *requester);

    virtual ~KexiStartupFileHandler();

    /*! Helper. Displays "This file already exists. Do you want to overwrite it?"
     message.
    @return true if @a filePath file does not exists or user has agreed on overwriting,
    false in user do not want to overwrite. Blocks until user 'answers'
    by to signal askForOverwriting(const KexiContextMessage&)
    by triggering 'yes' or 'no' action. */
    bool askForOverwriting(const QString& filePath);

    //! @return mode for the handler.
    KexiFileFilters::Mode mode() const;

    //! Sets mode for the handler.
    void setMode(KexiFileFilters::Mode mode);

    QStringList additionalMimeTypes() const;

    //! Sets additional mime types, e.g. "text/x-csv"
    void setAdditionalMimeTypes(const QStringList &mimeTypes);

    QStringList excludedMimeTypes() const;

    //! Excludes mime types
    void setExcludedMimeTypes(const QStringList &mimeTypes);

//removed in KEXI3    void setLocationText(const QString& fn);

    //! Sets default extension which will be added after accepting
    //! if user didn't provided one. This method is usable when there is
    //! more than one filter so there is no rule what extension should be selected
    //! (by default first one is selected).
    void setDefaultExtension(const QString& ext);

    /*! \return true if the current URL meets requies constraints
    (i.e. exists or doesn't exist);
    shows appropriate message box if needed. */
    bool checkSelectedUrl();

    /*! If true, user will be asked to accept overwriting existing file.
    This is true by default. */
    void setConfirmOverwrites(bool set);


    //! Updates the requested URL based on specified name. Performs any necessary character conversions.
    void updateUrl(const QString &name);
Q_SIGNALS:
    void askForOverwriting(const KexiContextMessage& message);

protected Q_SLOTS:
    void slotAccepted();
    void saveRecentDir();

    void messageWidgetActionYesTriggered();
    void messageWidgetActionNoTriggered();

private:
    void init(const QUrl &startDirOrVariable, KexiFileFilters::Mode mode);
    void updateFilters();

    class Private;
    Private * const d;
};

#endif
