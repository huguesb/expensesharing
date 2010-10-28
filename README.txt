ExpenseSharing
--------------
Copyright (c) 2010 Hugues Bruant <hugues.bruant@gmail.com>
All rights reserved.
Licensed under GNU General Pulic License version 3.


What is it ?
------------

A small program to manage shared expenses among flatmates (for instance).


How to build it
---------------

To build ExpenseSharing you need a C++ compiler and Qt development packages in
version 4.6 or higher.


How to use it
-------------

Depending on your taste you can use the command line or GUI interface. The
command line interface comes in two different flavors : interactive or not.

To start in GUI mode :
$ expensesharing --ui
or, as GUI mode is the default
$ expensesharing

To start in interactive CLI mode :
$ expensesharing --cli

To use as a "usual" command line tool :
$ expensesharing <file> <command> [args]

The two CLI modes share the exact same command syntax and semantics.


Getting started
---------------

There are a variety of possible workflows. The most common are :
 * local storage, one person act as ccountant for the group
 * remote storage, everyone adds its own expenses to file shared over the network

The following only covers the first workflow type using the GUI.

The first step is to add all the persons sharing expense. To do that, right
click in the Person view and choose the "New person..." entry of the context
menu.

Once the "team" setup is done you can start entering your expenses by clicking
on the "Add" button on the toolbar or choosing the "New expense..." entry of
the context menu in the expense details tab.

One important feature of this tool is that it allows fine grained sharing of
the expense among a group. Not all expenses are always shared by all members
of the group and some expenses can be made by someone and fully reimbursed
by others (the original payer has no share in the expense)...


More details
------------

ExpenseSharing stores its data in XSEM files (eXtensible Shared Expense Manager)
which are basically XML files with a .xsem extension storing a list of persons
and expenses.

The XSEM file format is formalized in schemas (DTD, XSD and compact RelaxNG are
provided). However these schemas do not allow complete validation. For instance,
due to limitations of the schema languages, they cannot ensure that expenses
refer to valid persons.


TODO
----
 * CLI
 * display of p2p debts
 * expense log diff
 * some form of networked / shared edit
 * interface polish
