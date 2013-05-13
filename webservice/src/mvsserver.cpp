// *************************************************************************
// A B O U T   T H I S   W O R K  -   J C L W E B S E R V I C E
// *************************************************************************
// Work Name   : JCLWebSerice
// Description : This provides a tool to submit JCL Jobs to Hercules
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : mvsserver.cpp
// Description : WebService CPP Implementation
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

#include "soapH.h"
#include "jcl_ws.nsmap"
#include "mtom.h"

#include <syslog.h>

int main() 
{ 
  struct soap soap;
  int ret;

  setlogmask (LOG_UPTO (LOG_INFO));
  openlog ("jcl_webservice.cgi", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
                              
  syslog (LOG_NOTICE, "Program started by User %d", getuid ());
  
  // Set the MTOM callbacks 
  soap_init1(&soap, SOAP_ENC_MTOM); // Enable MTOM XOP attachments
  soap.fmimereadopen = readOpenCallback;
  soap.fmimereadclose = readCloseCallback;
  soap.fmimeread = readCallback;
  soap.fmimewriteopen = writeOpenCallback;
  soap.fmimewriteclose = writeCloseCallback;
  soap.fmimewrite = writeCallback;

  writeFileName = "/tmp/mvsserver.txt"; 
  ret = soap_serve(&soap); 
  
  syslog (LOG_INFO, "WebService Complete");
  closelog ();
  return ret; 
} 

int jcl_ws__Ping(struct soap *soap, char *&response) 
{ 

  syslog (LOG_INFO, "In jcl_ws__Ping()");
  response = "The Open-BPM MVS JCL Tool (c) Adrian Sutherland - Available"; 
  return SOAP_OK; 
}


int jcl_ws__EchoTest(struct soap *soap, std::string test, struct jcl_data__Payload* jcl, struct jcl_ws__jclResponse& response)
{

  syslog (LOG_INFO, "In jcl_ws__EchoTest()");
  // OK we should have already recieved the file
  
  // Avoid SOAP_IO_INSTORE
  if ((soap->omode & SOAP_IO) == SOAP_IO_STORE)
       soap->omode = (soap->omode & ~SOAP_IO) | SOAP_IO_BUFFER;
  
  // Alloc return Structure
  response = *(struct jcl_ws__jclResponse*)soap_malloc(soap, sizeof(struct jcl_ws__jclResponse));
 
  response._return = soap_new_jcl_ws__output(soap, -1);
  response._return->printOutput = (struct jcl_data__Payload*)soap_malloc(soap, sizeof(struct jcl_data__Payload)); 

  // Send back the file
  openReadFile(soap, writeFileName, response._return->printOutput);

//  syslog (LOG_INFO, "Leaving jcl_ws__EchoTest()");  
  return SOAP_OK;
}
