/******************************************************************************
** Copyright (C) 2010 Hugues Bruant <hugues.bruant@gmail.com>
** All rights reserved.
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation.
** See <http://www.gnu.org/licenses/> or GPL.txt included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
******************************************************************************/

#ifndef EXPENSESHARING_H
#define EXPENSESHARING_H

#include <QUrl>
#include <QMainWindow>

#include "ui_expensesharing.h"

class ExpenseGroup;

class QNetworkAccessManager;

class ExpenseSharing : public QMainWindow, private Ui::ExpenseSharing
{
    Q_OBJECT

public:
    explicit ExpenseSharing(QWidget *parent = 0);
    ~ExpenseSharing();

public slots:
    bool open(const QUrl& url);
    bool open(const QString& filename);

private slots:
    void on_actionOpen_triggered();
    void on_actionOpenUrl_triggered();
    void on_actionClearRecent_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionAdd_triggered();
    void on_actionHelp_triggered();
    void on_actionAbout_triggered();

    void openRecentTriggered();

    void summaryChanged();
    void currentPersonChanged(const QModelIndex& idx);
    void on_lvPersons_customContextMenuRequested(const QPoint& pos);
    void on_tvExpenseDetails_customContextMenuRequested(const QPoint& pos);

private:
    void updateRecentMenu();
    void setCurrentFileName(const QUrl& url);

    QUrl m_url;
    ExpenseGroup *m_group;

    QNetworkAccessManager *m_manager;
};

#endif // EXPENSESHARING_H
