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

#include "personmodel.h"

#include "person.h"
#include "expense.h"
#include "expensegroup.h"

PersonModel::PersonModel(const ExpenseGroup *group, QObject *p)
    : QAbstractItemModel(p)
    , m_group(group) {
    connect(group, SIGNAL( personAboutToBeAdded(int) ),
            this , SLOT  ( personAboutToBeAdded(int) ));
    connect(group, SIGNAL( personAdded(Person*) ),
            this , SLOT  ( personAdded(Person*) ));
    connect(group, SIGNAL( personAboutToBeRemoved(int) ),
            this , SLOT  ( personAboutToBeRemoved(int) ));
    connect(group, SIGNAL( personRemoved(Person*) ),
            this , SLOT  ( personRemoved(Person*) ));
    connect(group, SIGNAL( personsAboutToBeReset() ),
            this , SLOT  ( personsAboutToBeReset() ));
    connect(group, SIGNAL( personsReset() ),
            this , SLOT  ( personsReset() ));
}

QModelIndex PersonModel::index(int row, int column,
                               const QModelIndex &parent) const {
    QList<Person*> l = m_group->persons();
    return parent.isValid() || row < 0 || row >= l.count()
            ? QModelIndex()
            : createIndex(row, column, l.at(row));
}

QModelIndex PersonModel::parent(const QModelIndex &child) const {
    return QModelIndex();
}

int PersonModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_group->persons().count();
}

int PersonModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

bool PersonModel::hasChildren(const QModelIndex &parent) const {
    return !parent.isValid();
}

QVariant PersonModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole && index.isValid())
        return m_group->persons().at(index.row())->name();

    return QVariant();
}

QVariant PersonModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
    return orientation == Qt::Horizontal && role == Qt::DisplayRole && !section
            ? "Name" : QVariant();
}

void PersonModel::personAboutToBeAdded(int idx) {
    beginInsertRows(QModelIndex(), idx, idx);
}

void PersonModel::personAdded(Person *person) {
    endInsertRows();
}

void PersonModel::personAboutToBeRemoved(int idx) {
    beginRemoveRows(QModelIndex(), idx, idx);
}

void PersonModel::personRemoved(Person *person) {
    endRemoveRows();
}

void PersonModel::personsAboutToBeReset() {
    beginResetModel();
}

void PersonModel::personsReset() {
    endResetModel();
}
