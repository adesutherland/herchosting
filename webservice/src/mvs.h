// *************************************************************************
// A B O U T   T H I S   W O R K  -   J C L W E B S E R V I C E
// *************************************************************************
// Work Name   : JCLWebSerice
// Description : This provides a tool to submit JCL Jobs to Hercules
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : mvs.h
// Description : WebService Header (input for gsoap's soapcpp2)
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

#import "soap12.h"
#import "xop.h"
#import "xmlmime.h"

//gsoap jcl_ws service name:		jcl_ws
//gsoap jcl_ws service namespace:	http://mvs.open-bpm.org/jcl_ws.wsdl
//gsoap jcl_ws service location:	http://mvs.open-bpm.org/jcl_webservice.cgi

//gsoap jcl_data schema namespace:	http://mvs.open-bpm.org/jcl_data.xsd
//gsoap jcl_data schema elementForm:	qualified

//gsoap jcl_data schema type-documentation: Payload structure holding the JCL input or Print output as a MIME attachment
struct jcl_data__Payload
{ _xop__Include xop__Include;
  @char *xmlmime__contentType;
};

class jcl_ws__output
{
  public:
    std::string status;
    struct jcl_data__Payload* printOutput;
};
                  
struct jcl_ws__jclResponse
{
   jcl_ws__output* _return;
};

//gsoap jcl_ws service method-documentation: Checks the connection to and service status of the JCL WebService
jcl_ws__Ping(char*&);

//gsoap jcl_ws service method-documentation: Temporary Service to test streaming 
//gsoap jcl_ws service method-mime-type: EchoTest */*
//gsoap jcl_ws service method-mime-type: EchoTest */*
int jcl_ws__EchoTest(std::string test, struct jcl_data__Payload* jcl, struct jcl_ws__jclResponse& response);
