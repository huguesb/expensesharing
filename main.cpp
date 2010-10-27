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

#include <QtGui/QApplication>

#include "expensesharingui.h"
#include "expensesharingcli.h"

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("ExpenseSharing");
    QApplication::setOrganizationName("Coloclosoft");
    QApplication::setOrganizationDomain("http://bruant.it");

    QApplication app(argc, argv);

    if (argc <= 1 || !QString::fromLocal8Bit(argv[1]).startsWith("--")) {
        ExpenseSharingUI ui;
        ui.show();
        return app.exec();
    } else {
        ExpenseSharingCLI cli;
        return app.exec();
    }
}
