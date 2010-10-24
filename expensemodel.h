#ifndef EXPENSEMODEL_H
#define EXPENSEMODEL_H

#include <QAbstractItemModel>

class Expense;
class ExpenseGroup;

class ExpenseModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ExpenseModel(ExpenseGroup *group);

    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const;
signals:

public slots:

private slots:
    void expenseAboutToBeAdded(int idx);
    void expenseAdded(Expense *expense);
    void expenseAboutToBeRemoved(int idx);
    void expenseRemoved(Expense *expense);
    void expensesAboutToBeReset();
    void expensesReset();

private:
    ExpenseGroup *m_group;
};

#endif // EXPENSEMODEL_H
