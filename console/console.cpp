// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C C O N S O L E
// *************************************************************************
// Work Name   : HercConsole
// Description : This provides a user web console for Hercules Hosting
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : console.cpp
// Description : Main Application
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

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WBoxLayout>
#include <Wt/WLength>
#include <Wt/WMenu>
#include <Wt/WMessageBox>
#include <Wt/WVBoxLayout>

#include <iostream>
#include <syslog.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#ifndef STANDALONE
#include <herchosting.h> // Common Functionality
#endif
#include <console.h>
#include <terminal.h>

using namespace Wt;

WApplication *createApplication(const WEnvironment& env)
{
  return new HercConsole(env);
}

int main(int argc, char **argv)
{
  int result=0;
#ifndef STANDALONE
  openlog("console.fcgi", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
#else
  openlog("web3270.fcgi", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
#endif
  syslog(LOG_NOTICE, "Version: " VERSION);
  syslog(LOG_NOTICE, "Program started by User %d", getuid());

  // Seed Rand with current time
  srand(time(NULL));

#ifndef STANDALONE
  result = loadDBConfig();
#endif

  if (!result) result = WRun(argc, argv, &createApplication);

  closelog();

  return result;
}

HercConsole::~HercConsole()
{
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (intro) delete intro;
  if (terminal) delete terminal;
#ifndef STANDALONE
  if (status) delete status;
  if (print) delete print;
  if (jobs) delete jobs;
  if (punch) delete punch;
  if (tape) delete tape;
  if (account) delete account;
  if (webservice) delete webservice;
  if (adminPrint) delete adminPrint;
  disconnectDB(dbConnection);
#endif
}

HercConsole::HercConsole(const WEnvironment& env)
  : WApplication(env)
{
  loadedTerminalJS = false;
  intro = 0;
  terminal = 0;
#ifndef STANDALONE
  status = 0;
  print = 0;
  jobs = 0;
  punch = 0;
  tape = 0;
  account = 0;
  webservice = 0;
  adminPrint = 0;
  dbConnection = 0;
#endif

  useStyleSheet("openbpm.css");

  // Set-up Initial Page
#ifndef STANDALONE
  setTitle("Open-BPM - Hercules Hosting Console");
#else
  setTitle("Open-BPM - Standalone 3270 Emulator");
#endif

  bodyContainer = new WContainerWidget(root());
  leftContainer = new WContainerWidget(root());
  headerContainer = new WContainerWidget(root());
  footerContainer = new WContainerWidget(root());

  // Setup Header 
  headerContainer->setStyleClass("obpm-header");
  headerContainer->addWidget(new WImage("images/Open-BPM-tiny-logo.gif"));
#ifndef STANDALONE
  headerContainer->addWidget(new WText(" Open-BPM - Hercules Hosting Console"));
#else
  headerContainer->addWidget(new WText(" Open-BPM - Standalone 3270 Emulator"));
#endif

  // Setup Footer 
  footerContainer->setStyleClass("obpm-footer");
  footerContainer->setContentAlignment(AlignCenter);
#ifndef STANDALONE
  footerContainer->addWidget(new WText("The Open-BPM Hercules Hosting Console - Copyright Adrian Sutherland 2009"));
#else
  footerContainer->addWidget(new WText("The Open-BPM Standalone 3270 Emulator - Copyright Adrian Sutherland 2009"));
#endif

  // Setup Body
  bodyContainer->setStyleClass("obpm-body");

  intro = new MyIntro();
  intro->activate();
  bodyContainer->addWidget(intro);

  // Setup Left (Menu Area)
  leftContainer->setStyleClass("obpm-leftarea");

  // create a menu
  menu = new WContainerWidget(leftContainer);
  menu->setStyleClass("obpm-menu");

  // Intro Menu
  introAnchor = makeMenuAnchor("Main");
  introAnchor->clicked().connect(SLOT(this, HercConsole::showIntro));
  menu->addWidget(introAnchor);

#ifndef STANDALONE
  // Login Area
  leftContainer->addWidget(new WBreak());
  leftContainer->addWidget(new WText("<h1>Member Login</h1>"));

  loginMenu = new WStackedWidget(leftContainer); 
  loginContainer = new WContainerWidget(loginMenu); 

  loginContainer->addWidget(new WText("Username"));

  loginContainer->addWidget(new WBreak());
  useridEdit = new WLineEdit(loginContainer);
  useridEdit->setMaxLength(15);
  useridEdit->setFocus();

  loginContainer->addWidget(new WBreak());
  loginContainer->addWidget(new WText("Password"));

  loginContainer->addWidget(new WBreak());
  passwordEdit = new WLineEdit(loginContainer);
  passwordEdit->setMaxLength(15);
  passwordEdit->setEchoMode(WLineEdit::Password);

  loginContainer->addWidget(new WBreak());
  WPushButton *b1 = new WPushButton("Login", loginContainer);

  loginContainer->addWidget(new WBreak());
  loginContainer->addWidget(new WText("Register / Forgotten Password*"));

  loginContainer->addWidget(new WBreak());
  loginContainer->addWidget(new WBreak());
  loginContainer->addWidget(new WText("<i>*Registration via <a href=\"http://www.open-bpm.org/index.php/register.html\">www.open-bpm.org</a></i>"));

  logoffContainer = new WContainerWidget(loginMenu); 
  loginMessage = new WText(logoffContainer);
  logoffContainer->addWidget(new WBreak());
  logoffContainer->addWidget(new WBreak());
  WPushButton *b2 = new WPushButton("Logout", logoffContainer);

  loginMenu->setCurrentWidget(loginContainer);

  // Connect signals with slots
  b1->clicked().connect(SLOT(this, HercConsole::login));
  b2->clicked().connect(SLOT(this, HercConsole::logoff));
  passwordEdit->enterPressed().connect(SLOT(this, HercConsole::login));
#else
  // Terminal Menu
  terminalAnchor = makeMenuAnchor("3270 Terminal");
  terminalAnchor->clicked().connect(SLOT(this, HercConsole::showTerminal));
  menu->addWidget(terminalAnchor);
#endif
}

#ifndef STANDALONE
void HercConsole::login()
{
  char pass[20];
  user[19]=0;
  pass[19]=0;

  strncpy( user, useridEdit->text().narrow().c_str(), 19 );
  strncpy( pass, passwordEdit->text().narrow().c_str(), 19 );

  if (authenticate(user,pass))
  {
    WMessageBox::show ("Invalid Logon", "Invalid Logon details - have you registered with www.open-bpm.org?", Ok); 
    quit();
    redirect("/");
    return;
  }  

  // Connect to the database
  if (connectDB(dbConnection))
  {
    WMessageBox::show ("System Error", "Sorry there has been a system error (logged). Please let adrian@open-bpm.org know and try again later", Ok); 
    quit();
    redirect("/");
    return;
  }

  loginMessage->setText("You are logged in as " + useridEdit->text() + ".");
  loginMenu->setCurrentWidget(logoffContainer);

  // Host Status  Menu
  statusAnchor = makeMenuAnchor("Host Status");
  statusAnchor->clicked().connect(SLOT(this, HercConsole::showStatus));
  menu->addWidget(statusAnchor);

  // Print Output Menu
  printAnchor = makeMenuAnchor("Print Output");
  printAnchor->clicked().connect(SLOT(this, HercConsole::showPrint));
  menu->addWidget(printAnchor);

  // Jobs Menu
  jobsAnchor = makeMenuAnchor("Jobs");
  jobsAnchor->clicked().connect(SLOT(this, HercConsole::showJobs));
  menu->addWidget(jobsAnchor);

  // Punch Menu
  punchAnchor = makeMenuAnchor("Punch Output");
  punchAnchor->clicked().connect(SLOT(this, HercConsole::showPunch));
  menu->addWidget(punchAnchor);

  // Tape Menu
  tapeAnchor = makeMenuAnchor("Tapes");
  tapeAnchor->clicked().connect(SLOT(this, HercConsole::showTape));
  menu->addWidget(tapeAnchor);

  // Account Menu
  accountAnchor = makeMenuAnchor("Account Provisioning");
  accountAnchor->clicked().connect(SLOT(this, HercConsole::showAccount));
  menu->addWidget(accountAnchor);

  // Terminal Menu
  terminalAnchor = makeMenuAnchor("3270 Terminal");
  terminalAnchor->clicked().connect(SLOT(this, HercConsole::showTerminal));
  menu->addWidget(terminalAnchor);

  // Webservice Menu
  webserviceAnchor = makeMenuAnchor("Webservice");
  webserviceAnchor->clicked().connect(SLOT(this, HercConsole::showWebservice));
  menu->addWidget(webserviceAnchor);

  // Admin Menus
  if ( isAdmin( dbConnection, user ) > 0 )
  {
    // Admin Print Output Menu
    adminPrintAnchor = makeMenuAnchor("Print Admin");
    adminPrintAnchor->clicked().connect(SLOT(this, HercConsole::showAdminPrint));
    menu->addWidget(adminPrintAnchor);
  }
}
#endif

#ifndef STANDALONE
void HercConsole::logoff()
{
  loginMenu->setCurrentWidget(loginContainer);
  quit();
  redirect("/");
}
#endif

// Creates a title widget
WText *HercConsole::createTitle(const WString& title)
{
  WText *result = new WText(title);
  result->setInline(false);
  result->setStyleClass("obpm-title");
  return result;
}

void HercConsole::showIntro()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!intro) intro = new MyIntro();
  bodyContainer->addWidget(intro);
  intro->activate();
}

void HercConsole::showTerminal()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!terminal) terminal = new MyTerminal(user);
  bodyContainer->addWidget(terminal);
  terminal->activate();
}

#ifndef STANDALONE
void HercConsole::showStatus()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!status) status = new MyStatus(user);
  bodyContainer->addWidget(status);
  status->activate();
}

void HercConsole::showPrint()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!print) print = new MyPrint(user, false);
  bodyContainer->addWidget(print);
  print->activate();
}

void HercConsole::showJobs()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!jobs) jobs = new MyJobs(user);
  bodyContainer->addWidget(jobs);
  jobs->activate();
}

void HercConsole::showPunch()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!punch) punch = new MyPunch(user);
  bodyContainer->addWidget(punch);
  punch->activate();
}

void HercConsole::showTape()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!tape) tape = new MyTape(user);
  bodyContainer->addWidget(tape);
  tape->activate();
}

void HercConsole::showAccount()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!account) account = new MyAccount(user);
  bodyContainer->addWidget(account);
  account->activate();
}

void HercConsole::showWebservice()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!webservice) webservice = new MyWebservice(user);
  bodyContainer->addWidget(webservice);
  webservice->activate();
}

void HercConsole::showAdminPrint()
{
  ((BasePage*)(bodyContainer->widget(0)))->deactivate();
  bodyContainer->removeWidget(bodyContainer->widget(0));
  if (!adminPrint) adminPrint = new MyPrint(user, true);
  bodyContainer->addWidget(adminPrint);
  adminPrint->activate();
}

#endif

WAnchor* HercConsole::makeMenuAnchor(char* title)
{
  WAnchor *anchor = new WAnchor();
  anchor->setText(title);
  anchor->setRef("#");
  anchor->clicked().setPreventDefault(true);
  anchor->setInline(false);
  return anchor;
}
