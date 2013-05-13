// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C C O N S O L E
// *************************************************************************
// Work Name   : HercConsole
// Description : This provides a user web console for Hercules Hosting
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : print.cpp
// Description : print section
// *************************************************************************
// L I C E N S E
// *************************************************************************
// This program is free software: you can redistribute it and/or modify
// it under the terms of version 3 of the GNU General Public License as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// For the avoidance of doubt:
// - Version 3 of the license (i.e. not earlier nor later versions) apply.
// - a copy of the license text should be in the "license" directory of the
//   source distribution.
// - Requests for use under other licenses will be treated sympathetically,
//   please see contact details.
// *************************************************************************
// C O N T A C T   D E T A I L S
// *************************************************************************
// E-mail      : adrian@sutherlandonline.org
//             : adrian@open-bpm.org
//             : sutherland@users.sourceforge.net
// Web         : www.open-bpm.org
// Telephone   : Please e-mail for details
// Postal      : UK - Please e-mail for details
// *************************************************************************

#include <Wt/WGridLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WStandardItemModel>

#include <iostream>

#include <herchosting.h> // Common Functionality
#include <console.h>

using namespace Wt;


MyPrint::MyPrint(char* user, bool admin)
{
 // Save the user
 adminMode = admin;
 strcpy(member,user);

 // Set-up Data Models
 accountModel = 0;
 listModel = 0;

 // The layout is a 3x2 grid layout
 WGridLayout *layout = new WGridLayout();
 layout->addWidget(HercConsole::createTitle("Accounts"), 0, 0);
 layout->addWidget(HercConsole::createTitle("Print Files"), 0, 1);

 layout->addWidget(createAccountView(), 1, 0);
                                                             
 WVBoxLayout *vbox = new WVBoxLayout();
 vbox->addWidget(createListView(), 1);
 vbox->addWidget(new WText("Output listing goes here"), 0);
                                                          
 layout->addLayout(vbox, 1, 1);            
                                        
 layout->addWidget(helpDisplay(), 2, 0, 1, 2, AlignTop | AlignLeft);
                                                       
 // Let row 1 and column 1 take the excess space.
 layout->setRowStretch(1, 1);
 layout->setColumnStretch(1, 1);
 setLayout(layout);

 // Load Accounts
 loadAccounts();

 resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));
}

MyPrint::~MyPrint()
{
  // std::cout << "Destructor" << std::endl;
}

// Create help area
WWidget *MyPrint::helpDisplay() 
{
   WText *result = new WText("Help Information goes here");
   result->setStyleClass("obpm-help");
   return result;
}
                   
WStandardItem *MyPrint::createHostItem(const WString& host)
{
  WStandardItem *result = new WStandardItem(host);
  result->setIcon("images/server.png");
  result->setFlags(result->flags().clear(ItemIsSelectable));
  return result;
}
                                                                                    
WStandardItem *MyPrint::createAccountItem(const WString& account)
{
  WStandardItem *result = new WStandardItem(account);
  result->setIcon("images/user.png");
  return result;
}

WWidget *MyPrint::createAccountView() 
{
//  WContainerWidget *container = new WContainerWidget();
//  accountView = new WTreeView(container);    
  accountView = new WTreeView();    
  accountView->selectionChanged().connect(SLOT(this, MyPrint::loadList));
//  accountView->mouseWentDown().connect(SLOT(this, TreeViewDragDrop::showPopup));

  accountView->resize(150,  WLength(100, WLength::Percentage));
//  container->resize(150,  WLength(100, WLength::Percentage));

//  return container;
  return accountView;
}

WWidget *MyPrint::createListView() 
{
//  WContainerWidget *container = new WContainerWidget();
//  listView = new WTreeView(container);
  listView = new WTreeView();
// listView->doubleClicked().connect(SLOT(this, TreeViewDragDrop::editFile));
 
//  container->resize(600, WLength(100, WLength::Percentage));
  listView->resize(600, WLength(100, WLength::Percentage));

//  return container;
   return listView;
}

void MyPrint::loadAccounts()
{
 std::list<UserAccount*> accounts;
 WStandardItem *level1, *level2;
 WStandardItemModel* oldModel = accountModel;
 
 accountModel = new WStandardItemModel(0,1,this);
 

 if (adminMode) getAccountsWithOutput( ((HercConsole*)WApplication::instance())->dbConnection, member, accounts);
 else  getAllUserAccounts( ((HercConsole*)WApplication::instance())->dbConnection, member, accounts);

 char lasthost[10]; lasthost[0]=0;
 for(std::list<UserAccount*>::iterator it = accounts.begin(); it != accounts.end(); ++it) 
 {
    if (strcmp(lasthost, (*it)->host))
    { // New Host
      accountModel->appendRow(level1 = createHostItem((*it)->host));
      level1->appendRow(level2 = createAccountItem( (*it)->account ));
      strcpy(lasthost, (*it)->host);
    }
    else
    { // Same host
      level1->appendRow(level2 = createAccountItem( (*it)->account ));
    }
 }
 if (adminMode) accountModel->setHeaderData(0, Horizontal, boost::any(std::string("Admin")));
 else accountModel->setHeaderData(0, Horizontal, boost::any(std::string("User")));

 emptyUserAccount( accounts );

 accountView->setModel(accountModel);
 accountView->expandToDepth(1);
// accountView->setColumnResizeEnabled(false);
 accountView->setSortingEnabled(false);
// accountView->resize(WLength(100, WLength::Percentage),  WLength(100, WLength::Percentage));
 accountView->setColumnWidth(0, WLength(100, WLength::Percentage));
 accountView->setSelectionMode(SingleSelection);

 if ( accountModel->rowCount() && accountModel->columnCount()) 
    accountView->select(accountModel->index(0, 0, accountModel->index(0, 0)));

 if (oldModel) delete oldModel;
}


void MyPrint::loadList() 
{
 char host[10];
 char account[10];
 std::list<PrintOutput*> docs;
 WStandardItemModel* oldModel = listModel;
 
 listModel = new WStandardItemModel(0,6,this);
 
 if ( !accountView->selectedIndexes().empty() )
 {
   WModelIndex selected = *accountView->selectedIndexes().begin();
   strcpy(account, (boost::any_cast<WString>)(selected.data()).narrow().c_str());
   strcpy(host, (boost::any_cast<WString>)(selected.parent().data()).narrow().c_str());

   getAllPrintOutput( ((HercConsole*)WApplication::instance())->dbConnection, member, host, account, docs);
 
   int row=0;
   WStandardItem *item;
   char buffer[15];
   for(std::list<PrintOutput*>::iterator it = docs.begin(); it != docs.end(); ++it) 
   {
     listModel->invisibleRootItem()->setRowCount(row+1);

     // printnum
     sprintf(buffer, "%d",  (*it)->printnum);
     item = new WStandardItem(WString(buffer));
     item->setIcon("images/printer.png");
     listModel->setItem(row, 0, item);

     // jobname
     item = new WStandardItem();
     item->setText(WString((*it)->jobname));
     listModel->setItem(row, 1, item);

     // created
     item = new WStandardItem();
     item->setText(WString((*it)->created));
     listModel->setItem(row, 2, item);

     // retainuntil
     item = new WStandardItem();
     item->setText(WString((*it)->retainuntil));
     listModel->setItem(row, 3, item);

     // jobclass
     item = new WStandardItem();
     item->setText(WString((*it)->jobclass));
     listModel->setItem(row, 4, item);

     // jesjobnum
     item = new WStandardItem();
     sprintf(buffer, "%d",  (*it)->jesjobnum);
     item->setText(WString(buffer));
     listModel->setItem(row, 5, item);         

     row++; 
   }
 }
   
 listModel->setHeaderData(0, Horizontal, boost::any(std::string("Print Num")));
 listModel->setHeaderData(1, Horizontal, boost::any(std::string("Job Name")));
 listModel->setHeaderData(2, Horizontal, boost::any(std::string("Created")));
 listModel->setHeaderData(3, Horizontal, boost::any(std::string("Retain Until")));
 listModel->setHeaderData(4, Horizontal, boost::any(std::string("Job Class")));
 listModel->setHeaderData(5, Horizontal, boost::any(std::string("JES Job Num")));

 emptyPrintOutput( docs );

 listView->setModel(listModel);

// listView->resize(WLength(100, WLength::Percentage),WLength(100, WLength::Percentage));
 listView->sortByColumn(0, DescendingOrder);
 listView->setRootIsDecorated(false);
 listView->setAlternatingRowColors(true);
 listView->setSelectionMode(SingleSelection);
 listView->setDragEnabled(false);
 listView->setColumnWidth(0, 100); // printnum
 listView->setColumnWidth(1, 100); // jobname
 listView->setColumnWidth(2, 100); // created
 listView->setColumnWidth(3, 100); // retainuntil
 listView->setColumnWidth(4, 100); // jobclass
 listView->setColumnWidth(5, 100); // jesjobnum

// listView->refresh();

 if (oldModel) delete oldModel;
}
