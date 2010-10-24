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
