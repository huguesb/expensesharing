#include <QtGui/QApplication>
#include "expensesharing.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ExpenseSharing w;
    w.show();

    return a.exec();
}
