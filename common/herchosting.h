// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C H O S T I N G   C O M M O N
// *************************************************************************
// Work Name   : Herchosting Common
// Description : Common Functionality for HercHosting project
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : herchosting.h
// Description : Header file for the common functionality
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
#ifndef herchosting_h
#define herchosting_h

#include <list>

// Load DB Configuration
int loadDBConfig();

// Database Connection
int connectDB( void*& connection );
int disconnectDB( void*& connection );

// Authenticate (with Typolight)
int authenticate(char* userid, char* password);

// User/Member Access Rights

int isAdmin( void* connection, char* user);

int isAdmin( void* connection, char* user, char* host);

int canAccessAccount( void* connection, char* user, char* host, char* account);

// User Accounts
class UserAccount
{
  public:
    char host[10]; // 8 (or 9 including null) should be enough - but hey!
    char account[10];
};

void emptyUserAccount( std::list<UserAccount*>& accounts );

int getAllUserAccounts( void* connection, char* userid, std::list<UserAccount*>& accounts);

int getAccountsWithOutput( void* connection, char* userid, std::list<UserAccount*>& accounts);

// Print Output
class PrintOutput
{
  public:
    int printnum;
    char jobname[9];
    char user[9];
    char host[9];
    char created[20];
    char retainuntil[20];
    char jobclass[2];
    int jesjobnum;
};

void emptyPrintOutput( std::list<PrintOutput*>& docs );

int getAllPrintOutput( void* connection, char* user, char* host, char* account, std::list<PrintOutput*>& docs);

#endif
