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
#include <QFileDialog>
#include <QMessageBox>

ExpenseSharing::ExpenseSharing(QWidget *parent)
    : QMainWindow(parent), m_group(new ExpenseGroup(this)) {
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

    summaryChanged();
}

ExpenseSharing::~ExpenseSharing() {
}

void ExpenseSharing::on_actionOpen_triggered() {
    QString fn =
    QFileDialog::getOpenFileName(this,
                                 tr("Open expense log..."),
                                 QString(),
                                 "XSEM files (*.xsem);;All files (*)");
    if (fn.count()) {
        m_group->load(fn);
        tvExpenseDetails->resizeColumnsToContents();
        m_fileName = fn;
    }
}

void ExpenseSharing::on_actionSave_triggered() {
    if (m_fileName.count())
        m_group->save(m_fileName);
    else
        on_actionSaveAs_triggered();
}

void ExpenseSharing::on_actionSaveAs_triggered() {
    QString fn =
    QFileDialog::getSaveFileName(this,
                                 tr("Save expense log..."),
                                 QString(),
                                 "XSEM files (*.xsem);;All files (*)");
    if (fn.count()) {
        m_group->save(fn);
        m_fileName = fn;
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
