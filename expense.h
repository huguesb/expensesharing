#ifndef EXPENSE_H
#define EXPENSE_H

#include <QList>
#include <QDate>
#include <QString>

class Person;

class Expense
{
public:
    Expense();
    Expense(Person *payer, const QString& description, double value);
    Expense(Person *payer, const QDate& date, const QString& description, double value);
    Expense(Person *payer, const QDate& date, const QString& description, double value, const QList<Person*>& persons);

    QDate date() const;
    void setDate(const QDate& date);

    QString description() const;
    void setDescription(const QString& description);

    double value() const;
    void setValue(double value);

    Person* payer() const;
    void setPayer(Person *person);

    int personsSharingCount() const;
    QList<Person*> personsSharing() const;
    void setPersonSharing(const QList<Person*>& persons);
    void addPersonSharing(Person *person);
    void removePerson(Person *person);

private:
    QDate m_date;
    QString m_description;
    double m_value;
    Person *m_payer;
    QList<Person*> m_personsSharing;
};

#endif // EXPENSE_H
