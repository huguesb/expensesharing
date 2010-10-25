#-------------------------------------------------
#
# Project created by QtCreator 2010-10-24T12:39:49
#
#-------------------------------------------------

QT       += core gui network

TARGET = expensesharing
TEMPLATE = app


SOURCES += main.cpp\
        expensesharing.cpp \
    expense.cpp \
    person.cpp \
    expensegroup.cpp \
    expensemodel.cpp \
    personmodel.cpp \
    newpersondialog.cpp \
    newexpensedialog.cpp

HEADERS  += expensesharing.h \
    expense.h \
    person.h \
    expensegroup.h \
    expensemodel.h \
    personmodel.h \
    newpersondialog.h \
    newexpensedialog.h

FORMS    += expensesharing.ui \
    newpersondialog.ui \
    newexpensedialog.ui

OTHER_FILES += \
    README.txt \
    GPL.txt
