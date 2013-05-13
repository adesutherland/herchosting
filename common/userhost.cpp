// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C H O S T I N G   C O M M O N
// *************************************************************************
// Work Name   : Herchosting Common
// Description : Common Functionality for HercHosting project
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : userhost.cpp
// Description : user and host common functions
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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mysql.h>
#include <list>

#include "herchosting.h"
 
using namespace std;

#define ISADMIN "select count(*) from userhosts where user=\"%s\" and adminflag=1"
#define ISADMINHOST "select count(*) from userhosts where user=\"%s\" and host=\"%s\" and adminflag=1"
#define CANACCESSACCOUNT "select count(*) from useraccounts where user=\"%s\" and host=\"%s\" and account=\"%s\""
#define GETALLUSERACCOUNTS "select host, account from useraccounts where user=\"%s\" order by host, account"
#define GETACCOUNTSWITHOUTPUT "select distinct host, user from printoutput where host in (select host from userhosts where adminflag=1 and user=\"%s\") order by host, user"


void emptyUserAccount( list<UserAccount*>& accounts )
{
  for(list<UserAccount*>::iterator it = accounts.begin(); it != accounts.end(); ++it) delete *it;
  accounts.clear();
}


int getAllUserAccounts( void* connection, char* userid, list<UserAccount*>& accounts)
{
  MYSQL_RES *res;
  MYSQL_ROW row;  
  MYSQL *mysql = (MYSQL*)connection;

  /* Escape userid */                     
  char checkeduserid[50];
  char querybuffer[200];
  mysql_real_escape_string(mysql, checkeduserid, userid, strlen(userid));  
                                                                                                     
  /* Setup the query */
  sprintf(querybuffer, GETALLUSERACCOUNTS, checkeduserid);
   
  // Run SQL Query 
  if(mysql_query(mysql,querybuffer))
  {
     syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(mysql));
     return -1;
  }
                                                                                           
  /* store the result from our query */
  res = mysql_store_result(mysql);                                       
  
  /* fetch and store result */
  while((row = mysql_fetch_row(res))) {
    UserAccount *acnt = new UserAccount;
    strcpy(acnt->host, row[0]);
    strcpy(acnt->account, row[1]);
    accounts.push_back( acnt );
  }

  /* Clean-up MYSQL result*/
  mysql_free_result(res);
                                                                      
  return 0;
}


int getAccountsWithOutput( void* connection, char* userid, list<UserAccount*>& accounts)
{
  MYSQL_RES *res;
  MYSQL_ROW row;  
  MYSQL *mysql = (MYSQL*)connection;

  /* Escape userid */                     
  char checkeduserid[50];
  char querybuffer[200];
  mysql_real_escape_string(mysql, checkeduserid, userid, strlen(userid));  
                                                                                                     
  /* Setup the query */
  sprintf(querybuffer, GETACCOUNTSWITHOUTPUT, checkeduserid);
   
  // Run SQL Query 
  if(mysql_query(mysql,querybuffer))
  {
     syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(mysql));
     return -1;
  }
                                                                                           
  /* store the result from our query */
  res = mysql_store_result(mysql);                                       
  
  /* fetch and store result */
  while((row = mysql_fetch_row(res))) {
    UserAccount *acnt = new UserAccount;
    strcpy(acnt->host, row[0]);
    strcpy(acnt->account, row[1]);
    accounts.push_back( acnt );
  }

  /* Clean-up MYSQL result*/
  mysql_free_result(res);
                                                                      
  return 0;
}


int isAdmin( void* connection, char* userid)
{
  MYSQL_RES *res;
  MYSQL_ROW row;  
  MYSQL *mysql = (MYSQL*)connection;
  int result = 0;

  /* Escape userid */                     
  char checkeduserid[50];
  char querybuffer[200];
  mysql_real_escape_string(mysql, checkeduserid, userid, strlen(userid));  
                                                                                                     
  /* Setup the query */
  sprintf(querybuffer, ISADMIN, checkeduserid);
   
  // Run SQL Query 
  if(mysql_query(mysql,querybuffer))
  {
     syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(mysql));
     return -1;
  }
                                                                                           
  /* store the result from our query */
  res = mysql_store_result(mysql);                                       
  
  /* fetch and store result */
  row = mysql_fetch_row(res);
  if (row)
  {  
    if (strcmp("0", row[0])) result = 1;
    else result = 0;
  }
  else result = -1;

  /* Clean-up MYSQL result*/
  mysql_free_result(res);
                                                                      
  return result;
}

int isAdmin( void* connection, char* user, char* host)
{
  MYSQL_RES *res;
  MYSQL_ROW row;  
  MYSQL *mysql = (MYSQL*)connection;
  int result = 0;

  /* Escape userid and host */                     
  char checkeduserid[50];
  char checkedhost[50];
  char querybuffer[200];
  mysql_real_escape_string(mysql, checkeduserid, user, strlen(user));  
  mysql_real_escape_string(mysql, checkedhost, host, strlen(host));  
                                                                                                     
  /* Setup the query */
  sprintf(querybuffer, ISADMINHOST, checkeduserid, checkedhost);
   
  // Run SQL Query 
  if(mysql_query(mysql,querybuffer))
  {
     syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(mysql));
     return -1;
  }
                                                                                           
  /* store the result from our query */
  res = mysql_store_result(mysql);                                       
  
  /* fetch and store result */
  row = mysql_fetch_row(res);
  if (row)
  {  
    if (strcmp("0", row[0])) result = 1;
    else result = 0;
  }
  else result = -1;

  /* Clean-up MYSQL result*/
  mysql_free_result(res);
                                                                      
  return result;
}

int canAccessAccount( void* connection, char* user, char* host, char* account)
{ 
  int result;
  
  // First Check Admin Access
  result = isAdmin(connection, user, host);
  
  if (result == 0) 
  {  // Does not have admin access so check account
    MYSQL_RES *res;
    MYSQL_ROW row;  
    MYSQL *mysql = (MYSQL*)connection;

    /* Escape userid and host */                     
    char checkeduserid[50];
    char checkedhost[50];
    char checkedaccount[50];
    char querybuffer[200];
    mysql_real_escape_string(mysql, checkeduserid, user, strlen(user));  
    mysql_real_escape_string(mysql, checkedhost, host, strlen(host));  
    mysql_real_escape_string(mysql, checkedaccount, account, strlen(account));  
                                                                                                     
    /* Setup the query */
    sprintf(querybuffer, CANACCESSACCOUNT, checkeduserid, checkedhost, checkedaccount);
   
    // Run SQL Query 
    if(mysql_query(mysql,querybuffer))
    {
       syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(mysql));
       return -1;
    }
                                                                                           
    /* store the result from our query */
    res = mysql_store_result(mysql);                                       
  
    /* fetch and store result */
    row = mysql_fetch_row(res);
    if (row)
    {  
      if (strcmp("0", row[0])) result = 1;
      else result = 0;
    }
    else result = -1;

    /* Clean-up MYSQL result*/
    mysql_free_result(res);
  }
  
  return result;
}
