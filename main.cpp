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

#include <QApplication>

#include "expensesharing.h"
#include "expensesharingui.h"
#include "expensesharingcli.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("ExpenseSharing");
    QCoreApplication::setOrganizationName("Coloclosoft");
    QCoreApplication::setOrganizationDomain("http://bruant.it");

    if (argc <= 1 || QString::fromLocal8Bit(argv[1]) == "--ui") {
        QApplication app(argc, argv);
        ExpenseSharingUI ui;
        ui.show();
        return app.exec();
    } else if (QString::fromLocal8Bit(argv[1]) != "--help") {
        QCoreApplication app(argc, argv);
        bool interactive = QString::fromLocal8Bit(argv[1]) == "--cli";
        ExpenseSharingCLI cli(interactive);
        if (interactive) {
            return app.exec();
        } else {
            QStringList l = app.arguments();
            l.takeFirst();
            if (l.count() >= 2) {
                cli.command("open", QStringList() << l.takeFirst());
                cli.command(l.takeFirst(), l);
                if (cli.expenseSharing()->isModified())
                    cli.command("save", QStringList());
                return 0;
            }
        }
    }

    qDebug(
        "ExpenseSharing, Copyright (c) 2010 Hugues Bruant. All rights reserved.\n"
        "Easy management of shared expense among a group of people.\n\n"
        "expensesharing --gui\n"
        "expensesharing --cli\n"
        "expensesharing <file.xsem> <cmd> [args]\n");

    return 0;
}
