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

#include "expensesharingcli.h"

#include "person.h"
#include "expense.h"
#include "expensegroup.h"
#include "expensesharing.h"

#ifdef USE_READLINE
#include <readline/history.h>
#include <readline/readline.h>
#else
#define CMD_BUFFER_SIZE 1024
#endif

#include <QAuthenticator>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTimer>
#include <QThread>

class NetworkWaiterCLI : public NetworkWaiter {
    Q_OBJECT
public:
    NetworkWaiterCLI(QNetworkReply *reply, QObject *p = 0)
        : NetworkWaiter(reply, p) {
    }

    bool wait() {
        return NetworkWaiter::wait();
    }

    void finished() {
        emit output("\rTransfer finished.\n");
    }

    void error(QNetworkReply::NetworkError error) {
        Q_UNUSED(error)
        emit output("\rTransfer failed.\n");
    }

    void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth) {
        NetworkWaiter::authenticationRequired(reply, auth);
    }

    void downloadProgress(qint64 received, qint64 total) {
        emit output(tr("\rtransfered %1/%2 kb")
                        .arg(received / 1000)
                        .arg(total / 1000));
    }

    void uploadProgress(qint64 sent, qint64 total) {
        emit output(tr("\rtransfered %1/%2 kb")
                        .arg(sent / 1000)
                        .arg(total / 1000));
    }

signals:
    void output(const QString& output);

private:
};

static QStringList splitCommandLine(const char *s) {
    QStringList l;
    QString current;
    bool quoted = false;
    bool escaped = false;
    while (*s) {
        if (*s == '\"' && !escaped) {
            quoted = !quoted;
        } else if (*s == '\\' && !escaped) {
            escaped = true;
        } else if (*s <= ' ' && !(quoted || escaped)) {
            l << current;
            current.clear();
        } else {
            escaped = false;
            current += *s;
        }
        ++s;
    }
    if (current.count())
        l << current;
    return l;
}

class CLI : public QThread {
    Q_OBJECT
public:
    CLI(bool interactive, QObject *p = 0)
        : QThread(p) {
        if (interactive)
            start();
    }

    virtual void run() {
        forever {
            QStringList l = getCommand();
            if (l.isEmpty())
                continue;

            QString cmd = l.takeAt(0);
            if (cmd == "q" || cmd == "quit" || cmd == "exit")
                break;

            m_notReady = true;
            emit command(cmd, l);
            while (m_notReady)
                msleep(100);
        }
        qApp->quit();
    }

    QStringList getCommand() {
        QStringList l;
#if USE_READLINE
        char *s = readline("expensesharing> ");
        if (s && *s)
            add_history(s);
        l = splitCommandLine(s);
        free(s);
#else
        printf("expensesharing> ");
        fflush(stdout);
        char buffer[CMD_BUFFER_SIZE];
        char *s = fgets(buffer, CMD_BUFFER_SIZE, stdin);
        if (feof(stdin)) {
            ;
        }
        l = splitCommandLine(s);
#endif
        return l;
    }

signals:
    void command(const QString& cmd, const QStringList& args);

private slots:
    void promptReady() {
        m_notReady = false;
    }

    void output(const QString& out) {
        printf("%s", qPrintable(out));
    }

private:
    volatile bool m_notReady;
};

class ExpenseSharingCLIPrivate : public ExpenseSharing {
public:
    ExpenseSharingCLIPrivate(ExpenseSharingCLI *p)
        : ExpenseSharing(p) {
    }

protected:
    NetworkWaiter* createWaiter(QNetworkReply *reply) {
        NetworkWaiter *w = new NetworkWaiterCLI(reply, parent());
        connect(w       , SIGNAL( output(QString) ),
                parent(), SIGNAL( output(QString) ));
        return w;
    }
};

ExpenseSharingCLI::ExpenseSharingCLI(bool interactive, QObject *p)
    : QObject(p)
    , m_cli(new CLI(interactive, this))
    , m_d(new ExpenseSharingCLIPrivate(this)) {
    if (m_cli) {
        connect(m_cli,
                SIGNAL( command(QString, QStringList) ),
                SLOT  ( command(QString, QStringList) ));
        connect(this , SIGNAL( commandDone() ),
                m_cli, SLOT  ( promptReady() ));
        connect(this , SIGNAL( output(QString) ),
                m_cli, SLOT  ( output(QString) ));
    }
}

ExpenseSharing* ExpenseSharingCLI::expenseSharing() const {
    return m_d;
}

static QUrl urlFromInput(const QString& input) {
    QFileInfo info(input);
    QString cano = info.canonicalFilePath();
    if (cano.count())
        return QUrl::fromLocalFile(cano);
    return QUrl::fromUserInput(input);
}

static Person* findPerson(const QString& s, QList<Person*> l) {
    bool ok = false;
    int n = s.toInt(&ok, 0);

    if (ok && 0 <= n && n < l.count())
        return l.at(n);

    foreach (Person *p, l)
        if (p->name() == s)
            return p;

    return 0;
}

static Expense* findExpense(const QString& s, QList<Expense*> l) {
    bool ok = false;
    int n = s.toInt(&ok, 0);

    if (ok && 0 <= n && n < l.count())
        return l.at(n);

    // TODO: improve matching (use other fields and regexps)
    foreach (Expense *e, l)
        if (e->description() == s)
            return e;

    return 0;
}

static QString expenseToString(Expense *e) {
    return e->description();
}

void ExpenseSharingCLI::command(const QString& cmd, const QStringList& args) {
    if (cmd == "help") {
        if (args.isEmpty()) {
            emit output(
                "Available commands :\n"
                "  open\n"
                "  save\n"
                "  addperson\n"
                "  removeperson\n"
                "  showperson\n"
                "  addexpense\n"
                "  removeexpense\n"
                "  showexpense\n"
                "  undo\n"
                "  redo\n"
                "  quit\n"
                "  \n"
                "Type help <command> for detailed help.\n");
        } else {
            const QString& s = args.at(0);
            if (s == "open" || s == "o")
                emit output(tr("\n"));
            else if (s == "save" || s == "s")
                emit output(tr("\n"));
            else if (s == "addperson" || s == "ap")
                emit output(tr("\n"));
            else if (s == "removeperson" || s == "rp")
                emit output(tr("\n"));
            else if (s == "showperson" || s == "sp" )
                emit output(tr("\n"));
            else if (s == "addexpense" || s == "ax")
                emit output(tr("\n"));
            else if (s == "removeexpense" || s == "rx")
                emit output(tr("\n"));
            else if (s == "showexpense" || s == "sx")
                emit output(tr("\n"));
            else if (s == "undo" || s == "u")
                emit output(tr("\n"));
            else if (s == "redo" || s == "r")
                emit output(tr("\n"));
            else
                emit output(tr("Unrecognized command : \"%1\"\n").arg(s));
        }
    } else if (cmd == "open" || cmd == "o") {
        if (args.count() != 1) {
            emit output("open expects exactly one argument.\n");
        } else {
            if (!m_d->open(urlFromInput(args.at(0))))
                emit output(m_d->errorString() + "\n");
        }
    } else if (cmd == "save" || cmd == "s") {
        if (args.count() > 1) {
            emit output("save expects at most one argument.\n");
        } else {
            bool ok = args.isEmpty() ? m_d->save() : m_d->saveAs(urlFromInput(args.at(0)));
            if (!ok)
                emit output(m_d->errorString() + "\n");
        }
    } else if (cmd == "addperson" || cmd == "ap") {
        if (args.count() != 1) {
            emit output("addperson expects exactly one argument.\n");
        } else {
            Person *p = new Person(args.at(0));
            m_d->addPerson(p);
        }
    } else if (cmd == "removeperson" || cmd == "rp") {
        if (args.count() != 1) {
            emit output("removeperson expects exactly one argument.\n");
        } else {
            Person *person = findPerson(args.at(0),
                                        m_d->expenseGroup()->persons());
            if (person)
                m_d->removePerson(person);
            else
              emit output(tr("No match for person \"%1\"\n").arg(args.at(0)));
        }
    } else if (cmd == "showperson" || cmd == "sp") {
        if (args.count() > 1) {
            emit output("showperson expects at most one argument.\n");
        } else if (args.count()) {
            Person *person = findPerson(args.at(0),
                                        m_d->expenseGroup()->persons());
            if (person)
                emit output(tr("%1 : \n"
                               "  expense = %2\n"
                               "  group debt = %3\n"
                               "\n")
                            .arg(person->name())
                            .arg(m_d->expenseGroup()->expense(person))
                            .arg(m_d->expenseGroup()->debt(person)));
            else
                emit output(tr("No match for person \"%1\"\n").arg(args.at(0)));
        } else {
            emit output(m_d->expenseGroup()->personNames().join("\n") + "\n");
        }
    } else if (cmd == "addexpense" || cmd == "ax") {
        if (args.count() != 5) {
            emit output("addexpense expects exactly five argument.\n");
        } else {
            Person *p;
            QList<Person*> l = m_d->expenseGroup()->persons();
            QDate date = QDate::fromString(args.at(0));
            QString description = args.at(1);
            double value = args.at(2).toDouble();
            Person *paidBy = findPerson(args.at(3), l);
            if (!paidBy)
                emit output(tr("No match for person \"%1\"\n").arg(args.at(3)));
            QList<Person*> sharedBy;
            QStringList sharedNames = args.at(4).split(",");
            foreach (QString name, sharedNames) {
                if ((p = findPerson(name, l)))
                    sharedBy << p;
                else
                    emit output(tr("No match for person \"%1\"\n").arg(name));
            }

            if (paidBy && sharedBy.count() && sharedBy.count() == sharedNames.count()) {
                Expense *e = new Expense(paidBy, date, description, value, sharedBy);
                m_d->addExpense(e);
            } else {
                emit output(tr("Expense creation failed due to invalid parameters.\n"));
            }
        }
    } else if (cmd == "removeexpense" || cmd == "rx") {
        if (args.count() != 1) {
            emit output("removeperson expects exactly one argument.\n");
        } else {
            Expense *expense = findExpense(args.at(0),
                                           m_d->expenseGroup()->expenses());
            if (expense)
                m_d->removeExpense(expense);
            else
              emit output(tr("No match for expense \"%1\"\n").arg(args.at(0)));
        }
    } else if (cmd == "showexpense" || cmd == "sx") {
        QList<Expense*> l = m_d->expenseGroup()->expenses();
        if (args.count() > 1) {
            emit output("showexpense expects at most one argument.\n");
        } else if (args.count()) {
            Expense *e = findExpense(args.at(0), l);
            if (e)
                emit output(expenseToString(e));
            else
                emit output(tr("No match for expense \"%1\"\n").arg(args.at(0)));
        } else {
            foreach (Expense *e, l)
                emit output(expenseToString(e));
        }
    } else if (cmd == "undo" || cmd == "u") {
        if (args.count())
            emit output("undo expects no argument.\n");
        else if (m_d->canUndo())
            m_d->undo();
        else
            emit output(tr("Nothing to undo.\n"));
    } else if (cmd == "redo" || cmd == "r") {
        if (args.count())
            emit output("redo expects no argument.\n");
        else if (m_d->canRedo())
            m_d->redo();
        else
            emit output(tr("Nothing to redo.\n"));
    } else {
        emit output(tr("Unrecognized command : \"%1\"\n").arg(cmd));
    }
    emit commandDone();
}

#include "expensesharingcli.moc"
