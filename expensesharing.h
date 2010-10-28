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

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QNetworkReply>

class Person;
class Expense;
class ExpenseGroup;

class QAuthenticator;
class QNetworkAccessManager;

class NetworkWaiter : public QObject {
    Q_OBJECT
public:
    NetworkWaiter(QNetworkReply *reply, QObject *p = 0);

public slots:
    virtual bool wait();

protected slots:
    virtual void finished();
    virtual void error(QNetworkReply::NetworkError error);
    virtual void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth);

    virtual void uploadProgress(qint64 sent, qint64 total);
    virtual void downloadProgress(qint64 received, qint64 total);

protected:
    QNetworkReply *m_reply;
};

class ExpenseSharing : public QObject
{
    Q_OBJECT
public:
    explicit ExpenseSharing(QObject *parent = 0);

    QUrl url() const;
    const ExpenseGroup* expenseGroup() const;

    QString errorString() const;

    bool isModified() const;

    bool canUndo();
    bool canRedo();

    QAction* createUndoAction(QObject *parent, const QString& prefix = QString()) const;
    QAction* createRedoAction(QObject *parent, const QString& prefix = QString()) const;

signals:
    void summaryChanged();
    void urlChanged(const QUrl& url);
    void modificationChanged(bool modified);

public slots:
    virtual bool open(const QUrl& url);
    virtual bool open(const QString& filename);

    virtual bool save();
    virtual bool saveAs(const QUrl& url);
    virtual bool saveAs(const QString& filename);

    void addPerson(Person *person);
    void removePerson(Person *person);

    void addExpense(Expense *expense);
    void removeExpense(Expense *expense);

    void undo();
    void redo();

protected:
    virtual void setUrl(const QUrl& url);
    virtual NetworkWaiter* createWaiter(QNetworkReply *reply) = 0;

private slots:
    void cleanChanged(bool clean);

private:
    QUrl m_url;
    QString m_error;
    ExpenseGroup *m_group;
    QUndoStack m_commands;
    QNetworkAccessManager *m_manager;
};

#endif // EXPENSESHARING_H
