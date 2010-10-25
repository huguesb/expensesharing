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

#include "expense.h"

Expense::Expense()
    : m_value(0.0), m_payer(0) {
}

Expense::Expense(Person *payer, const QString& description, double value)
    : m_description(description), m_value(value), m_payer(payer) {

}

Expense::Expense(Person *payer, const QDate& date, const QString& description, double value)
    : m_date(date), m_description(description), m_value(value), m_payer(payer) {

}

Expense::Expense(Person *payer, const QDate& date, const QString& description, double value, const QList<Person*>& persons)
    : m_date(date), m_description(description), m_value(value), m_payer(payer), m_personsSharing(persons) {

}

QDate Expense::date() const {
    return m_date;
}

void Expense::setDate(const QDate& date) {
    m_date = date;
}

QString Expense::description() const {
    return m_description;
}

void Expense::setDescription(const QString& description) {
    m_description = description;
}

double Expense::value() const {
    return m_value;
}

void Expense::setValue(double value) {
    m_value = value;
}

Person* Expense::payer() const {
    return m_payer;
}

void Expense::setPayer(Person *payer) {
    m_payer = payer;
}

int Expense::personsSharingCount() const {
    return m_personsSharing.count();
}

QList<Person*> Expense::personsSharing() const {
    return m_personsSharing;
}

void Expense::setPersonSharing(const QList<Person*>& persons) {
    m_personsSharing = persons;
}

void Expense::addPersonSharing(Person *person) {
    if (!m_personsSharing.contains(person))
        m_personsSharing.append(person);
}

void Expense::removePerson(Person *person) {
    m_personsSharing.removeAll(person);
}
