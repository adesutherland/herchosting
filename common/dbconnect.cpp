// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C H O S T I N G   C O M M O N 
// *************************************************************************
// Work Name   : HercHosting Common
// Description : This provides common functionality for herchosting
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : dbconfig.cpp
// Description : Functions to Read Database config file, connect and
//             : disconnect
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
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <mysql.h>
 
#include "herchosting.h"

#define config "/usr/local/etc/hercauth"
 
using namespace std;

// Configuration to access database
static char dbhost[100];
static char dbuser[100];
static char dbpass[100];
static char db[100];

int loadDBConfig()
{
  // Read database access configuration
  FILE* cfg = fopen(config, "r");
  if (!cfg)
  {
      syslog (LOG_ERR, "Could not open config file %s - %s", config, strerror(errno));
      return -1;
  }
  if ( fgets(dbhost, 100, cfg) == NULL ||
       fgets(db, 100, cfg) == NULL ||
       fgets(dbuser, 100, cfg) == NULL ||
       fgets(dbpass, 100, cfg) == NULL )
  {
     syslog (LOG_ERR, "Could not read config file %s (bad format?) - %s", config, strerror(errno));
     return -1;
  }
  char* c;    
  if ((c = strchr(dbhost,'\n')) != NULL) *c= '\0';
  if ((c = strchr(db,'\n')) != NULL) *c= '\0';
  if ((c = strchr(dbuser,'\n')) != NULL) *c= '\0';
  if ((c = strchr(dbpass,'\n')) != NULL) *c= '\0';
  fclose(cfg);
  
  return 0;
}

int connectDB( void*& connection )
{
  MYSQL *mysql;
  mysql = new MYSQL;
  
  mysql_init(mysql);
  if (!mysql_real_connect(mysql,dbhost,dbuser,dbpass,db,0,NULL,0))
  {
    syslog (LOG_ERR, "mysql_connect() Failed: %s", mysql_error(mysql));
    return -1;
  }
  connection = (void*)mysql;
  return 0;
}

int disconnectDB( void*& connection )
{
  if (connection)
  {
    MYSQL *mysql = (MYSQL*)connection;
    mysql_close(mysql);
    connection = 0;
  }  
  return 0;
}  

