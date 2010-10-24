#ifndef PERSONMODEL_H
#define PERSONMODEL_H

#include <QAbstractItemModel>

class Person;
class ExpenseGroup;

class PersonModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit PersonModel(ExpenseGroup *group);

    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const;

signals:

public slots:

private slots:
    void personAboutToBeAdded(int idx);
    void personAdded(Person *person);
    void personAboutToBeRemoved(int idx);
    void personRemoved(Person *person);
    void personsAboutToBeReset();
    void personsReset();

private:
    ExpenseGroup *m_group;
};

#endif // PERSONMODEL_H
