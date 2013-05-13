// *************************************************************************
// A B O U T   T H I S   W O R K  -   J C L W E B S E R V I C E
// *************************************************************************
// Work Name   : JCLWebSerice
// Description : This provides a tool to submit JCL Jobs to Hercules
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : spooljcl.cpp
// Description : Checks the JCL to change the job name and spools JCL
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>

#include <fstream>
#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>





char *spoolJCL(char* input, char* host, int port, char* jobname)
{
  ifstream fin(file);
  string line;
  
  // Read Job Card
  string job;
  string 
  while( getline(fin,s) ) {
  if( s.size() >= 2 && s[0] == '/' && s[1] == '/' ) 
  {
      cout << "  ignoring comment: " << s << endl;
  } else 
  {
   istringstream ss(s);
   double d;
  while( ss >> d ) 
  {
   cout << "  got a number: " << d << endl;
  }






#define version "version 0.4 (c) Adrian Sutherland 2009"
#define tempFileD "printHandlerTemp/printHandler.temp.%d" 

using namespace std;


  // Main loop
  while (!cin.eof())
  {
     int formfeed;
     formfeed = myReadLine(line);
     skipok = false;








  
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
  Address.sin_port=htons(port);
  Address.sin_family=AF_INET;
                          
  /* connect to host */
  if(connect(hostSocket,(struct sockaddr*)&Address,sizeof(Address))  == -1)
  {
    syslog (LOG_ERR, "Could not connect to host");
    close(sessionSocket);
    return;
  }





  char lpBuffer[256+1]; // Add one for a trailing null if needed
  int nBytesRead;
  bool reading = true;
  int n;
  struct timeval tv;
  fd_set readfds;         
                          
  while(reading && (*(data->dead))!=true)
  {





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

}

void removeWhiteSpace(char *text)
{
 for (int i=0; text[i]; i++) if (isspace(text[i])) text[i]=0;
}
