#include "newpersondialog.h"
#include "ui_newpersondialog.h"

#include "person.h"

NewPersonDialog::NewPersonDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewPersonDialog) {
    ui->setupUi(this);
}

NewPersonDialog::~NewPersonDialog() {
    delete ui;
}

Person* NewPersonDialog::getPerson(QWidget *p) {
    NewPersonDialog dlg(p);
    return dlg.exec() == QDialog::Accepted ? new Person(dlg.ui->name->text()) : 0;
}
