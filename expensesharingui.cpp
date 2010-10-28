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

#include "expensesharingui.h"

#include "person.h"
#include "expense.h"
#include "expensegroup.h"
#include "expensesharing.h"
#include "expensemodel.h"
#include "personmodel.h"

#include "newpersondialog.h"
#include "newexpensedialog.h"

#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QNetworkReply>
#include <QAuthenticator>

class NetworkWaiterUI : public NetworkWaiter {
public:
    NetworkWaiterUI(QNetworkReply *reply, QWidget *p = 0)
        : NetworkWaiter(reply, p)
        , m_dialog(tr("Transferring %1...").arg(reply->url().toString()),
                   tr("Abort transfer"), 0, -1, p) {
        m_dialog.setAutoClose(true);
    }

    bool wait() {
        m_dialog.exec();
        return m_dialog.wasCanceled();
    }

    void finished() {
        m_dialog.reset();
    }

    void error(QNetworkReply::NetworkError error) {
        QMessageBox::warning(&m_dialog,
                             tr("Transfer failed"),
                             tr("%2 [%1]")
                                .arg(error)
                                .arg(m_reply->errorString()));
        m_dialog.reset();
    }

    void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth) {
        if (reply != m_reply)
            return;
        // TODO : proper credential input
        QString usr = QInputDialog::getText(&m_dialog, tr("Auth required"),
                                            tr("Username"));
        QString pwd = QInputDialog::getText(&m_dialog, tr("Auth required"),
                                            tr("Password"));
        auth->setUser(usr);
        auth->setPassword(pwd);
    }

    void downloadProgress(qint64 received, qint64 total) {
        m_dialog.setMaximum(total);
        m_dialog.setValue(received);
    }

    void uploadProgress(qint64 sent, qint64 total) {
        m_dialog.setMaximum(total);
        m_dialog.setValue(sent);
    }

private:
    QProgressDialog m_dialog;
};

class ExpenseSharingUIPrivate : public ExpenseSharing {
public:
    ExpenseSharingUIPrivate(ExpenseSharingUI *p)
        : ExpenseSharing(p) {
    }

protected:
    NetworkWaiter* createWaiter(QNetworkReply *reply) {
        return new NetworkWaiterUI(reply, qobject_cast<QWidget*>(parent()));
    }
};

ExpenseSharingUI::ExpenseSharingUI(QWidget *parent)
    : QMainWindow(parent)
    , m_d(new ExpenseSharingUIPrivate(this)) {
    setupUi(this);

    menu_Edit->addAction(m_d->createRedoAction(this, tr("Redo ")));
    menu_Edit->addAction(m_d->createUndoAction(this, tr("Undo ")));

    tvExpenseDetails->setShowGrid(false);
    tvExpenseDetails->setModel(new ExpenseModel(m_d->expenseGroup(), this));
    tvExpenseDetails->setContextMenuPolicy(Qt::CustomContextMenu);

    lvPersons->setModel(new PersonModel(m_d->expenseGroup(), this));
    lvPersons->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(lvPersons->selectionModel(),
            SIGNAL( currentChanged(QModelIndex, QModelIndex) ),
            this,
            SLOT  ( currentPersonChanged(QModelIndex) ));

    connect(m_d , SIGNAL( urlChanged(const QUrl&) ),
            this, SLOT  ( urlChanged(const QUrl&) ));

    connect(m_d , SIGNAL( summaryChanged() ),
            this, SLOT  ( summaryChanged() ));

    updateRecentMenu();
    summaryChanged();
}

ExpenseSharingUI::~ExpenseSharingUI() {
}

ExpenseSharing* ExpenseSharingUI::expenseSharing() const {
    return m_d;
}

void ExpenseSharingUI::urlChanged(const QUrl& url) {
    QSettings s;
    QStringList l = s.value("recentFiles", QStringList()).toStringList();
    int maxEntries = s.value("maxRecentFileEntries", 10).toUInt();
    l.removeAll(url.toString());
    l.prepend(url.toString());
    while (l.count() > maxEntries)
        l.removeLast();
    s.setValue("recentFiles", l);
    updateRecentMenu();
}

void ExpenseSharingUI::updateRecentMenu() {
    QList<QAction*> l = menuOpenRecent->actions();
    foreach (QAction *a, l) {
        if (a != actionClearRecent && !a->isSeparator()) {
            menuOpenRecent->removeAction(a);
        }
    }
    QStringList urls = QSettings().value("recentFiles").toStringList();
    for (int n = 0; n < urls.count(); ++n) {
        QString fn = urls.at(n);
        QString prefix;
        if (n < 10)
            prefix = QString("&") + QString::number(n) + " ";
        menuOpenRecent->addAction(prefix + fn, this,
                                  SLOT( openRecentTriggered() ))
            ->setData(fn);
    }
}

void ExpenseSharingUI::openRecentTriggered() {
    QAction *a = qobject_cast<QAction*>(sender());
    if (a)
        open(QUrl(a->data().toString()));
}

bool ExpenseSharingUI::open(const QUrl& url) {
    bool ok = m_d->open(url);
    if (ok)
        tvExpenseDetails->resizeColumnsToContents();
    return ok;
}

bool ExpenseSharingUI::open(const QString& filename) {
    return filename.count() ? m_d->open(QUrl::fromLocalFile(filename)) : false;
}

void ExpenseSharingUI::on_actionOpen_triggered() {
    QString fn =
    QFileDialog::getOpenFileName(this,
                                 tr("Open expense log..."),
                                 QString(),
                                 "XSEM files (*.xsem);;All files (*)");
    open(fn);
}

void ExpenseSharingUI::on_actionOpenUrl_triggered() {
    QString url = QInputDialog::getText(this, tr("Open URL..."), tr("URL"));
    open(QUrl::fromUserInput(url));
}

void ExpenseSharingUI::on_actionClearRecent_triggered() {
    QSettings().remove("recentFiles");
    updateRecentMenu();
}

void ExpenseSharingUI::on_actionSave_triggered() {
    if (m_d->url().isValid()) {
        m_d->save();
    } else {
        on_actionSaveAs_triggered();
    }
}

void ExpenseSharingUI::on_actionSaveAs_triggered() {
    QString fn =
    QFileDialog::getSaveFileName(this,
                                 tr("Save expense log..."),
                                 m_d->url().toLocalFile(),
                                 "XSEM files (*.xsem);;All files (*)");
    if (fn.count())
        m_d->saveAs(fn);
}

void ExpenseSharingUI::on_actionAdd_triggered() {
    Expense *e = NewExpenseDialog::getExpense(m_d->expenseGroup()->persons(), this);
    if (e)
        m_d->addExpense(e);
}

void ExpenseSharingUI::on_actionHelp_triggered() {
    QMessageBox::information(this,
                             tr("Sorry"),
                             tr("No documentation yet..."),
                             QMessageBox::Ok);
}

void ExpenseSharingUI::on_actionAbout_triggered() {
    QMessageBox::information(this,
                             tr("About ExpenseSharingUI"),
                             tr("A simple program for Shared Expense Management\n"
                                "Brought to you by Hugues Bruant."),
                             QMessageBox::Ok);
}

void ExpenseSharingUI::summaryChanged() {
    currentPersonChanged(lvPersons->currentIndex());
}

void ExpenseSharingUI::currentPersonChanged(const QModelIndex& idx) {
    lePersonExpenses->clear();
    lePersonExpenses->setEnabled(idx.isValid());
    lePersonGroupDebts->clear();
    lePersonGroupDebts->setEnabled(idx.isValid());

    if (idx.isValid()) {
        const int row = idx.row();
        const ExpenseGroup::Summary& s = m_d->expenseGroup()->summary();
        lePersonExpenses->setText(QString::number(s.expenses.at(row)));
        lePersonGroupDebts->setText(QString::number(s.groupDebts.at(row)));
    }
}

void ExpenseSharingUI::on_lvPersons_customContextMenuRequested(const QPoint& pos) {
    QMenu m(this);
    QAction *add = m.addAction("New person...");
    QAction *rem = m.addAction("Remove person");

    QModelIndex idx = lvPersons->indexAt(pos);
    rem->setEnabled(idx.isValid());

    QAction *a = m.exec(lvPersons->mapToGlobal(pos));
    if (a == add) {
        Person *p = NewPersonDialog::getPerson(this);
        if (p)
            m_d->addPerson(p);
    } else if (a == rem) {
        m_d->removePerson(m_d->expenseGroup()->persons().at(idx.row()));
    }
}

void ExpenseSharingUI::on_tvExpenseDetails_customContextMenuRequested(const QPoint& pos) {
    QMenu m(this);
    QAction *add = m.addAction("New expense...");
    QAction *rem = m.addAction("Remove expense");

    QModelIndex idx = tvExpenseDetails->indexAt(pos);
    rem->setEnabled(idx.isValid());

    QAction *a = m.exec(tvExpenseDetails->mapToGlobal(pos));
    if (a == add) {
        on_actionAdd_triggered();
    } else if (a == rem) {
        m_d->removeExpense(m_d->expenseGroup()->expenses().at(idx.row()));
    }
}
