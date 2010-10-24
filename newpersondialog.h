#ifndef NEWPERSONDIALOG_H
#define NEWPERSONDIALOG_H

#include <QDialog>

namespace Ui {
    class NewPersonDialog;
}

class Person;

class NewPersonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPersonDialog(QWidget *parent = 0);
    ~NewPersonDialog();

    static Person* getPerson(QWidget *p = 0);

private:
    Ui::NewPersonDialog *ui;
};

#endif // NEWPERSONDIALOG_H
