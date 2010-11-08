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
#include "expensesharing.h"
#include "expensemodel.h"
#include "personmodel.h"

#include "newpersondialog.h"
#include "newexpensedialog.h"

#include <QAuthenticator>
#include <QBuffer>
#include <QCoreApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUndoCommand>

NetworkWaiter::NetworkWaiter(QNetworkReply *reply, QObject *p)
    : QObject(p), m_reply(reply) {
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
            SLOT  ( finished() ));
}

bool NetworkWaiter::wait() {
    while (!m_reply->isFinished())
        QCoreApplication::processEvents();
    return false;
}

void NetworkWaiter::finished() {
}

void NetworkWaiter::error(QNetworkReply::NetworkError error) {
    Q_UNUSED(error)
}

void NetworkWaiter::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth) {
    Q_UNUSED(reply)
    Q_UNUSED(auth)
}


void NetworkWaiter::NetworkWaiter::uploadProgress(qint64 sent, qint64 total) {
    Q_UNUSED(sent)
    Q_UNUSED(total)
}

void NetworkWaiter::downloadProgress(qint64 received, qint64 total) {
    Q_UNUSED(received)
    Q_UNUSED(total)
}

////////////////////////////////////////////////////////////////////////////////

class ExpenseSharingCommand : public QUndoCommand {
public:
    enum Command {
        CNone     = 0,
        CAdd      = 1,
        CRemove   = 2,

        CMask     = 0x0FFF,
        TMask     = 0xF000,

        TPerson   = 0x0000,
        TExpense  = 0x8000,
    };

    ExpenseSharingCommand(int cmd, QList<void*> param, ExpenseGroup *group)
        : m_done(false), m_command(cmd), m_param(param), m_group(group) {
        setText(commandString());
    }

    ~ExpenseSharingCommand() {
        if ((m_done && ((m_command & CMask) == CRemove))
            || (!m_done && ((m_command & CMask) == CAdd))) {
            if ((m_command & TMask) == TExpense) {
                foreach (void *e, m_param)
                    delete static_cast<Expense*>(e);
            } else {
                foreach (void *p, m_param)
                    delete static_cast<Person*>(p);
            }
        }
    }

    QString commandString() {
        QString s = (m_command & CMask) == CAdd ? "Add" : "Remove";
        s += " ";
        if (m_param.count() == 1) {
            s += (m_command & TMask) == TExpense ? "expense:" : "person:";
            s += (m_command & TMask) == TExpense
                 ? static_cast<Expense*>(m_param.first())->description()
                 : static_cast<Person*>(m_param.first())->name();
        } else {
            s += QString::number(m_param.count());
            s += " ";
            s += (m_command & TMask) == TExpense ? "expenses" : "persons";
        }
        return s;
    }

    virtual void redo() {
        m_done = true;
        if (m_command & TExpense) {
            switch (m_command & CMask) {
            case CAdd:
                foreach (void *e, m_param)
                    m_group->addExpense(static_cast<Expense*>(e));
                break;
            case CRemove:
                foreach (void *e, m_param)
                    m_group->removeExpense(static_cast<Expense*>(e));
                break;
            default:
                break;
            }
        } else {
            switch (m_command & CMask) {
            case CAdd:
                foreach (void *p, m_param)
                    m_group->addPerson(static_cast<Person*>(p));
                break;
            case CRemove:
                foreach (void *p, m_param)
                    m_group->removePerson(static_cast<Person*>(p));
                break;
            default:
                break;
            }
        }
    }

    virtual void undo() {
        m_done = false;
        if (m_command & TExpense) {
            switch (m_command & CMask) {
            case CAdd:
                foreach (void *e, m_param)
                    m_group->removeExpense(static_cast<Expense*>(e));
                break;
            case CRemove:
                foreach (void *e, m_param)
                    m_group->addExpense(static_cast<Expense*>(e));
                break;
            default:
                break;
            }
        } else {
            switch (m_command & CMask) {
            case CAdd:
                foreach (void *p, m_param)
                    m_group->removePerson(static_cast<Person*>(p));
                break;
            case CRemove:
                foreach (void *p, m_param)
                    m_group->addPerson(static_cast<Person*>(p));
                break;
            default:
                break;
            }
        }
    }

private:
    bool m_done;
    int m_command;
    QList<void*> m_param;
    ExpenseGroup *m_group;
};

////////////////////////////////////////////////////////////////////////////////

ExpenseSharing::ExpenseSharing(QObject *parent)
    : QObject(parent)
    , m_group(new ExpenseGroup(this))
    , m_manager(new QNetworkAccessManager(this)) {

    connect(m_group, SIGNAL( summaryChanged() ),
            this   , SIGNAL( summaryChanged() ));

    connect(&m_commands, SIGNAL( cleanChanged(bool) ),
            this       , SLOT  ( cleanChanged(bool) ));

}

QUrl ExpenseSharing::url() const {
    return m_url;
}

void ExpenseSharing::setUrl(const QUrl& url) {
    m_url = url;
    emit urlChanged(url);
}

const ExpenseGroup* ExpenseSharing::expenseGroup() const {
    return m_group;
}

QString ExpenseSharing::errorString() const {
    return m_error;
}

bool ExpenseSharing::isModified() const {
    return !m_commands.isClean();
}

void ExpenseSharing::cleanChanged(bool clean) {
    emit modificationChanged(!clean);
}

bool ExpenseSharing::canUndo() {
    return m_commands.canUndo();
}

bool ExpenseSharing::canRedo() {
    return m_commands.canRedo();
}

QAction* ExpenseSharing::createUndoAction(QObject *parent, const QString& prefix) const {
    return m_commands.createUndoAction(parent, prefix);
}

QAction* ExpenseSharing::createRedoAction(QObject *parent, const QString& prefix) const {
    return m_commands.createRedoAction(parent, prefix);
}

bool ExpenseSharing::open(const QUrl& url) {
    if (!url.isValid()) {
        m_error = tr("Invalid URL.");
        return false;
    }
    bool ok = false;
    if (url.scheme() != "file") {
        if (m_manager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
            m_error = tr("Network not accessible.");
            return false;
        }
        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("User-Agent", "ExpenseSharing");
        QNetworkReply *reply = m_manager->get(request);
        NetworkWaiter *nw = createWaiter(reply);
        ok = !nw->wait();
        if (ok) {
            ok &= m_group->load(reply);
            if (!ok)
                m_error = tr("Not a valid XSEM file.");
        } else {
            m_error = tr("Remote read failed.");
        }
        reply->deleteLater();
        delete nw;
    } else {
        ok = m_group->load(url.toLocalFile());
        if (!ok)
            m_error = tr("Not a valid XSEM file.");
    }
    if (ok) {
        m_commands.clear();
        setUrl(url);
    }
    return ok;
}

bool ExpenseSharing::open(const QString& filename) {
    return filename.count() ? open(QUrl::fromLocalFile(filename)) : false;
}

bool ExpenseSharing::save() {
    bool ok = false;
    if (m_url.scheme() == "file") {
        ok = m_group->save(m_url.toLocalFile());
    } else {
        // try to write to distant storage
        if (m_manager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
            m_error = tr("Network not accessible.");
            return false;
        }
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        m_group->save(&buffer);
        buffer.seek(0);
        QNetworkRequest request;
        request.setUrl(m_url);
        request.setRawHeader("User-Agent", "ExpenseSharing");
        QNetworkReply *reply = m_manager->put(request, &buffer);
        NetworkWaiter *nw = createWaiter(reply);
        ok = !nw->wait();
        if (!ok)
            m_error = tr("Remote write failed.");
    }
    if (ok)
        m_commands.setClean();
    return ok;
}

bool ExpenseSharing::saveAs(const QUrl& url) {
    if (!url.isValid())
        return false;
    setUrl(url);
    return save();
}

bool ExpenseSharing::saveAs(const QString& filename) {
    return filename.count() ? saveAs(QUrl::fromLocalFile(filename)) : false;
}

void ExpenseSharing::close() {
    m_group->clearExpenses();
    m_group->clearPersons();
    m_commands.clear();
    setUrl(QUrl());
    emit summaryChanged();
    emit modificationChanged(false);
}

void ExpenseSharing::addPerson(Person *person) {
    m_commands.push(
        new ExpenseSharingCommand(
                ExpenseSharingCommand::CAdd | ExpenseSharingCommand::TPerson,
                QList<void*>() << person,
                m_group));
}

void ExpenseSharing::removePerson(Person *person) {
    m_commands.push(
        new ExpenseSharingCommand(
                ExpenseSharingCommand::CRemove | ExpenseSharingCommand::TPerson,
                QList<void*>() << person,
                m_group));
}

void ExpenseSharing::clearPersons() {
    QList<void*> l;
    foreach (Person *p, m_group->persons())
        l << p;
    m_commands.push(
        new ExpenseSharingCommand(
                ExpenseSharingCommand::CRemove | ExpenseSharingCommand::TPerson,
                l,
                m_group));
}

void ExpenseSharing::addExpense(Expense *expense) {
    m_commands.push(
        new ExpenseSharingCommand(
                ExpenseSharingCommand::CAdd | ExpenseSharingCommand::TExpense,
                QList<void*>() << expense,
                m_group));
}

void ExpenseSharing::removeExpense(Expense *expense) {
    m_commands.push(
        new ExpenseSharingCommand(
                ExpenseSharingCommand::CRemove | ExpenseSharingCommand::TExpense,
                QList<void*>() << expense,
                m_group));
}

void ExpenseSharing::clearExpenses() {
    QList<void*> l;
    foreach (Expense *e, m_group->expenses())
        l << e;
    m_commands.push(
        new ExpenseSharingCommand(
                ExpenseSharingCommand::CRemove | ExpenseSharingCommand::TExpense,
                l,
                m_group));
}

void ExpenseSharing::undo() {
    m_commands.undo();
}

void ExpenseSharing::redo() {
    m_commands.redo();
}

