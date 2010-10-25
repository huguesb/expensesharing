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
#include <QSettings>
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

    updateRecentMenu();
    summaryChanged();
}

ExpenseSharing::~ExpenseSharing() {
}

void ExpenseSharing::setCurrentFileName(const QString& filename) {
    m_fileName = filename;

    QSettings s;
    QStringList l = s.value("recentFiles", QStringList()).toStringList();
    int maxEntries = s.value("maxRecentFileEntries", 10).toUInt();
    l.removeAll(filename);
    l.prepend(filename);
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
    QStringList files = QSettings().value("recentFiles").toStringList();
    for (int n = 0; n < files.count(); ++n) {
        QString fn = files.at(n);
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
        open(a->data().toString());
}

bool ExpenseSharing::open(const QString& filename) {
    bool ok = false;
    if (QFile::exists(filename)) {
        ok = m_group->load(filename);
        tvExpenseDetails->resizeColumnsToContents();
        setCurrentFileName(filename);
    }
    return ok;
}

void ExpenseSharing::on_actionOpen_triggered() {
    QString fn =
    QFileDialog::getOpenFileName(this,
                                 tr("Open expense log..."),
                                 QString(),
                                 "XSEM files (*.xsem);;All files (*)");
    open(fn);
}

void ExpenseSharing::on_actionClearRecent_triggered() {
    QSettings().remove("recentFiles");
    updateRecentMenu();
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
                                 m_fileName,
                                 "XSEM files (*.xsem);;All files (*)");
    if (fn.count()) {
        m_group->save(fn);
        setCurrentFileName(fn);
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
