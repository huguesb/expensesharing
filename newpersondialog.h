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
