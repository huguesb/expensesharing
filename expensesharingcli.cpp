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
    CLI(QObject *p = 0)
        : QThread(p) {
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

ExpenseSharingCLI::ExpenseSharingCLI(QObject *p)
    : QObject(p)
    , m_cli(new CLI(this))
    , m_d(new ExpenseSharingCLIPrivate(this)) {

    connect(m_cli,
            SIGNAL( command(QString, QStringList) ),
            SLOT  ( command(QString, QStringList) ));
    connect(this , SIGNAL( commandDone() ),
            m_cli, SLOT  ( promptReady() ));
    connect(this , SIGNAL( output(QString) ),
            m_cli, SLOT  ( output(QString) ));
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
                "  quit\n"
                "  \n"
                "Type help <command> for detailed help.\n");
        } else {
            const QString& s = args.at(0);
            if (s == "open" || s == "o")
                ;
            else if (s == "save" || s == "s")
                ;
            else if (s == "addperson" || s == "ap")
                ;
            else if (s == "removeperson" || s == "rp")
                ;
            else if (s == "showperson" || s == "sp" )
                ;
            else if (s == "addexpense" || s == "ax")
                ;
            else if (s == "removeexpense" || s == "rx")
                ;
            else if (s == "showexpense" || s == "sx")
                ;
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
                emit output(tr("%1 : \n").arg(person->name()));
            else
                emit output(tr("No match for person \"%1\"\n").arg(args.at(0)));
        } else {
            emit output(m_d->expenseGroup()->personNames().join("\n") + "\n");
        }
    } else if (cmd == "addexpense" || cmd == "ax") {

    } else if (cmd == "removeexpense" || cmd == "rx") {

    } else if (cmd == "showexpense" || cmd == "sx") {

    } else {
        emit output(tr("Unrecognized command : \"%1\"\n").arg(cmd));
    }
    emit commandDone();
}

#include "expensesharingcli.moc"
