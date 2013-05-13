// *************************************************************************
// A B O U T   T H I S   W O R K  -   P R I N T H A N D L E R
// *************************************************************************
// Work Name   : PrintHandler
// Description : This provides a tool to split Hercules print output
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : printHandler.cpp
// Description : Linux (and perhaps other POSIX etc.) version
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

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <mysql.h>

#define version "version 0.5 (c) Adrian Sutherland 2009"
#define config "/usr/local/etc/hercauth"

#define saveOutput "insert into printoutput(host,user,jobname,jobclass,jesjobnum,created,retainuntil,printoutput) values (?,?,?,?,?,now(),now()+interval 1 week,?)"
#define BUFFER_SIZE 30

using namespace std;

int myReadLine(string &line)
{
  int result=0;
  char ch;
  bool start=true;
  
  line.clear();
  if (cin.eof()) return -1;
  while (cin.get(ch))
  {
    if (ch=='\n') break;  // Normal end of line
    if (ch=='\r') continue; // Ignore
    // Form feed is a tiny bit complex
    // if it is at the begining of a line ignore it but make sure we 
    // return 1 (form feed flag)
    // otherwise treat as a end of line - and push it back so that the next 
    // line read will have the form feed flag set
    if (ch=='\f')  // Form feed - ignore but make sure we return 1
    {
      if (start)
      { 
        result=1;
        continue;
      }
      else
      {
        cin.putback(ch);
        break;
      }
    }
    start=false;
    
    // Escape HTML 
    switch (ch)
    {
      case '"':
        line.append("&quot;");
        break;
      case '&':
        line.append("&amp;");
        break;
      case '>':
        line.append("&gt;");
        break;
      case '<':
        line.append("&lt;");
        break;
      default:
        line.append(1,ch);  
        break;
    }   
  }  
  return result;
}



int main(int argc, char* argv[])
{
  string breakln = "<b>=========================================================="
                   "=========================================================="
                   "================</b>";
  string line;

  string type;
  string num;
  string name;
  string jtime;
  string ampm;
  string dd;
  string mmm;
  string yy;
  string userid;
  string jobclass;
  int lookforuser = 0;
  bool injob = false;
  bool skipok;
  // Configuration to access database
  char dbhost[100];
  char dbuser[100];
  char dbpass[100];
  char db[100];

  MYSQL mysql;
  MYSQL_RES *res;
  MYSQL_ROW row;
  MYSQL_BIND bind[6];
  unsigned long length[6];
  char buffer[BUFFER_SIZE][6];
  MYSQL_STMT *stmt;
  
  cout << "printHandler Started - " version  << endl;   
  if (argc!=2)
  {
    cout << "printHandler Error - host name needed as argument - e.g. try \"printHandler MVS380\"" << endl;
    exit(-1);
  }
  if (strlen(argv[1])>29)
  {
    cout << "printHandler Error - host name too long!" << endl;
    exit(-1);
  }
  
  // Read database access configuration
  FILE* cfg = fopen(config, "r");
  if (!cfg)
  {
      cout << "Could not open config file " << config << " - " << strerror(errno) << endl;
      exit(-1);
  }
  if ( fgets(dbhost, 100, cfg) == NULL ||
       fgets(db, 100, cfg) == NULL ||
       fgets(dbuser, 100, cfg) == NULL ||
       fgets(dbpass, 100, cfg) == NULL )
  {
     cout << "Could not read config file " << config << " (bad format?) - " << strerror(errno) << endl;
     exit(-1);
  }
  char* c;    
  if ((c = strchr(dbhost,'\n')) != NULL) *c= '\0';
  if ((c = strchr(db,'\n')) != NULL) *c= '\0';
  if ((c = strchr(dbuser,'\n')) != NULL) *c= '\0';
  if ((c = strchr(dbpass,'\n')) != NULL) *c= '\0';
  fclose(cfg);

  // I want auto re-connect - might as well 
  mysql_init(&mysql);
  my_bool reconnect = 1;
  if (mysql_options(&mysql, MYSQL_OPT_RECONNECT, &reconnect)) 
  {
     cout << "mysql_option(reconnect) Failed: " <<  mysql_error(&mysql) << endl;
     exit(-1);
  }

  /* make connection to the database */
  if (!mysql_real_connect(&mysql,dbhost,dbuser,dbpass,db,0,NULL,0))
  {
     cout << "mysql_connect() Failed: " <<  mysql_error(&mysql) << endl;
     exit(-1);
  }

  // Main loop
  while (!cin.eof())
  {
     int formfeed;
     formfeed = myReadLine(line);
     skipok = false;
     
     // Check for start of Job
     if (!injob)
     {
       if ( ( line.substr(0,4).compare("****") == 0) &&
            ( line.substr(5,7).compare("  START") == 0) )
       {
         jobclass=line.substr(4,1);
         breakln.clear();
         breakln.append("<b>");
         breakln.append(line.length(),'='); 
         breakln.append("</b>");
         injob=true;
         formfeed = 0; // Supress formfeed at the begining of the job
       
         // Refresh for the job
         type.clear();
         num.clear();
         name.clear();
         jtime.clear();
         ampm.clear();
         dd.clear();
         mmm.clear();
         yy.clear();
         userid.clear();
         lookforuser = 0;
    
         stmt = mysql_stmt_init(&mysql);
         if (!stmt)
         {
           cout <<  "printHander Error - mysql_stmt_init() - out of memory" << endl;
           exit(-1);
         }
         if (mysql_stmt_prepare(stmt, saveOutput, strlen(saveOutput)))
         {
           cout <<  "printHander Error - mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << endl;
           exit(-1);
          }
          
          
          /* Get the parameter count from the statement */
          int param_count= mysql_stmt_param_count(stmt);
          
          if (param_count != 6) /* validate parameter count */
          {
            cout <<  "printHander Error - invalid parameter count returned by MySQL" << endl;
            exit(-1);
          }
          
          //Bind Variables
          memset(bind, 0, sizeof(bind));
          memset(length, 0, sizeof(length));
          // Host
          bind[0].buffer_type=MYSQL_TYPE_STRING;
          bind[0].buffer=buffer[0];
          bind[0].buffer_length=BUFFER_SIZE;
          bind[0].is_null=0;
          bind[0].length=&length[0];
          // user
          bind[1].buffer_type=MYSQL_TYPE_STRING;
          bind[1].buffer=buffer[1];
          bind[1].buffer_length=BUFFER_SIZE;
          bind[1].is_null=0;
          bind[1].length=&length[1];
          // jobname 
          bind[2].buffer_type=MYSQL_TYPE_STRING;
          bind[2].buffer=buffer[2];
          bind[2].buffer_length=BUFFER_SIZE;
          bind[2].is_null=0;
          bind[2].length=&length[2];
          // jobclass
          bind[3].buffer_type=MYSQL_TYPE_STRING;
          bind[3].buffer=buffer[3];
          bind[3].buffer_length=BUFFER_SIZE;
          bind[3].is_null=0;
          bind[3].length=&length[3];
          // jesjobnum
          bind[4].buffer_type=MYSQL_TYPE_STRING;
          bind[4].buffer=buffer[4];
          bind[4].buffer_length=BUFFER_SIZE;
          bind[4].is_null=0;
          bind[4].length=&length[4];
          // printoutput 
          bind[5].buffer_type=MYSQL_TYPE_STRING;
          bind[5].is_null=0;
          bind[5].length=&length[5];
          
          /* Bind the buffers */
          if (mysql_stmt_bind_param(stmt, bind))
          {
            cout <<  "printHander Error - param bind failed: " << mysql_stmt_error(stmt) << endl;
            exit(-1);
          }          

          // Supply printoutput in chunks to server
          char *header = "<html><head></head><body><pre>\n";
          if (mysql_stmt_send_long_data(stmt,5,header,strlen(header)))
          {
             cout <<  "printHander Error - mysql_stmt_send_long_data: " << mysql_stmt_error(stmt) << endl;
             exit(-1);
          }
       }
     }  
          
     // Check for end of job (and save job file)
     if (injob)
     {
       if ( line.substr(0,11).compare("****" + jobclass + "   END") == 0)
       {
         // End of Job
         char* buf = new char[line.length()+1];
         strcpy( buf, line.c_str());
       
         // Parse Footer for Job Info
         strtok( buf, " " ); // ****Z
         strtok( NULL, " " ); // END
         type = strtok( NULL, " " );
         num = strtok( NULL, " " );
         name = strtok( NULL, " " );
         strtok( NULL, " " ); // ROOM
         jtime = strtok( NULL, " " );
         ampm = strtok( NULL, " " );
         dd = strtok( NULL, " " );
         mmm = strtok( NULL, " " );
         yy = strtok( NULL, " " );
         delete[] buf;
       
         // Supply printoutput in chunks to server
         char *footer = "</pre></body></html>\n";
         if (mysql_stmt_send_long_data(stmt,5,footer,strlen(footer)))
         {
            cout <<  "printHander Error - mysql_stmt_send_long_data: " << mysql_stmt_error(stmt) << endl;
            exit(-1);
         }
    
         if (type.compare("JOB") == 0)
         {
           if ( userid.empty() ) userid = "UNKNOWN";
         }
         else userid = type;

         // Host
         strcpy(buffer[0], argv[1]);
         length[0] = strlen(buffer[0]);
         // user
         strcpy(buffer[1], userid.c_str());
         length[1] = strlen(buffer[1]);
         // jobname 
         strcpy(buffer[2], name.c_str());
         length[2] = strlen(buffer[2]);
         // jobclass
         strcpy(buffer[3], jobclass.c_str());
         length[3] = strlen(buffer[3]);
         // jesjobnum
         strcpy(buffer[4], num.c_str());
         length[4] = strlen(buffer[4]);

         // Execute the INSERT statement
         if (mysql_stmt_execute(stmt))
         {
            cout <<  "printHander Error - mysql_stmt_execute: " << mysql_stmt_error(stmt) << endl;
            exit(-1);
         }
      
         // Get the total rows affected 
         int affected_rows= mysql_stmt_affected_rows(stmt);
         if (affected_rows != 1) // validate affected rows
         {
            cout <<  "printHander Error - invalid affected rows by MySQL" << endl;
            exit(-1);
         }
                          
         // Close the statement 
         if (mysql_stmt_close(stmt))
         {
            cout <<  "printHander Error - mysql_stmt_close: " << mysql_stmt_error(stmt) << endl;
            exit(-1);
         }

         injob=false;
         jobclass.empty();
         skipok = true;
       }
     }
     
     // Stuff to find the job user
     if (injob)
     {
       // find "USER=" - but only if we have not already got a userid etc
       if (lookforuser > 0 && lookforuser < 11) // Only look at the first 10 lines after the job statistics
       {
         if (userid.empty())
         {
           string::size_type loc = line.find( "USER=");
           if( loc != string::npos ) 
           {
             string u = line.substr(loc+5);
             string::size_type l = u.find_first_of( ", ");
             if( l == string::npos ) userid = u;
             else userid = u.substr(0,l);
           }
         }
         lookforuser++;  
       }  
     
       if (lookforuser == 0) // Have not reached the end of the job statistics yet
       {
         if ( line.find("MINUTES EXECUTION TIME") != string::npos ) lookforuser = 1;
       }  
     }

     // Stuff to write the output     
     if (injob) // Otherwise silently ignore - which also means the job trailor is ignored - a good thing
     { 
       // Write out
       ostringstream tempStream;       
       
       if (formfeed)
       {
         tempStream << breakln << endl << endl << breakln << endl;
       }
       tempStream << line << endl;
       
       // Supply printoutput in chunks to server
       string st = tempStream.str();
       if (mysql_stmt_send_long_data(stmt,5,st.c_str(),st.length()))
       {
         cout <<  "printHander Error - mysql_stmt_send_long_data: " << mysql_stmt_error(stmt) << endl;
         exit(-1);
       }
     }  
     else if (!line.empty() && !skipok)
     {
       cout << "printHandler Warning - Skipped: " << line << endl; 
     }  
  }
  
  mysql_close(&mysql);
  return 0;
}
