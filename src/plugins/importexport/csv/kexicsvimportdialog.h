/* This file is part of the KDE project
   Copyright (C) 2005-2016 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2012 Oleg Kukharchuk <oleg.kuh@gmail.com>

   This work is based on kspread/dialogs/kspread_dlg_csv.cc.

   Copyright (C) 2002-2003 Norbert Andres <nandres@web.de>
   Copyright (C) 2002-2003 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2002 Laurent Montel <montel@kde.org>
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#ifndef KEXI_CSVIMPORTDIALOG_H
#define KEXI_CSVIMPORTDIALOG_H

#include <QList>
#include <QRegularExpression>
#include <QPixmap>
#include <QTextStream>
#include <QEvent>
#include <QModelIndex>
#include <QElapsedTimer>

#include <KAssistantDialog>

#include <KDbTristate>
#include <KDbPreparedStatement>

#include <config-kexi.h>
#include "kexicsvimportoptionsdlg.h"

class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QLabel;
class QTableView;
class QTreeView;
class QFile;
class QStackedWidget;
class QProgressDialog;
class QProgressBar;
class QRadioButton;
class QSpinBox;
class KComboBox;
class KPageWidgetItem;

class KDbConnection;
class KDbTableSchema;

class KexiCSVCommentWidget;
class KexiCSVDelimiterWidget;
class KexiCSVTextQuoteComboBox;
class KexiCSVInfoLabel;
class KexiProject;
class KexiCSVImportDialogModel;
class KexiCSVImportDialogItemDelegate;
#ifdef KEXI_USE_KFILEWIDGET
class KexiFileWidget;
#else
class KexiFileRequester;
#endif
class KexiNameWidget;
class KexiProjectNavigator;
class KexiFieldListModel;

namespace KexiPart {
class Item;
}

/**
 * @short Kexi CSV import dialog
 *
 * This is temporary solution for Kexi CSV import,
 * based on kspread/dialogs/kspread_dlg_csv.h, cc.
 *
 * Provides dialog for managing CSV (comma separated value) data.
 *
 * Currently KexiCSVImportDialog is used for converting text into columns,
 * inserting text file and pasting text from clipboard, where conversion
 * from CSV (comma separated value) data is is all required.
 * The different purposed mentioned above is determined
 * using mode, which can be Column, File, or Clipboard respectively.
*/
class KexiCSVImportDialog : public KAssistantDialog
{
    Q_OBJECT

public:
    enum Mode { Clipboard, File /*, Column*/ };
    enum Header { TEXT, NUMBER, DATE, CURRENCY };

    //! @todo what about making it kexidb-independent?
    explicit KexiCSVImportDialog(Mode mode, QWidget *parent = 0);

    virtual ~KexiCSVImportDialog();

    bool canceled() const;

protected:
    virtual bool eventFilter(QObject *watched, QEvent *e);
    bool openData();
    virtual void accept();
    virtual void reject();

private:
    //! Used in emergency by accept()
    //! @note @a partItemForSavedTable is IN-OUT
    void dropDestinationTable(KexiProject* project, KexiPart::Item* &partItemForSavedTable);

    //! Used in emergency by accept()
    //! @note @a partItemForSavedTable is IN-OUT
    void raiseErrorInAccept(KexiProject* project, KexiPart::Item* &partItemForSavedTable);

    QGridLayout* MyDialogLayout;
    QHBoxLayout* Layout1;
    KexiCSVImportDialogModel *m_table;
    KexiCSVImportDialogItemDelegate *m_tableItemDelegate;
    QTableView *m_tableView;
    KexiCSVDelimiterWidget* m_delimiterWidget;
    KexiCSVCommentWidget* m_commentWidget;
    bool m_detectDelimiter; //!< true if delimiter should be detected
                            //!< (true by default, set to false if user sets delimiter)
    QLabel* m_formatLabel;
    KComboBox* m_formatCombo;
    QSpinBox *m_startAtLineSpinBox;
    KexiCSVTextQuoteComboBox* m_comboQuote;
    QLabel* m_startAtLineLabel;
    QLabel* TextLabel2;
    QCheckBox* m_ignoreDuplicates;
    QCheckBox* m_1stRowForFieldNames;
    QCheckBox* m_primaryKeyField;
#ifdef KEXI_USE_KFILEWIDGET
    KexiFileWidget *m_openFileWidget;
#else
    QWidget *m_openFileWidget;
    KexiFileRequester *m_openFileRequester;
#endif
    QWidget *m_optionsWidget;
    QWidget *m_saveMethodWidget;
    KPageWidgetItem *m_openFilePage;
    KPageWidgetItem *m_optionsPage;
    KPageWidgetItem *m_saveMethodPage;
    KPageWidgetItem *m_chooseTablePage;
    QRadioButton *m_newTableOption;
    QRadioButton *m_existingTableOption;

    QStackedWidget *m_tableNameWidget;
    KPageWidgetItem *m_tableNamePage;
    KexiNameWidget *m_newTableWidget;
    KexiProjectNavigator *m_tablesList;
    QTreeView *m_fieldsListView;
    QLabel *m_tableCaptionLabel;
    QLabel *m_tableNameLabel;
    QLabel *m_recordCountLabel;
    QLabel *m_colCountLabel;

    QWidget *m_importWidget;
    KPageWidgetItem *m_importPage;
    KexiCSVInfoLabel *m_fromLabel;
    KexiCSVInfoLabel *m_toLabel;
    QLabel *m_importProgressLabel;

    void detectTypeAndUniqueness(int row, int col, const QString& text);
    void setText(int row, int col, const QString& text, bool inGUI);

    /*! Parses date from \a text and stores into \a date.
     m_dateRegExp is used for clever detection;
     if '/' separated is found, it's assumed the format is american mm/dd/yyyy.
     This function supports omitted zeros, so 1/2/2006 is parsed properly too.
     \return true on success. */
    bool parseDate(const QString& text, QDate& date);

    /*! Parses time from \a text and stores into \a date.
     m_timeRegExp1 and m_timeRegExp2 are used for clever detection;
     both hh:mm:ss and hh:mm are supported.
     This function supports omitted zeros, so 1:2:3 is parsed properly too.
     \return true on success. */
    bool parseTime(const QString& text, QTime& time);

    /*! Called after the first fillTable() when number of rows is unknown. */
    void adjustRows(int iRows);

    int  getHeader(int col);
    QString getText(int row, int col);
    void updateColumn(int col);
    bool isPrimaryKeyAllowed(int col);
    void setPrimaryKeyIcon(int column, bool set);
    void updateRowCountInfo();
    tristate loadRows(QString &field, int &row, int &columnm, int &maxColumn, bool inGUI);

    /*! Detects delimiter by looking at first 4K bytes of the data. Used by loadRows().
    The used algorithm:
    1. Look byte by byte and locate special characters that can be delimiters.
      Special fact is taken into account: if there are '"' quotes used for text values,
      delimiters that follow directly the closing quote has higher priority than the one
      that follows other character. We do not assume that every text value is quoted.
      Summing up, there is following hierarchy (from highest to lowest):
      quote+tab, quote+semicolon, quote+comma, tab, semicolon, comma.
      Space characters are skipped. Text inside quotes is skipped, as well as double
      (escaped) quotes.
    2. While scanning the data, for every row following number of tabs, semicolons and commas
      (only these outside of the quotes) are computed. On every line the values are appended
      to a separate list (QList<int>).
    3. After scanning, all the values are checked on the QList<int> of tabs.
      If the list has more one element (so there was more than one row) and all the values
      (numbers of tabs) are equal, it's very probable the tab is a delimiter.
      So, this character is returned as a delimiter.
      3a. The same algorithm as in 3. is performed for semicolon character.
      3b. The same algorithm as in 3. is performed for comma character.
    4. If the step 3. did not return a delimiter, a character found in step 1. with
      the highest priority is retured as delimiter. */
    QString detectDelimiterByLookingAtFirstBytesOfFile(QTextStream *inputStream);

    /*! Callback, called whenever row is loaded in loadRows(). When inGUI is true,
    nothing is performed, else database buffer is written back to the database. */
    bool saveRow(bool inGUI);

    //! @return date built out of @a y, @a m, @a d parts,
    //! taking m_minimumYearFor100YearSlidingWindow into account
    QDate buildDate(int y, int m, int d) const;

    //! Updates size of m_columnNames and m_changedColumnNames if needed
    void updateColumnVectorSize();

    QPushButton* configureButton() const;

    bool m_parseComments;
    bool m_canceled;
    bool m_adjustRows;
    int m_startline;
    QChar m_textquote;
    QChar m_commentSymbol;
    QString m_clipboardData;
    QByteArray m_fileArray;
    Mode m_mode;

    QRegularExpression m_dateRegExp, m_timeRegExp1, m_timeRegExp2, m_fpNumberRegExp1, m_fpNumberRegExp2;
    bool m_columnsAdjusted; //!< to call adjustColumn() only once
    bool m_1stRowForFieldNamesDetected; //!< used to force rerun fillTable() after 1st row
    bool m_firstFillTableCall; //!< used to know whether it's 1st fillTable() call
    bool m_blockUserEvents;
    int m_primaryKeyColumn; //!< index of column with PK assigned (-1 if none)
    int m_maximumRowsForPreview;
    int m_maximumBytesForPreview;
    /*! The minimum year for the "100 year sliding date window": range of years that defines
     where any year expressed as two digits falls. Example: for date window from 1930 to 2029,
     two-digit years between 0 and 29 fall in the 2000s, and two-digit years between 30 and 99 fall in the 1900s.
     The default is 1930. */
    int m_minimumYearFor100YearSlidingWindow;

    QPixmap m_pkIcon;
    QString m_fname;
    QFile* m_file;
    QTextStream *m_inputStream; //!< used in loadData()
    KexiCSVImportOptions m_options;
    QProgressDialog *m_loadingProgressDlg;
    QProgressBar *m_importingProgressBar;
    bool m_dialogCanceled;
    KexiCSVInfoLabel *m_infoLbl;
    KDbConnection *m_conn; //!< (temp) database connection used for importing
    KexiFieldListModel *m_fieldsListModel;
    KDbTableSchema *m_destinationTableSchema;  //!< (temp) dest. table schema used for importing
    KDbPreparedStatement m_importingStatement;
    QList<QVariant> m_dbRowBuffer; //!< (temp) used for importing
    bool m_implicitPrimaryKeyAdded; //!< (temp) used for importing
    bool m_allRowsLoadedInPreview; //!< we need to know whether all rows were loaded or it's just a partial data preview
    bool m_stoppedAt_MAX_BYTES_TO_PREVIEW; //!< used to compute m_allRowsLoadedInPreview
    const QString m_stringNo, m_stringI18nNo, m_stringFalse, m_stringI18nFalse; //!< used for importing boolean values
    int m_prevColumnForSetText; //!< used for non-gui tracking of skipped clolumns,
                                //!< so can be saved to the database,
                                //!< e.g. first three columns are saved for ,,,"abc" line in the CSV data
    QElapsedTimer m_elapsedTimer; //!< Used to update progress
    qint64 m_elapsedMs;

    void createImportMethodPage();
    void createOptionsPage();
    void createFileOpenPage();
    void createTableNamePage();
    void createImportPage();

    KDbPreparedStatementParameters m_valuesToInsert;
    KexiPart::Item* m_partItemForSavedTable;
    bool m_importInProgress;
    bool m_importCanceled;
    class Private;
    Private * const d;

public Q_SLOTS:
    virtual void next();

private Q_SLOTS:
    void fillTable();
    void fillTableLater();
    void initLater();
    void formatChanged(int id);
    void delimiterChanged(const QString& delimiter);
    void commentSymbolChanged(const QString& commentSymbol);
    void startlineSelected(int line);
    void textquoteSelected(int);
    void currentCellChanged(const QModelIndex &cur, const QModelIndex &prev);
    void ignoreDuplicatesChanged(int);
    void slot1stRowForFieldNamesChanged(int state);
    void optionsButtonClicked();
    void slotPrimaryKeyFieldToggled(bool on);
    void slotCurrentPageChanged(KPageWidgetItem *page, KPageWidgetItem *prev);
    void slotShowSchema(KexiPart::Item *item);
    void import();
};

#endif
