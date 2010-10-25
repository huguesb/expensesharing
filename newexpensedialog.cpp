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

#include "newexpensedialog.h"
#include "ui_newexpensedialog.h"

#include "person.h"
#include "expense.h"

NewExpenseDialog::NewExpenseDialog(QList<Person*> persons, QWidget *parent)
    : QDialog(parent)
    , m_persons(persons)
    , ui(new Ui::NewExpenseDialog) {
    ui->setupUi(this);
    ui->date->setDate(QDate::currentDate());
    foreach (Person *p, persons) {
        ui->paidBy->addItem(p->name());
        QListWidgetItem *item = new QListWidgetItem(p->name());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        ui->sharedBy->addItem(item);
    }
}

NewExpenseDialog::~NewExpenseDialog() {
    delete ui;
}

QDate NewExpenseDialog::date() const {
    return ui->date->date();
}

QString NewExpenseDialog::description() const {
    return ui->description->toPlainText();
}

double NewExpenseDialog::value() const {
    return ui->value->value();
}

Person* NewExpenseDialog::paidBy() const {
    return m_persons.at(ui->paidBy->currentIndex());
}

QList<Person*> NewExpenseDialog::sharedBy() const {
    QList<Person*> l;
    for (int i = 0; i < ui->sharedBy->count(); ++i)
        if (ui->sharedBy->item(i)->checkState() == Qt::Checked)
            l.append(m_persons.at(i));
    return l;
}

Expense* NewExpenseDialog::getExpense(QList<Person*> persons, QWidget *p) {
    NewExpenseDialog dlg(persons, p);
    return dlg.exec() == QDialog::Accepted
            ? new Expense(dlg.paidBy(),
                          dlg.date(),
                          dlg.description(),
                          dlg.value(),
                          dlg.sharedBy())
                : 0;
}
