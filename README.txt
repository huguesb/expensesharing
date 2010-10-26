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

To build ExpenseSharing you need a C++ compiler and Qt4 development packages.


How to use it
-------------

Depending on your taste you can use the command line or GUI interface.

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

ExpenseSharing stores its data in XSEM files (eXtensible Shared Expense Manager)
which are basically XML files with a .xsem extension storing a list of persons
and expenses.

TODO
----
 * CLI
 * XSEM DTD
 * display of p2p debts
 * expense log diff
 * some form of networked / shared edit
 * use more precise math?
 * interface polish
