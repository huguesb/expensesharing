#include "person.h"

Person::Person() {
}

Person::Person(const QString& name)
    : m_name(name) {
}

QString Person::name() const {
    return m_name;
}

void Person::setName(const QString& name) {
    m_name = name;
}
