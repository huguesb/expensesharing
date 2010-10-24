#ifndef EXPENSEGROUP_H
#define EXPENSEGROUP_H

#include <QList>
#include <QString>
#include <QObject>

class Person;
class Expense;

class ExpenseGroup : public QObject
{
Q_OBJECT

public:
    struct Summary {
        QList<Person*> persons;
        QList<double> expenses;
        QList<double> groupDebts;
        QList< QList<double> > personDebts;
    };

    ExpenseGroup(QObject *p = 0);
    explicit ExpenseGroup(const QString& name, QObject *p = 0);
    ~ExpenseGroup();

    QString name() const;
    void setName(const QString& name);

    QList<Person*> persons() const;
    void addPerson(Person *person);
    void removePerson(Person *person);
    void clearPersons();

    QList<Expense*> expenses() const;
    void addExpense(Expense *expense);
    void removeExpense(Expense *expense);
    void clearExpenses();

    const Summary& summary() const;

    double expense(Person *person) const;

    double debt(Person *person) const;
    double debt(Person *from, Person *to) const;

    void computeSummary(Summary *summary) const;

public slots:
    bool load(const QString& filename);
    bool save(const QString& filename);

signals:
    void expenseAboutToBeAdded(int idx);
    void expenseAdded(Expense *expense);
    void expenseAboutToBeRemoved(int idx);
    void expenseRemoved(Expense *expense);
    void expensesAboutToBeReset();
    void expensesReset();

    void personAboutToBeAdded(int idx);
    void personAdded(Person *person);
    void personAboutToBeRemoved(int idx);
    void personRemoved(Person *person);
    void personsAboutToBeReset();
    void personsReset();

    void summaryChanged();

private:
    QString m_name;
    QList<Person*> m_persons;
    QList<Expense*> m_expenses;
    Summary m_summary;
};

#endif // EXPENSEGROUP_H
