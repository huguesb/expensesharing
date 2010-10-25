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
