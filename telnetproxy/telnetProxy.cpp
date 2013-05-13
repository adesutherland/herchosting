// *************************************************************************
// A B O U T   T H I S   W O R K  -   T E L N E T P R O X Y
// *************************************************************************
// Work Name   : TelnetProxy
// Description : This provides a simple TelnetProxy with Authentication
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : telnetProxy.cpp
// Description : Linux (and perhaps other POSIX etc.) version of TelnetProxy
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

#define version "Version 0.3 (c) Adrian Sutherland 2009"
#define auth "typoauth"
#define authpath "/usr/local/bin/typoauth"

#define defaultlistenport 3270
#define getlistenport "select value from configuration where name=\"telnetProxyPort\""
#define getHosts "select u.host, h.description, h.status, h.server, h.tn3270port from userhosts u, hosts h where u.host=h.host and u.user=\"%s\""
#define config "/usr/local/etc/hercauth"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mysql.h>
#include <map>
 
using namespace std;

// Configuration to access database
char dbhost[100];
char dbuser[100];
char dbpass[100];
char db[100];

void HandleSession(int socket);
void* forwardThread(void* lpvThreadParam);
void removeWhiteSpace(char *text);

struct forwardData
{
  int fromSocket;
  int toSocket;
  bool *dead;
};

int main()
{
  int listenPort = defaultlistenport;
  int listenSocket;
  int sessionSocket;
  pid_t childpid;
  MYSQL mysql;
  MYSQL_RES *res;
  MYSQL_ROW row;
  

  signal(SIGCLD, SIG_IGN);  // To get rid of the zombies I am not waiting for

  struct sockaddr_in address; /* Internet socket address stuct */
  int addressSize=sizeof(struct sockaddr_in);

  openlog("telnetProxy", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  syslog(LOG_NOTICE, version);
  syslog(LOG_NOTICE, "Program started by User %d", getuid());

  // Read database access configuration
  FILE* cfg = fopen(config, "r");
  if (!cfg)
  {
      syslog (LOG_ERR, "Could not open config file %s - %s", config, strerror(errno));
      closelog ();
      exit(1);
  }
  if ( fgets(dbhost, 100, cfg) == NULL ||
       fgets(db, 100, cfg) == NULL ||
       fgets(dbuser, 100, cfg) == NULL ||
       fgets(dbpass, 100, cfg) == NULL )
  {
     syslog (LOG_ERR, "Could not read config file %s (bad format?) - %s", config, strerror(errno));
     closelog ();
     exit(1);
  }
  char* c;    
  if ((c = strchr(dbhost,'\n')) != NULL) *c= '\0';
  if ((c = strchr(db,'\n')) != NULL) *c= '\0';
  if ((c = strchr(dbuser,'\n')) != NULL) *c= '\0';
  if ((c = strchr(dbpass,'\n')) != NULL) *c= '\0';
  fclose(cfg);

  /* Read listen port from configuration table */
  /* make connection to the database */
  mysql_init(&mysql);
  if (!mysql_real_connect(&mysql,dbhost,dbuser,dbpass,db,0,NULL,0))
  {
     syslog (LOG_ERR, "mysql_connect() Failed: %s", mysql_error(&mysql));
     closelog ();
     exit(1);
  }
   
  // Run SQL Query 
  if(mysql_query(&mysql,getlistenport))
  {
     syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(&mysql));
     closelog ();
     exit(1);
  }
                                                                                           
  /* store the result from our query */
  res = mysql_store_result(&mysql);                                       
  
  /* fetch row */
  while((row = mysql_fetch_row(res))) {
    listenPort = atoi(row[0]);
  }
                                                                                                                 
  /* clean up database connection - not needed anymore*/
  mysql_free_result(res);
  mysql_close(&mysql);

  /* make a socket */
  listenSocket=socket(AF_INET,SOCK_STREAM,0);
  if (listenSocket < 0)
  {
     syslog (LOG_ERR, "Could not make socket");
     closelog ();
     exit(1);
  }

  /* fill address struct */
  address.sin_addr.s_addr=INADDR_ANY;
  address.sin_port=htons(listenPort);
  address.sin_family=AF_INET;

  /* bind to a port */
  if (bind(listenSocket,(struct sockaddr*)&address,sizeof(address)) < 0)
  {
     syslog (LOG_ERR, "Could not bind to port %d", listenPort);
     closelog ();
     exit(1);
  }

  /* establish listen queue */
  if (listen(listenSocket,5) < 0)
  {
     syslog (LOG_ERR, "Could not Listen");
     closelog ();
     exit(1);
  }

  while (1)
  {
     /* get the connected socket */
     sessionSocket=accept(listenSocket,(struct sockaddr*)&address,(socklen_t *)&addressSize);
     if (sessionSocket<0)
     {
       syslog (LOG_ERR, "Could not accept connection");
     }
     syslog (LOG_NOTICE, "Connection Accepted");

     // Fork a process to handle session
     childpid = fork();
       
     if (childpid >= 0) /* fork succeeded */
     {
        if (childpid == 0) /* child process */
        {
          close(listenSocket);
          HandleSession(sessionSocket);
          exit(0);
        }
        else close(sessionSocket);                
     }
     
     else
     {
       syslog (LOG_ERR, "Error doing fork()");
     }
  }
  
  /* FIXME - Never Gets here - needs a signal handler */
  closelog ();
  exit(0);
}

void HandleSession(int sessionSocket)
{
  char userid[21];
  char password[21];
  int n;
  struct sockaddr_in Address;
  char dummy[21];
  MYSQL mysql;
  MYSQL_RES *res;
  MYSQL_ROW row;

  char *echo_off="\xff\xfb\x01"; // i.e. Server Will Echo
  char *echo_on="\xff\xfc\x01"; // I.e. Server Wont Echo 
  char *line_mode="\xff\xfd\x22"; // I.e. Client you go into do line mode
  char *no_line_mode="\xff\xfe\x22"; // I.e. Client you do not go into do line mode
  char *welcome="\x1b[37mWelcome to the \x1b[35mOpen-BPM\x1b[37m Telnet Proxy\n\x1b[34m" version "\n\n\x1b[31m\x1b[5mUnauthorised access is prohibited\x1b[0m\n";
  char *userid_prompt="\n\x1b[37mUser ID: \x1b[32m";
  char *password_prompt="\n\x1b[37mPassword: \x1b[32m";
  char *ok_message="\n\n\x1b[37mThank you - I am connecting you to the host\n"; 
  char *bad_message="\n\n\x1b[33mINVALID - PLEASE GO AWAY!\n";
  char *error_message="\n\n\x1b[33mSorry there has been a system error\nPlease report this and try again later\n";
  char *nohost_message="\n\n\x1b[33mSorry, there are no available hosts that you have access to\nHave you set up an account?\n";
  char *list_title="\n\n\x1b[37mAvailable Hosts\n\nSel Host     Description\n";
  char *available_host="\x1b[37m%-3d \x1b[32m%-8s %s\n";
  char *unavailable_host="\x1b[32mN/A %-8s %s (%s)\n";
  char *host_select="\n\n\x1b[37mSelect Host (1-%d):\x1b[32m ";
  char *invalid_select="\n\x1b[33mInvalid Section";

  /* Set keepalive option */
  int optval = 1;
  socklen_t optlen = sizeof(optval);
  if(setsockopt(sessionSocket, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
    syslog (LOG_ERR, "setsocketopt error - %s", strerror(errno));
    close(sessionSocket);
    return;
  }

  // Put client in Line Mode
  n = write(sessionSocket,line_mode,strlen(line_mode));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  n = read(sessionSocket,dummy,20);
  if (n < 0) 
  {
    syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }    
  
  // Write Welcome
  n = write(sessionSocket,welcome,strlen(welcome));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }

  // Prompt and get userid
  n = write(sessionSocket,userid_prompt,strlen(userid_prompt));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  bzero(userid,21);
  n = read(sessionSocket,userid,20);
  if (n < 0) 
  {
    syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }    
  
  // Check for too long response
  if (n >= 20) 
  {
    syslog (LOG_WARNING, "Buffer Overflow hack attempt");
    write(sessionSocket,bad_message,strlen(bad_message));
    close(sessionSocket);
    return;
  }    

  // All This echo nonsense is to hide the password.

  // Echo Off
  n = write(sessionSocket,echo_off,strlen(echo_off));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  n = read(sessionSocket,dummy,20);
  if (n < 0) 
  {
    syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }

  // Put client out of Line Mode
/* doesn't seem to work or be needed with c2370 (at least)
  n = write(sessionSocket,no_line_mode,strlen(no_line_mode));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  n = read(sessionSocket,dummy,20);
  if (n < 0) 
  {
    syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }    
*/  

  // Password Prompt
  n = write(sessionSocket,password_prompt,strlen(password_prompt));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  
  // Read Password - character by character
  bzero(password,21);
  int i;
  char c;
  bool done = false;
  for(n=0;n<21 && !done;)
  {
    do
    {
      i = read(sessionSocket, &c, 1);
      if(i < 0) 
      {
        syslog (LOG_ERR, "ERROR reading from socket - %s", strerror(errno));
        close(sessionSocket);
        return;
      }
    } while(i == 0);
    switch(c)
    {
      case '\r':
        password[n] = '\0';
        done=true;
        break;
      case '\b':
        if(n > 0) 
        {
          n--;
          if (write(sessionSocket,"\b \b",3) < 0)
          {
            syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
            close(sessionSocket);
            return;
          }
        }  
        break;
      case 27: /* special keys - just finish */
        password[n] = '\0';
        done=true;
        break;
      default:
        password[n++] = c;
        if (write(sessionSocket,"*",1) < 0)
        {
          syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
          close(sessionSocket);
          return;
        }
        break;
    }
  }

  // Reply too long
  if (n >= 20) 
  {
    syslog (LOG_WARNING, "Buffer Overflow hack attempt");
    write(sessionSocket,bad_message,strlen(bad_message));
    close(sessionSocket);
    return;
  }    

  // Dump any extra characters (like LF or codes after the ESC)
  n = read(sessionSocket,dummy,20);
  if (n < 0) 
  {
    syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }    
  
  // Put client in Line Mode
  n = write(sessionSocket,line_mode,strlen(line_mode));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  n = read(sessionSocket,dummy,20);
  if (n < 0) 
  {
    syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }    
 
  // Echo On  
  n = write(sessionSocket,echo_on,strlen(echo_on));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  n = read(sessionSocket,dummy,20);
  if (n < 0) 
  {
    syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }    

  // Check Security
  removeWhiteSpace(userid);
  removeWhiteSpace(password);

  int pipe_to_auth[2];
  int pid;
  int rc;
  
  pipe(pipe_to_auth);

  signal(SIGCLD, SIG_DFL);  // I do want to wait for my child (I set it to ignore before)!

  pid = fork();

  if (pid == 0)
  {
    /* We are the child process */

    /* FIXME - Close any open FDs */
  
    /* Connect stdin to pipe */
    dup2(pipe_to_auth[0], 0);
    close(pipe_to_auth[0]);
    close(pipe_to_auth[1]);

    /* exec the authenticator program */
    execl(authpath, auth, NULL);

    /* If exec failed */ 
    exit(-1);
  }
  else
  {
    /* We are the parent process */

    close(pipe_to_auth[0]);

    /* Send the user */
    write(pipe_to_auth[1], userid, strlen(userid));
    write(pipe_to_auth[1], "\n", 1);
 
    /* Send the password */
    write(pipe_to_auth[1], password, strlen(password));
    write(pipe_to_auth[1], "\n", 1);

    close(pipe_to_auth[1]);

    waitpid(pid, &rc, 0);
  }
  
  if (rc == -1)
  {
    syslog (LOG_ERR, "ERROR calling system(auth) - %s", strerror(errno));
    write(sessionSocket,error_message,strlen(error_message));
    sleep(5);
    close(sessionSocket);
    return;
  }    
 
  rc=WEXITSTATUS(rc);
  if (rc)
  {
    write(sessionSocket,bad_message,strlen(bad_message));
    sleep(3);
    close(sessionSocket);
    return;
  } 

  n = write(sessionSocket,list_title,strlen(list_title));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    closelog();
    return;
  }      

  /* Get the user's hosts */
  /* make connection to the database */
  mysql_init(&mysql);
  if (!mysql_real_connect(&mysql,dbhost,dbuser,dbpass,db,0,NULL,0))
  {
     syslog (LOG_ERR, "mysql_connect() Failed: %s", mysql_error(&mysql));
     write(sessionSocket,error_message,strlen(error_message));
     sleep(5);
     close(sessionSocket);
     closelog ();
     exit(1);
  }

  /* Escape userid */                     
  char checkeduserid[50];
  char querybuffer[200];
  mysql_real_escape_string(&mysql, checkeduserid, userid, strlen(userid));  
                                                                                                     
  /* Setup the query */
  sprintf(querybuffer, getHosts, checkeduserid);
   
  // Run SQL Query 
  if(mysql_query(&mysql,querybuffer))
  {
     syslog (LOG_ERR, "mysql_query() Failed: %s", mysql_error(&mysql));
     closelog ();
     write(sessionSocket,error_message,strlen(error_message));
     sleep(5);
     close(sessionSocket);
     exit(1);
  }
                                                                                           
  /* store the result from our query */
  res = mysql_store_result(&mysql);                                       
  
  /* fetch hosts */
  map<const int, int> ports;
  map<const int, char*> servers;
  char output[200];
  i = 0;
  while((row = mysql_fetch_row(res))) {
    if ( strcmp(row[2],"AVAILABLE")==0 )
    {
      i++;
      sprintf(output, available_host,i,row[0],row[1]);
      ports.insert( make_pair(i, atoi(row[4])) );
      char *sv = new char(strlen(row[3]));
      strcpy(sv,row[3]);
      servers.insert( make_pair(i, sv) );
    }
    else
    {
      sprintf(output, unavailable_host,row[0],row[1],row[2]);
    }
    n = write(sessionSocket,output,strlen(output));
    if (n < 0) {
      syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
      close(sessionSocket);
      closelog();
      return;
    }      
  }

  /* clean up database connection - not needed anymore*/
  mysql_free_result(res);
  mysql_close(&mysql);

  int hostport;
  char host[41];
                           
  if (i == 0)
  {
     closelog ();
     write(sessionSocket,nohost_message,strlen(nohost_message));
     sleep(5);
     close(sessionSocket);
     exit(1);
  }
  else if (i==1)
  {  // Only one host - just go to it
     hostport = ports[1];
     strcpy(host, servers[1]);
  }
  else
  {  // Multiple Hosts - ask the user which one ...
    char sel[5];
    sprintf(output, host_select,i);
    n = write(sessionSocket,output,strlen(output));
    if (n < 0) {
      syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
      close(sessionSocket);
      return;
    }
    bzero(sel,5);
    n = read(sessionSocket,sel,4);
    if (n < 0) 
    {
      syslog (LOG_ERR, "ERROR reading to socket - %s", strerror(errno));
      close(sessionSocket);
      return;
    }    
    int s = atoi(sel);
    if (s<1 || s>i) // Invalid Selection
    {
      n = write(sessionSocket, invalid_select, strlen(invalid_select));
      if (n < 0) {
        syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
        close(sessionSocket);
        return;
      }
      sleep(5);
      return;
    }
    hostport = ports[s];
    strcpy(host, servers[s]);
  }
                                                                      
  // Clean up Maps (and delete contents)
  for(std::map<int, char*>::iterator it = servers.begin(); it != servers.end(); ++it) delete(*it).second;
  servers.clear();
  ports.clear();                                                                                                                                            

  // OK - Ready to connect to host
  n = write(sessionSocket,ok_message,strlen(ok_message));
  if (n < 0) {
    syslog (LOG_ERR, "ERROR writing to socket - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  sleep(1); // Just so folks can read the message ...
  
  // Connect to Host
  int hostSocket;
  struct hostent *pHostInfo;
  long nHostAddress;

  /* make a socket */
  hostSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if(hostSocket == -1)
  {
    syslog (LOG_ERR, "Could not make a socket");
    close(sessionSocket);
    return;
  }
                                    
  /* get IP address from name */
  pHostInfo=gethostbyname(host);

  /* copy address into long */
  memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);
                                                    
  /* fill address struct */
  Address.sin_addr.s_addr=nHostAddress;
  Address.sin_port=htons(hostport);
  Address.sin_family=AF_INET;
                          
  /* connect to host */
  if(connect(hostSocket,(struct sockaddr*)&Address,sizeof(Address))  == -1)
  {
    syslog (LOG_ERR, "Could not connect to host");
    close(sessionSocket);
    return;
  }

  /* Set keepalive option */
  optval = 1;
  if(setsockopt(hostSocket, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
    syslog (LOG_ERR, "ERROR setsockopt() - %s", strerror(errno));
    close(sessionSocket);
    return;
  }
  
  // Forwarding Threads
  pthread_t up_thread;
  pthread_t down_thread;
  
  struct forwardData up_data;
  struct forwardData down_data;
  bool sessionDead = false;

  up_data.fromSocket = sessionSocket;
  up_data.toSocket = hostSocket;
  up_data.dead = &sessionDead;
  down_data.toSocket = sessionSocket;
  down_data.fromSocket = hostSocket;
  down_data.dead = &sessionDead;
  
  if (pthread_create(&(up_thread), NULL, forwardThread, (void *)&up_data))
  {
      syslog (LOG_ERR, "ERROR starting up thread - %s", strerror(errno));
      return;
  }

  if (pthread_create(&(down_thread), NULL, forwardThread, (void *)&down_data))
  {
      syslog (LOG_ERR, "ERROR starting down thread - %s", strerror(errno));
      return;
  }

  // Wait for threads to finish
  if (pthread_join(up_thread,NULL))
  {
     syslog (LOG_ERR, "Failure in pthread_join(up_thread) - %s", strerror(errno));
  }

  if (pthread_join(down_thread,NULL))
  {
     syslog (LOG_ERR, "Failure in pthread_join(down_thread) - %s", strerror(errno));
  }
  
  close(sessionSocket); // Just in Case
  close(hostSocket); // Just in Case

  syslog (LOG_NOTICE, "Session Closed");

  return;
};

void* forwardThread(void* lpvThreadParam)
{
  struct forwardData *data = (struct forwardData*)lpvThreadParam;

  char lpBuffer[256+1]; // Add one for a trailing null if needed
  int nBytesRead;
  bool reading = true;
  int n;
  struct timeval tv;
  fd_set readfds;         
                          
  while(reading && (*(data->dead))!=true)
  {
    FD_ZERO(&readfds);
    FD_SET(data->fromSocket, &readfds);
    tv.tv_sec = 5;
    tv.tv_usec = 0;
            
    // don't care about writefds and exceptfds:
    select(data->fromSocket+1, &readfds, NULL, NULL, &tv);
             
    if (!FD_ISSET(data->fromSocket, &readfds)) continue;
    
    if ((*(data->dead))==true) break;
                                                          

    nBytesRead = read(data->fromSocket, lpBuffer, 256);
    if (nBytesRead == 0) reading = false;
    else if (nBytesRead == -1)
    {
      syslog (LOG_ERR, "Failure in read() - %s", strerror(errno));
      reading = false;
    }
    else
    {
      n = write(data->toSocket,lpBuffer,nBytesRead);
      if (n < 0) 
      {
        syslog (LOG_ERR, "Failure in write() - %s", strerror(errno));
        close(data->fromSocket);
        reading = false;
      }
      if (n != nBytesRead)
      {
        syslog (LOG_ERR, "FIXME - Didn't write everything");
      }
    }
  }
  (*(data->dead)) = true;
  close(data->toSocket);
  close(data->fromSocket);
}

void removeWhiteSpace(char *text)
{
 for (int i=0; text[i]; i++) if (isspace(text[i])) text[i]=0;
}
