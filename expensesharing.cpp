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

#include "expensesharing.h"

#include "person.h"
#include "expense.h"
#include "expensegroup.h"
#include "expensemodel.h"
#include "personmodel.h"

#include "newpersondialog.h"
#include "newexpensedialog.h"

#include <cfloat>
#include <QDebug>
#include <QBuffer>
#include <QSettings>
#include <QAuthenticator>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

class NetworkWaiter : public QProgressDialog {
    Q_OBJECT
public:
    NetworkWaiter(QNetworkReply *reply, QWidget *p = 0)
        : QProgressDialog(tr("Transferring %1...").arg(reply->url().toString()),
                          tr("Abort transfer"),
                          0, -1, p)
        , m_reply(reply) {
        setAutoClose(true);
        connect(reply->manager(),
                SIGNAL( authenticationRequired(QNetworkReply*, QAuthenticator*) ),
                SLOT  ( authenticationRequired(QNetworkReply*, QAuthenticator*) ));
        connect(reply,
                SIGNAL( error(QNetworkReply::NetworkError) ),
                SLOT  ( error(QNetworkReply::NetworkError) ));
        connect(reply,
                SIGNAL( uploadProgress(qint64, qint64) ),
                SLOT  ( uploadProgress(qint64, qint64) ));
        connect(reply,
                SIGNAL( downloadProgress(qint64, qint64) ),
                SLOT  ( downloadProgress(qint64, qint64) ));
        connect(reply,
                SIGNAL( finished() ),
                SLOT  ( reset() ));
    }

public slots:
    bool wait() {
        exec();
        bool err = wasCanceled() && !m_reply->isFinished();
        if (err)
            m_reply->abort();
        return err;
    }

private slots:
    void error(QNetworkReply::NetworkError error) {
        QMessageBox::warning(this,
                             tr("Transfer failed"),
                             tr("%2 [%1]")
                                .arg(error)
                                .arg(m_reply->errorString()));
        reset();
    }

    void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth) {
        if (reply != m_reply)
            return;
        // TODO : proper credential input
        QString usr = QInputDialog::getText(this, tr("Auth required"), tr("Username"));
        QString pwd = QInputDialog::getText(this, tr("Auth required"), tr("Password"));
        auth->setUser(usr);
        auth->setPassword(pwd);
    }

    void downloadProgress(qint64 received, qint64 total) {
        setMaximum(total);
        setValue(received);
    }

    void uploadProgress(qint64 sent, qint64 total) {
        setMaximum(total);
        setValue(sent);
    }

private:
    QNetworkReply *m_reply;
};

ExpenseSharing::ExpenseSharing(QWidget *parent)
    : QMainWindow(parent)
    , m_group(new ExpenseGroup(this))
    , m_manager(new QNetworkAccessManager(this)) {
    setupUi(this);

    tvExpenseDetails->setShowGrid(false);
    tvExpenseDetails->setModel(new ExpenseModel(m_group));
    tvExpenseDetails->setContextMenuPolicy(Qt::CustomContextMenu);

    lvPersons->setModel(new PersonModel(m_group));
    lvPersons->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(lvPersons->selectionModel(),
            SIGNAL( currentChanged(QModelIndex, QModelIndex) ),
            this,
            SLOT  ( currentPersonChanged(QModelIndex) ));

    connect(m_group, SIGNAL( summaryChanged() ),
            this   , SLOT  ( summaryChanged() ));

    updateRecentMenu();
    summaryChanged();
}

ExpenseSharing::~ExpenseSharing() {
}

void ExpenseSharing::setCurrentFileName(const QUrl& url) {
    m_url = url;

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

void ExpenseSharing::updateRecentMenu() {
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

void ExpenseSharing::openRecentTriggered() {
    QAction *a = qobject_cast<QAction*>(sender());
    if (a)
        open(QUrl(a->data().toString()));
}

bool ExpenseSharing::open(const QUrl& url) {
    if (!url.isValid())
        return false;
    bool ok = false;
    if (url.scheme() != "file") {
        if (m_manager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
            QMessageBox::warning(this, tr("Error"), tr("Network is down."),
                                 QMessageBox::Ok);
            return false;
        }
        // read from distant storage
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("User-Agent", "ExpenseSharing");
        QNetworkReply *reply = m_manager->get(request);
        ok = NetworkWaiter(reply, this).wait() ? false : m_group->load(reply);
        reply->deleteLater();
    } else {
        ok = m_group->load(url.toLocalFile());
    }

    if (ok) {
        tvExpenseDetails->resizeColumnsToContents();
        setCurrentFileName(url);
    }
    return ok;
}

bool ExpenseSharing::open(const QString& filename) {
    return filename.count() ? open(QUrl::fromLocalFile(filename)) : false;
}

void ExpenseSharing::on_actionOpen_triggered() {
    QString fn =
    QFileDialog::getOpenFileName(this,
                                 tr("Open expense log..."),
                                 QString(),
                                 "XSEM files (*.xsem);;All files (*)");
    open(fn);
}

void ExpenseSharing::on_actionOpenUrl_triggered() {
    QString url = QInputDialog::getText(this, tr("Open URL..."), tr("URL"));
    open(QUrl::fromUserInput(url));
}

void ExpenseSharing::on_actionClearRecent_triggered() {
    QSettings().remove("recentFiles");
    updateRecentMenu();
}

void ExpenseSharing::on_actionSave_triggered() {
    if (m_url.isValid()) {
        if (m_url.scheme() == "file") {
            m_group->save(m_url.toLocalFile());
        } else {
            // try to write to distant storage
            if (m_manager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
                QMessageBox::warning(this, tr("Error"), tr("Network is down."),
                                     QMessageBox::Ok);
                return;
            }
            QBuffer buffer;
            buffer.open(QBuffer::ReadWrite);
            m_group->save(&buffer);
            buffer.seek(0);
            QNetworkRequest request;
            request.setUrl(m_url);
            request.setRawHeader("User-Agent", "ExpenseSharing");
            QNetworkReply *reply = m_manager->put(request, &buffer);
            NetworkWaiter(reply, this).wait();
        }
    } else {
        on_actionSaveAs_triggered();
    }
}

void ExpenseSharing::on_actionSaveAs_triggered() {
    QString fn =
    QFileDialog::getSaveFileName(this,
                                 tr("Save expense log..."),
                                 m_url.toLocalFile(),
                                 "XSEM files (*.xsem);;All files (*)");
    if (fn.count()) {
        m_group->save(fn);
        setCurrentFileName(QUrl::fromLocalFile(fn));
    }
}

void ExpenseSharing::on_actionAdd_triggered() {
    Expense *e = NewExpenseDialog::getExpense(m_group->persons(), this);
    if (e)
        m_group->addExpense(e);
}

void ExpenseSharing::on_actionHelp_triggered() {
    QMessageBox::information(this,
                             tr("Sorry"),
                             tr("No documentation yet..."),
                             QMessageBox::Ok);
}

void ExpenseSharing::on_actionAbout_triggered() {
    QMessageBox::information(this,
                             tr("About ExpenseSharing"),
                             tr("A simple program for Shared Expense Management\n"
                                "Brought to you by Hugues Bruant."),
                             QMessageBox::Ok);
}

void ExpenseSharing::summaryChanged() {
    currentPersonChanged(lvPersons->currentIndex());
}

void ExpenseSharing::currentPersonChanged(const QModelIndex& idx) {
    lePersonExpenses->clear();
    lePersonExpenses->setEnabled(idx.isValid());
    lePersonGroupDebts->clear();
    lePersonGroupDebts->setEnabled(idx.isValid());

    if (idx.isValid()) {
        const int row = idx.row();
        const ExpenseGroup::Summary& s = m_group->summary();
        lePersonExpenses->setText(QString::number(s.expenses.at(row)));
        lePersonGroupDebts->setText(QString::number(s.groupDebts.at(row)));
    }
}

void ExpenseSharing::on_lvPersons_customContextMenuRequested(const QPoint& pos) {
    QMenu m(this);
    QAction *add = m.addAction("New person...");
    QAction *rem = m.addAction("Remove person");

    QModelIndex idx = lvPersons->indexAt(pos);
    rem->setEnabled(idx.isValid());

    QAction *a = m.exec(lvPersons->mapToGlobal(pos));
    if (a == add) {
        Person *p = NewPersonDialog::getPerson(this);
        if (p)
            m_group->addPerson(p);
    } else if (a == rem) {
        m_group->removePerson(m_group->persons().at(idx.row()));
    }
}

void ExpenseSharing::on_tvExpenseDetails_customContextMenuRequested(const QPoint& pos) {
    QMenu m(this);
    QAction *add = m.addAction("New expense...");
    QAction *rem = m.addAction("Remove expense");

    QModelIndex idx = tvExpenseDetails->indexAt(pos);
    rem->setEnabled(idx.isValid());

    QAction *a = m.exec(tvExpenseDetails->mapToGlobal(pos));
    if (a == add) {
        on_actionAdd_triggered();
    } else if (a == rem) {
        m_group->removeExpense(m_group->expenses().at(idx.row()));
    }
}

#include "expensesharing.moc"
