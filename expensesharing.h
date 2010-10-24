#ifndef EXPENSESHARING_H
#define EXPENSESHARING_H

#include <QMainWindow>
#include "ui_expensesharing.h"

class ExpenseGroup;

class ExpenseSharing : public QMainWindow, private Ui::ExpenseSharing
{
    Q_OBJECT

public:
    explicit ExpenseSharing(QWidget *parent = 0);
    ~ExpenseSharing();

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionAdd_triggered();
    void on_actionHelp_triggered();
    void on_actionAbout_triggered();

    void summaryChanged();
    void currentPersonChanged(const QModelIndex& idx);
    void on_lvPersons_customContextMenuRequested(const QPoint& pos);
    void on_tvExpenseDetails_customContextMenuRequested(const QPoint& pos);

private:
    QString m_fileName;
    ExpenseGroup *m_group;
};

#endif // EXPENSESHARING_H