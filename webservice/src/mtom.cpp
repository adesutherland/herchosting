// *************************************************************************
// A B O U T   T H I S   W O R K  -   J C L W E B S E R V I C E
// *************************************************************************
// Work Name   : JCLWebSerice
// Description : This provides a tool to submit JCL Jobs to Hercules
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : mtom.cpp
// Description : Streaming MTOM utility and callback functions
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
#include <sys/stat.h>

#include "mtom.h"

#include <syslog.h>

// Function to open the imput 
int openReadFile(struct soap *soap, const char *file, struct jcl_data__Payload *data)
{ 
//  syslog (LOG_INFO, "openReadFile for file %s", file);
  struct stat sb;
  FILE *fd = NULL;
  int size;

  soap_default_jcl_data__Payload(soap, data);

  fd = fopen(file, "rb");
  if (!fd) 
  {
    syslog (LOG_ERR, "In openReadFile() - could not open file %s", file);
  
    return -1;
  }  
 
  // The callback handle is the fd
  data->xop__Include.__ptr = (unsigned char*)fd;
  if (!fstat(fileno(fd), &sb) && sb.st_size > 0)
  {
    size = sb.st_size;
  }
  else // Try HTTP chunking - problems with Apache?
  {
     soap_set_omode(soap, SOAP_IO_CHUNK);
     size = 0;
  }

  data->xop__Include.__size = size;
  data->xmlmime__contentType = "*/*";
  data->xop__Include.id = NULL;
  data->xop__Include.type = data->xmlmime__contentType;
  data->xop__Include.options = NULL;
  
//  syslog (LOG_INFO, "In openReadFile - file %s size is %d", file, size);
  
  return 0;
}


// Streaming callback for read (called at the beginning of the streaming)
// Return NULL without setting soap->error to not use callback
void *readOpenCallback(struct soap *soap, void *handle, const char *id, const char *type, const char *description)
{ 
//  syslog (LOG_INFO, "readOpenCallback");

  // Nothing to do as we have already opened the file
  FILE *fd = (FILE*)handle;
  return (void*)fd;
}

// Streaming callback for read (called for each bit of the file)                                 
size_t readCallback(struct soap *soap, void *handle, char *buf, size_t len)
{ 
//  syslog (LOG_INFO, "readCallback");

  return fread(buf, 1, len, (FILE*)handle);
}

// Streaming callback for read (called at the end)
void readCloseCallback(struct soap *soap, void *handle)
{ 
//  syslog (LOG_INFO, "readCloseCallback");

  fclose((FILE*)handle);
}

// To store file name
char *writeFileName;

// Streaming callback for write (called at the beginning)
void *writeOpenCallback(struct soap *soap, void *unused_handle, const char *id, 
                        const char *type, const char *description, enum soap_mime_encoding encoding)
{
//  syslog (LOG_INFO, "writeOpenCallback");

  FILE *fd;

  fd = fopen(writeFileName, "wb");
  if (!fd) soap->error = soap_receiver_fault(soap, "Cannot create file", writeFileName);
 
  return (void*)fd;
}

// Streaming callback (called for each bit of the file)
int writeCallback(struct soap *soap, void *handle, const char *buf, size_t len)
{ 
//  syslog (LOG_INFO, "writeCallback");

  FILE *fd = (FILE*)handle;
  while (len)
  { 
    size_t n = fwrite(buf, 1, len, fd);
    if (!n)
    { 
      soap->errnum = errno;
      return SOAP_EOF;
    }
    len -= n; buf += n;
  }
  return SOAP_OK;
}

// Streaming callback for write (called at the end)
void writeCloseCallback(struct soap *soap, void *handle)
{ 
//  syslog (LOG_INFO, "writeCloseCallback");

  fclose((FILE*)handle);
}
