// *************************************************************************
// A B O U T   T H I S   W O R K  -   J C L W E B S E R V I C E
// *************************************************************************
// Work Name   : JCLWebSerice
// Description : This provides a tool to submit JCL Jobs to Hercules
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : mvsclient.cpp
// Description : Client Program
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

#include "soapjcl_wsProxy.h"
#include "jcl_ws.nsmap"
#include "mtom.h"


int main(int args, char* argv[]) 
{ 
  char *s; 
  jcl_ws w;

  soap_init1(w.soap, SOAP_ENC_MTOM); // MTOM  
  
//  w.endpoint = "http://mvs.open-bpm.org/mvsserver.cgi";

  if (args==3)
  {
    w.soap->userid = argv[1]; 
    w.soap->passwd = argv[2];
  } 

  if (w.jcl_ws__Ping(s) == SOAP_OK) 
    std::cout <<  s << std::endl; 
  else
  {
    soap_print_fault(w.soap, stderr); 
    return -1;
  }

  struct jcl_data__Payload jcl;

  openReadFile(w.soap, "input.txt", &jcl);

  writeFileName = "output.txt";
  
  // Alloc return Structure
  struct jcl_ws__jclResponse response;
  response._return = soap_new_jcl_ws__output(w.soap, -1);

  std::string param = "Test";
  
  // Call EchoTest
  w.soap->userid = argv[1]; 
  w.soap->passwd = argv[2];
  w.soap->fmimereadopen = readOpenCallback;
  w.soap->fmimereadclose = readCloseCallback;
  w.soap->fmimeread = readCallback;
  w.soap->fmimewriteopen = writeOpenCallback;
  w.soap->fmimewriteclose = writeCloseCallback;
  w.soap->fmimewrite = writeCallback;

  if ( w.jcl_ws__EchoTest(param, &jcl, response) == SOAP_OK ) 
    std::cout <<  "EchoTest OK" << std::endl; 
  else
  {
    soap_print_fault(w.soap, stderr); 
    return -1;
  }

  soap_destroy(w.soap);
  soap_end(w.soap);
  soap_done(w.soap);
                        
  return 0; 
}
