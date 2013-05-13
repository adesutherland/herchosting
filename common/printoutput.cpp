// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C H O S T I N G   C O M M O N
// *************************************************************************
// Work Name   : Herchosting Common
// Description : Common Functionality for HercHosting project
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : printoutput.cpp
// Description : Printer Output Functionality
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

#define GETALLPRINTOUTPUT "select printnum,jobname,user,host,created,retainuntil,jobclass,jesjobnum from printoutput where host=\"%s\" and user=\"%s\" order by printnum desc"

void emptyPrintOutput( list<PrintOutput*>& docs )
{
  for(list<PrintOutput*>::iterator it = docs.begin(); it != docs.end(); ++it) delete *it;
  docs.clear();
}

int getAllPrintOutput( void* connection, char* user, char* host, char* account, list<PrintOutput*>& docs)
{
  MYSQL_RES *res;
  MYSQL_ROW row;  
  MYSQL *mysql = (MYSQL*)connection;

  // Check Access
  if ( canAccessAccount( connection, user, host, account ) > 0 )
  {
    /* Escape userid */                     
    char checkedhost[50];
    char checkedaccount[50];
    char querybuffer[250];
  
    mysql_real_escape_string(mysql, checkedhost, host, strlen(host));  
    mysql_real_escape_string(mysql, checkedaccount, account, strlen(account));  

    /* Setup the query */  
    sprintf(querybuffer, GETALLPRINTOUTPUT, checkedhost, checkedaccount);

cout << "sql: " << querybuffer << endl;                                                                                                     
   
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
      PrintOutput *job = new PrintOutput;
      job->printnum = 0;
      job->jobname[0] = 0;
      job->user[0] = 0;
      job->host[0] = 0;
      job->created[0] = 0;
      job->retainuntil[0] = 0;
      job->jobclass[0] = 0;
      job->jesjobnum = 0;

      job->printnum = atoi(row[0]);
      strcpy(job->jobname, row[1]);
      strcpy(job->user, row[2]);
      strcpy(job->host, row[3]);
      strcpy(job->created, row[4]);
      strcpy(job->retainuntil, row[5]);
      strcpy(job->jobclass, row[6]);
      job->jesjobnum = atoi(row[7]);

      docs.push_back( job );
    }

    /* Clean-up MYSQL result*/
    mysql_free_result(res);
  } 
                                                                     
  return 0;
}
