#ifndef PERSON_H
#define PERSON_H

#include <QString>

class Person
{
public:
    Person();
    explicit Person(const QString& name);

    QString name() const;
    void setName(const QString& name);

private:
    QString m_name;
};

#endif // PERSON_H
