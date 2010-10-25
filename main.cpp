#include <QtGui/QApplication>
#include "expensesharing.h"

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("ExpenseSharing");
    QApplication::setOrganizationName("Coloclosoft");
    QApplication::setOrganizationDomain("http://bruant.it");

    QApplication app(argc, argv);
    ExpenseSharing w;
    w.show();

    return app.exec();
}
