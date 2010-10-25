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
