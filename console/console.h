// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C C O N S O L E
// *************************************************************************
// Work Name   : HercConsole
// Description : This provides a user web console for Hercules Hosting
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : console.h
// Description : Header file
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

#ifndef herchosting_console_h
#define herchosting_console_h

#define VERSION "0.2.3 - 28 Nov 2009"
#define COPYRIGHT "(c) Adrian Sutherland 2009 - Licensed by the GNU General Public License"

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WStackedWidget>
#include <Wt/WVBoxLayout>
#include <Wt/WText>
#include <Wt/WMenu>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WTreeView>
#include <Wt/WAnchor>
#include <Wt/WVBoxLayout>

using namespace Wt;

class BasePage : public WContainerWidget
{
 public:
  BasePage() {};
  ~BasePage() {};
  virtual void activate() {};
  virtual void deactivate() {};
};


class MyIntro : public BasePage
{
 public:
  MyIntro();
  ~MyIntro();
};

#ifndef STANDALONE

class MyStatus : public BasePage
{
 public:
  MyStatus(char* user);
  ~MyStatus();
};

class MyPrint : public BasePage
{
 public:
  MyPrint(char* user, bool admin);
  ~MyPrint();

 private:
  bool adminMode;
  char member[20];
  WTreeView *accountView;
  WTreeView *listView;
  WStandardItemModel* listModel;
  WStandardItemModel* accountModel;

  void loadAccounts();
  void loadList();

  WStandardItem *createHostItem(const WString& host);
  WStandardItem *createAccountItem(const WString& account);

  WWidget *helpDisplay();   
  WWidget* createAccountView();  
  WWidget* createListView();  
};

class MyJobs : public BasePage
{
 public:
  MyJobs(char* user);
  ~MyJobs();
};

class MyPunch : public BasePage
{
 public:
  MyPunch(char* user);
  ~MyPunch();
};

class MyTape : public BasePage
{
 public:
  MyTape(char* user);
  ~MyTape();
};

class MyAccount : public BasePage
{
 public:
  MyAccount(char* user);
  ~MyAccount();
};

class MyWebservice : public BasePage
{
 public:
  MyWebservice(char* user);
  ~MyWebservice();
};

#endif

class MyTerminal;

class HercConsole : public WApplication
{
public:
  HercConsole(const WEnvironment& env);
  ~HercConsole();
  static WText *createTitle(const WString& title);
  char user[20];
  bool loadedTerminalJS;
#ifndef STANDALONE
  void* dbConnection;
#endif

private:
  WContainerWidget *bodyContainer;
  WContainerWidget *leftContainer;
  WContainerWidget *headerContainer;
  WContainerWidget *footerContainer;
  WContainerWidget *menu;
  MyIntro *intro;
  MyTerminal *terminal;
  WAnchor *introAnchor;
  WAnchor *terminalAnchor;
  void showIntro();
  void showTerminal();
  WAnchor* makeMenuAnchor(char* title);

#ifndef STANDALONE

  MyStatus *status;
  MyPrint *print;
  MyJobs *jobs;
  MyPunch *punch;
  MyTape *tape;
  MyAccount *account;
  MyWebservice *webservice;
  MyPrint *adminPrint;
  WAnchor *statusAnchor;
  WAnchor *printAnchor;
  WAnchor *jobsAnchor;
  WAnchor *punchAnchor;
  WAnchor *tapeAnchor;
  WAnchor *accountAnchor;
  WAnchor *webserviceAnchor;
  WAnchor *adminPrintAnchor;
  void showStatus();
  void showPrint();
  void showJobs();
  void showPunch();
  void showTape();
  void showAccount();
  void showWebservice();
  void showAdminPrint();

  // Login/Logoff Processing
  WStackedWidget *loginMenu;
  WLineEdit *useridEdit;
  WLineEdit *passwordEdit;
  WText *loginMessage;
  WContainerWidget *loginContainer;
  WContainerWidget *logoffContainer;
  void login();
  void logoff();
#endif
};

#endif
