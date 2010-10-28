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

#ifndef EXPENSESHARINGCLI_H
#define EXPENSESHARINGCLI_H

#include <QObject>

class CLI;
class ExpenseSharing;

class ExpenseSharingCLI : public QObject
{
    Q_OBJECT
public:
    ExpenseSharingCLI(bool interactive, QObject *p = 0);

    ExpenseSharing* expenseSharing() const;

public slots:
    void command(const QString& command, const QStringList& args);

signals:
    void commandDone();
    void output(const QString& out);

private:
    CLI *m_cli;
    ExpenseSharing *m_d;
};

#endif // EXPENSESHARINGCLI_H
