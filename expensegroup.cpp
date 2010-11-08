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

#include "expensegroup.h"

#include "person.h"
#include "expense.h"

#include <QHash>
#include <QFile>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

ExpenseGroup::ExpenseGroup(QObject *p)
    : QObject(p) {}

ExpenseGroup::ExpenseGroup(const QString& name, QObject *p)
    : QObject(p), m_name(name) {}

ExpenseGroup::~ExpenseGroup() {
    qDeleteAll(m_persons);
    qDeleteAll(m_expenses);
}

QString ExpenseGroup::name() const {
    return m_name;
}

void ExpenseGroup::setName(const QString& name) {
    m_name = name;
}

QList<Person*> ExpenseGroup::persons() const {
    return m_persons;
}

QStringList ExpenseGroup::personNames() const {
    QStringList l;
    foreach (Person *person, m_persons)
        l << person->name();
    return l;
}

void ExpenseGroup::addPerson(Person *person) {
    if (!m_persons.contains(person)) {
        emit personAboutToBeAdded(m_persons.count());
        m_persons.append(person);
        emit personAdded(person);
    }
    // TODO: smarter incremental update
    computeSummary(&m_summary);
    emit summaryChanged();
}

void ExpenseGroup::removePerson(Person *person) {
    int idx = m_persons.indexOf(person);
    if (idx != -1) {
        emit personAboutToBeRemoved(idx);
        m_persons.removeAt(idx);
        emit personRemoved(person);
        // TODO: smarter incremental update
        computeSummary(&m_summary);
        emit summaryChanged();
    }
}

void ExpenseGroup::clearPersons(bool del) {
    emit personsAboutToBeReset();
    if (del)
        qDeleteAll(m_persons);
    m_persons.clear();
    emit personsReset();
    // TODO: smarter incremental update
    computeSummary(&m_summary);
    emit summaryChanged();
}

QList<Expense*> ExpenseGroup::expenses() const {
    return m_expenses;
}

void ExpenseGroup::addExpense(Expense *expense) {
    if (!m_expenses.contains(expense)) {
        emit expenseAboutToBeAdded(m_expenses.count());
        m_expenses.append(expense);
        emit expenseAdded(expense);
        // TODO: smarter incremental update
        computeSummary(&m_summary);
        emit summaryChanged();
    }
}

void ExpenseGroup::removeExpense(Expense *expense) {
    int idx = m_expenses.indexOf(expense);
    if (idx != -1) {
        emit expenseAboutToBeRemoved(idx);
        m_expenses.removeAt(idx);
        emit expenseRemoved(expense);
        // TODO: smarter incremental update
        computeSummary(&m_summary);
        emit summaryChanged();
    }
}

void ExpenseGroup::clearExpenses(bool del) {
    emit expensesAboutToBeReset();
    if (del)
        qDeleteAll(m_expenses);
    m_expenses.clear();
    emit expensesReset();
    // TODO: smarter incremental update
    computeSummary(&m_summary);
    emit summaryChanged();
}

const ExpenseGroup::Summary& ExpenseGroup::summary() const {
    return m_summary;
}

bool ExpenseGroup::load(QIODevice *device) {
    QList<Person*> persons;
    QList<Expense*> expenses;
    QHash<int, Person*> personIds;
    QXmlStreamReader xml(device);
    if (xml.readNextStartElement() && xml.name() == "xsem" && xml.attributes().value("version") == "1.0") {
        m_name = xml.attributes().value("name").toString();
        while (xml.readNextStartElement()) {
            if (xml.name() == "person") {
                int id = -1;
                QString name;
                while (xml.readNextStartElement()) {
                    if (xml.name() == "id") {
                        if (id > 0) {
                            xml.raiseError(tr("<person> element can only have one <id> child element."));
                            continue;
                        }
                        bool ok = false;
                        id = xml.readElementText().toULong(&ok, 0);
                        if (!ok)
                            xml.raiseError(tr("Invalid <person> id."));
                        if (personIds.contains(id))
                            xml.raiseError(tr("Duplicate <person> id."));
                    } else if (xml.name() == "name") {
                        if (name.count()) {
                            xml.raiseError(tr("<person> element can only have one <name> child element."));
                            continue;
                        }
                        name = xml.readElementText();
                    } else {
                        qWarning("unsupported element encountered : %s", qPrintable(xml.name().toString()));
                        xml.skipCurrentElement();
                    }
                }
                if (id >= 0 && name.count() && !xml.error()) {
                    Person *person = new Person(name);
                    personIds[id] = person;
                    persons.append(person);
                } else {
                    qWarning("Invalid <person> element.");
                }
            } else if (xml.name() == "expense") {
                QDate date;
                QString description;
                double value = 0.0;
                Person *paidBy = 0;
                QList<Person*> sharedBy;
                while (xml.readNextStartElement()) {
                    if (xml.name() == "date") {
                        if (date.isValid()) {
                            xml.raiseError(tr("<expense> element can only have one <date> child element."));
                            continue;
                        }
                        date = QDate::fromString(xml.readElementText());
                        if (!date.isValid())
                            xml.raiseError(tr("Invalid <expense> date."));
                    } else if (xml.name() == "description") {
                        if (description.count()) {
                            xml.raiseError(tr("<expense> element can only have one <description> child element."));
                            continue;
                        }
                        description = xml.readElementText();
                    } else if (xml.name() == "value") {
                        if (value > 0.0) {
                            xml.raiseError(tr("<expense> element can only have one <description> child element."));
                            continue;
                        }
                        bool ok = false;
                        value = xml.readElementText().toDouble(&ok);
                        if (!ok || value <= 0.0)
                            xml.raiseError(tr("Invalid <expense> value."));
                    } else if (xml.name() == "paidBy") {
                        if (paidBy) {
                            xml.raiseError(tr("<expense> element can only have one <paidBy> child element."));
                            continue;
                        }
                        paidBy = personIds.value(xml.readElementText().toULong(), 0);
                        if (!paidBy) {
                            // TODO : support out of order loading?
                            xml.raiseError(tr("Invalid <paidBy> element."));
                        }
                    } else if (xml.name() == "sharedBy") {
                        if (sharedBy.count()) {
                            xml.raiseError(tr("<expense> element can only have one <sharedBy> child element."));
                            continue;
                        }
                        while (xml.readNextStartElement()) {
                            if (xml.name() == "personRef") {
                                bool ok = false;
                                Person *person = personIds.value(xml.readElementText().toULong(&ok, 0), 0);
                                if (ok && person && !sharedBy.contains(person))
                                    sharedBy.append(person);
                                else
                                    qWarning("Invalid or duplicate <personRef>.");
                            } else {
                                qWarning("unsupported element encountered : %s", qPrintable(xml.name().toString()));
                                xml.skipCurrentElement();
                            }
                        }
                    } else {
                        qWarning("unsupported element encountered : %s", qPrintable(xml.name().toString()));
                        xml.skipCurrentElement();
                    }
                }
                if (paidBy && value > 0.0 && !xml.error()) {
                    Expense *expense = new Expense(paidBy, date, description, value, sharedBy);
                    expenses.append(expense);
                } else {
                    qWarning("Invalid <expense> element.");
                }
            } else {
                qWarning("unsupported element encountered : %s", qPrintable(xml.name().toString()));
                xml.skipCurrentElement();
            }
        }
    } else {
       xml.raiseError(tr("The file is not an XSEM version 1.0 file."));
    }

    if (xml.error()) {
        qWarning("parsing failed : %s", qPrintable(xml.errorString()));
        return false;
    }

    emit personsAboutToBeReset();
    qDeleteAll(m_persons);
    m_persons = persons;
    emit personsReset();

    emit expensesAboutToBeReset();
    qDeleteAll(m_expenses);
    m_expenses = expenses;
    emit expensesReset();

    computeSummary(&m_summary);
    emit summaryChanged();

    return true;
}

bool ExpenseGroup::save(QIODevice *device) {
    QXmlStreamWriter xml(device);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE xsem>");
    xml.writeStartElement("xsem");
    xml.writeAttribute("version", "1.0");
    xml.writeAttribute("name", m_name);

    foreach (Person *person, m_persons) {
        xml.writeStartElement("person");
        xml.writeTextElement("id", QString::number(m_persons.indexOf(person)));
        xml.writeTextElement("name", person->name());
        xml.writeEndElement();
    }

    foreach (Expense *expense, m_expenses) {
        xml.writeStartElement("expense");
        xml.writeTextElement("date", expense->date().toString());
        xml.writeTextElement("description", expense->description());
        xml.writeTextElement("value", QString::number(expense->value()));
        xml.writeTextElement("paidBy", QString::number(m_persons.indexOf(expense->payer())));
        xml.writeStartElement("sharedBy");
        QList<Person*> sharedBy = expense->personsSharing();
        foreach (Person *person, sharedBy)
            xml.writeTextElement("personRef", QString::number(m_persons.indexOf(person)));
        xml.writeEndElement();
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    return true;
}

bool ExpenseGroup::load(const QString &filename) {
    QFile input(filename);
    if (!input.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("Unable to open %s for reading.", qPrintable(filename));
        return false;
    }
    return load(&input);
}

bool ExpenseGroup::save(const QString &filename) {
    QFile output(filename);
    if (!output.open(QFile::WriteOnly | QFile::Text)) {
        qWarning("Unable to open %s for writing.", qPrintable(filename));
        return false;
    }
    return save(&output);
}

double ExpenseGroup::expense(Person *person) const {
    double e = 0.0;
    foreach (Expense *expense, m_expenses)
        if (expense->payer() == person)
            e += expense->value();
    return e;
}

double ExpenseGroup::debt(Person *person) const {
    double d = 0.0;
    foreach (Expense *expense, m_expenses) {
        if (expense->payer() == person)
            d -= expense->value();
        if (expense->personsSharing().contains(person))
            d += expense->value() / expense->personsSharingCount();
    }
    return d;
}

double ExpenseGroup::debt(Person *from, Person *to) const {
    double d = 0.0;
    foreach (Expense *expense, m_expenses) {
        if (expense->payer() == from && expense->personsSharing().contains(to))
            d -= expense->value() / expense->personsSharingCount();
        if (expense->payer() == to && expense->personsSharing().contains(from))
            d += expense->value() / expense->personsSharingCount();
    }
    return d;
}

void ExpenseGroup::computeSummary(ExpenseGroup::Summary *summary) const {
    QList<double> l;
    for (int i = 0; i < m_persons.count(); ++i)
        l.append(0.0);
    summary->persons = m_persons;
    summary->expenses = l;
    summary->groupDebts = l;
    summary->personDebts.clear();
    for (int i = 0; i < m_persons.count(); ++i)
        summary->personDebts.append(l);

    foreach (Expense *expense, m_expenses) {
        int idx = m_persons.indexOf(expense->payer());
        if (idx == -1) {
            qWarning("unpaid expense...");
            continue;
        }
        summary->expenses[idx] += expense->value();
        summary->groupDebts[idx] -= expense->value();
        const double share = expense->value() / expense->personsSharingCount();
        for (int i = 0; i < m_persons.count(); ++i) {
            if (!expense->personsSharing().contains(m_persons.at(i)))
                continue;
            summary->groupDebts[i] += share;
            summary->personDebts[i][idx] += share;
            summary->personDebts[idx][i] -= share;
        }
    }
}
