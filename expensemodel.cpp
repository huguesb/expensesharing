#include "expensemodel.h"

#include "person.h"
#include "expense.h"
#include "expensegroup.h"

#include <QStringList>

ExpenseModel::ExpenseModel(ExpenseGroup *group) :
    QAbstractItemModel(group), m_group(group) {
    connect(group, SIGNAL( expenseAboutToBeAdded(int) ),
            this , SLOT  ( expenseAboutToBeAdded(int) ));
    connect(group, SIGNAL( expenseAdded(Expense*) ),
            this , SLOT  ( expenseAdded(Expense*) ));
    connect(group, SIGNAL( expenseAboutToBeRemoved(int) ),
            this , SLOT  ( expenseAboutToBeRemoved(int) ));
    connect(group, SIGNAL( expenseRemoved(Expense*) ),
            this , SLOT  ( expenseRemoved(Expense*) ));
    connect(group, SIGNAL( expensesAboutToBeReset() ),
            this , SLOT  ( expensesAboutToBeReset() ));
    connect(group, SIGNAL( expensesReset() ),
            this , SLOT  ( expensesReset() ));
}

QModelIndex ExpenseModel::index(int row, int column,
                                const QModelIndex &parent) const {
    QList<Expense*> l = m_group->expenses();
    return parent.isValid() || row < 0 || row >= l.count() || column < 0 || column >= 5
            ? QModelIndex()
            : createIndex(row, column, l.at(row));
}

QModelIndex ExpenseModel::parent(const QModelIndex &child) const {
    return QModelIndex();
}

int ExpenseModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_group->persons().count();
}

int ExpenseModel::columnCount(const QModelIndex &parent) const {
    return 5;
}

bool ExpenseModel::hasChildren(const QModelIndex &parent) const {
    return !parent.isValid();
}

static QStringList getNames(const QList<Person*>& persons) {
    QStringList l;
    foreach (Person *p, persons)
        l << p->name();
    return l;
}

QVariant ExpenseModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole && index.isValid()) {
        Expense *expense = m_group->expenses().at(index.row());
        switch (index.column()) {
        case 0:
            return expense->date();
        case 1:
            return expense->description();
        case 2:
            return expense->value();
        case 3:
            return expense->payer()->name();
        case 4:
            return getNames(expense->personsSharing()).join(",");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QVariant ExpenseModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
    static QStringList headers = QStringList()
        << "Date"
        << "Description"
        << "Value"
        << "Paid by"
        << "Shared by";
    return orientation == Qt::Horizontal && role == Qt::DisplayRole
            ? headers.at(section) : QVariant();
}

void ExpenseModel::expenseAboutToBeAdded(int idx) {
    beginInsertRows(QModelIndex(), idx, idx);
}

void ExpenseModel::expenseAdded(Expense *expense) {
    endInsertRows();
}

void ExpenseModel::expenseAboutToBeRemoved(int idx) {
    beginRemoveRows(QModelIndex(), idx, idx);
}

void ExpenseModel::expenseRemoved(Expense *expense) {
    endRemoveRows();
}

void ExpenseModel::expensesAboutToBeReset() {
    beginResetModel();
}

void ExpenseModel::expensesReset() {
    endResetModel();
}
