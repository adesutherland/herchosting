// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C H O S T I N G   C O M M O N
// *************************************************************************
// Work Name   : Herchosting Common
// Description : Common Functionality for HercHosting project
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : authenticate.cpp
// Description : Authenticates User ID and Password
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

#define auth "typoauth"
#define authpath "/usr/local/bin/typoauth"

#include "herchosting.h"

#include <ctype.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
 
using namespace std;

void removeWhiteSpace(char *text)
{
 for (int i=0; text[i]; i++) if (isspace(text[i])) text[i]=0;
}

int authenticate(char* userid, char* password)
{
  // Check Security
  removeWhiteSpace(userid);
  removeWhiteSpace(password);

  int pipe_to_auth[2];
  int pid;
  int rc;
  
  pipe(pipe_to_auth);

  signal(SIGCLD, SIG_DFL);  // I do want to wait for my child
                            // hopefully this does not sc*w up the parent program

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
    return -1;
  }    
 
  rc=WEXITSTATUS(rc);
  if (rc) rc = 1;
  return rc;
}
