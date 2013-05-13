// *************************************************************************
// A B O U T   T H I S   W O R K  -   T E L N E T P R O X Y
// *************************************************************************
// Work Name   : TelnetProxy
// Description : This provides a simple TelnetProxy with Authentication
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : typoauth.c
// Description : Authentication for typolight members
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
#include <string.h>
#include <mysql.h>
#include <openssl/sha.h>
#include <syslog.h>
#include <errno.h>
 
#define thequery "select password, disable from tl_member where username=\"%s\""
#define config "/usr/local/etc/typoauth"
#define version "typoauth version 0.2 (c) Adrian Sutherland" 

char *toHex(char* sha);

int main(int argc, char **argv)
{
   char inuserid[21];
   char checkuserid[42];
   char checkpassword[21];
   char *c;
   char host[100];
   char user[100];
   char pass[100];
   char db[100];
  
   MYSQL mysql;
   MYSQL_RES *res;
   MYSQL_ROW row;
         
   int i=0;
   char querybuffer[100];
   char hash[20];

   openlog ("typolight-member-auth", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

   syslog (LOG_NOTICE, version);

   /* Read Config File */
   FILE* cfg = fopen(config, "r");
   if (!cfg)
   {
      syslog (LOG_ERR, "Could not open config file %s - %s", config, strerror(errno));
      closelog ();
      exit(1);
   }
   if ( fgets(host, 100, cfg) == NULL ||
        fgets(db, 100, cfg) == NULL ||
        fgets(user, 100, cfg) == NULL ||
        fgets(pass, 100, cfg) == NULL )
   {
      syslog (LOG_ERR, "Could not read config file %s (bad format?) - %s", config, strerror(errno));
      closelog ();
      exit(1);
   }    
   if ((c = strchr(host,'\n')) != NULL) *c= '\0';
   if ((c = strchr(db,'\n')) != NULL) *c= '\0';
   if ((c = strchr(user,'\n')) != NULL) *c= '\0';
   if ((c = strchr(pass,'\n')) != NULL) *c= '\0';
        
   fclose(cfg);
   

   /* Read Userid and password */              
   if (fgets(inuserid, 20, stdin) == NULL || fgets(checkpassword, 20, stdin) == NULL) exit(1);

   if ((c = strchr(inuserid,'\n')) != NULL) *c= '\0';
   if ((c = strchr(checkpassword,'\n')) != NULL) *c= '\0';

   /* make connection to the database */
   mysql_init(&mysql);
   if (!mysql_real_connect(&mysql,host,user,pass,db,0,NULL,0))
   {
      syslog (LOG_ERR, "mysql_connect() Failed: %s", mysql_error(&mysql));
      closelog ();
      exit(1);
   }
                     
   /* Escape userid */                     
   mysql_real_escape_string(&mysql, checkuserid, inuserid, strlen(inuserid));  
                                                                                                       
   /* Setup the query */
   sprintf(querybuffer, thequery, checkuserid);
   if(mysql_query(&mysql,querybuffer))
   {
      syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(&mysql));
      closelog ();
      exit(1);
   }
                                                                                           
   /* store the result from our query */
   res = mysql_store_result(&mysql);
                                        
   int done=0;  
   char password[100];
   char* salt = 0;
   char* disable;     
                                                                                         
   /* fetch row */
   while((row = mysql_fetch_row(res))) {
     done++;
     strcpy(password, row[0]);
     disable = row[1];
   }
                                                                                                                 
   /* clean up */
   mysql_free_result(res);
   mysql_close(&mysql);

   if (done == 0) 
   {
     syslog (LOG_WARNING, "Invalid logon - Typolight Member not found - %s", checkuserid);
     closelog ();
     exit(1);
   }
  
   if (done!=1) 
   {
     syslog (LOG_ERR, "More than 1 Typelight member record found for %s", checkuserid);
     closelog ();
     exit(1);
   }
   
   if (strlen(disable)) 
   {
     syslog (LOG_WARNING, "Typelight member logon disable for user %s", checkuserid);
     closelog ();
     exit(1);
   }

   /* Check if the user password has any "salt" - i.e. from Typolight version 2.7 */
   for (i=0; password[i]; i++)
   {
     if (password[i]==':')
     {
       password[i] = 0;
       salt = password + i + 1; 
       break;
     }
   } 
   
   if (salt)
   {
     char temp[100];
     sprintf(temp, "%s%s", salt, checkpassword);
     SHA1(temp, strlen(temp), hash);
   }
   else SHA1(checkpassword, strlen(checkpassword), hash);
   
   if (strcmp(password,toHex(hash))) 
   {
     syslog (LOG_NOTICE, "Invalid logon - Wrong Password for user %s", checkuserid);
     closelog ();
     exit(1);                 
   }

   syslog (LOG_NOTICE, "External logon for user %s", checkuserid);
   closelog ();
   
   exit(0);
}


char *toHex(char* sha)
{
  static unsigned char const* digits = "0123456789abcdef";
  static unsigned char result[41];
  result[40]=0;
  int i,j;

  for (i=0,j=0; i<20; i++)
  {
    unsigned char c=(unsigned char)sha[i];
    result[j++] = digits[c >> 4];        
    result[j++] = digits[c & 15];    
  }
  return result;  
}
