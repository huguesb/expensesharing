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

#ifndef NEWEXPENSEDIALOG_H
#define NEWEXPENSEDIALOG_H

#include <QDialog>

namespace Ui {
    class NewExpenseDialog;
}

class Person;
class Expense;

class QDate;

class NewExpenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewExpenseDialog(QList<Person*> persons, QWidget *parent = 0);
    ~NewExpenseDialog();

    QDate date() const;
    QString description() const;
    double value() const;
    Person* paidBy() const;
    QList<Person*> sharedBy() const;

    static Expense* getExpense(QList<Person*> persons, QWidget *p = 0);

private:
    QList<Person*> m_persons;
    Ui::NewExpenseDialog *ui;
};

#endif // NEWEXPENSEDIALOG_H
