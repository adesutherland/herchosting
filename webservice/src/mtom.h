// *************************************************************************
// A B O U T   T H I S   W O R K  -   J C L W E B S E R V I C E
// *************************************************************************
// Work Name   : JCLWebSerice
// Description : This provides a tool to submit JCL Jobs to Hercules
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : mtom.h
// Description : Streaming MTOM utility and callback functions header
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

// Function to open the imput 
int openReadFile(struct soap *soap, const char *file, struct jcl_data__Payload *data);

// Streaming callback for read (called at the beginning of the streaming)
// Return NULL without setting soap->error to not use callback
void *readOpenCallback(struct soap *soap, void *handle, const char *id, const char *type, const char *description);
      
// Streaming callback for read (called for each bit of the file)
size_t readCallback(struct soap *soap, void *handle, char *buf, size_t len);
        
// Streaming callback for read (called at the end)
void readCloseCallback(struct soap *soap, void *handle);

// To store file name
extern char *writeFileName;

// Streaming callback for write (called at the beginning)
void *writeOpenCallback(struct soap *soap, void *unused_handle, const char *id, const char *type, 
                        const char *description, enum soap_mime_encoding encoding);

              
// Streaming callback (called for each bit of the file)
int writeCallback(struct soap *soap, void *handle, const char *buf, size_t len);
                                                        
// Streaming callback for write (called at the end)
void writeCloseCallback(struct soap *soap, void *handle);
