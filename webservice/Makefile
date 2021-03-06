# *************************************************************************
# A B O U T   T H I S   W O R K  -   J C L W E B S E R V I C E
# *************************************************************************
# Work Name   : JCLWebSerice
# Description : This provides a tool to submit JCL Jobs to Hercules
# Copyright   : Copyright (C) 2009 Adrian Sutherland
# *************************************************************************
# A B O U T   T H I S   F I L E
# *************************************************************************
# File Name   : Makefile
# Description : Makefile for Webservice
# *************************************************************************
# L I C E N S E
# *************************************************************************
# This program is free software: you can redistribute it and/or modify
# it under the terms of version 3 of the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http:#www.gnu.org/licenses/>.
#
# For the avoidance of doubt:
# - Version 3 of the license (i.e. not earlier nor later versions) apply.
# - a copy of the license text should be in the "license" directory of the
#   source distribution.
# - Requests for use under other licenses will be treated sympathetically,
#   please see contact details.
# *************************************************************************
# C O N T A C T   D E T A I L S
# *************************************************************************
# E-mail      : adrian@sutherlandonline.org
#             : adrian@open-bpm.org
#             : sutherland@users.sourceforge.net
# Web         : www.open-bpm.org
# Telephone   : Please e-mail for details
# Postal      : UK - Please e-mail for details
# *************************************************************************

INSTALLDIR = /local/www/mvs.open-bpm.org/

INCLUDES = -Igenerated -Igsoap

# Uncomment for Windows
# WIN32 = -DWIN32
# WINLIB = -lwsock32

# ============================================================================

all : bin/mvsclient bin/jcl_webservice.cgi

# ============================================================================

bin/jcl_webservice.cgi : obj/mvsserver.o obj/soapServer.o obj/stdsoap2.o obj/soapC.o obj/mtom.o
	g++ ${WIN32} -o bin/jcl_webservice.cgi obj/soapServer.o obj/mvsserver.o obj/stdsoap2.o obj/soapC.o obj/mtom.o ${WINLIB}

obj/mvsserver.o : src/mvsserver.cpp generated/soapH.h src/mtom.h
	g++ ${WIN32} -o obj/mvsserver.o -c src/mvsserver.cpp ${INCLUDES}

obj/soapServer.o : generated/soapServer.cpp
	g++ ${WIN32} -o obj/soapServer.o -c generated/soapServer.cpp ${INCLUDES}

# ============================================================================

bin/mvsclient : obj/mvsclient.o obj/soapClient.o obj/stdsoap2.o obj/soapC.o obj/mtom.o
	g++ ${WIN32} -o bin/mvsclient obj/soapClient.o obj/mvsclient.o obj/stdsoap2.o obj/soapC.o obj/mtom.o ${WINLIB}

obj/mvsclient.o : src/mvsclient.cpp generated/soapH.h src/mtom.h
	g++ ${WIN32} -o obj/mvsclient.o -c src/mvsclient.cpp ${INCLUDES}

obj/soapClient.o : generated/soapClient.cpp
	g++ ${WIN32} -o obj/soapClient.o -c generated/soapClient.cpp ${INCLUDES}

# ============================================================================

generated/soapH.h : src/mvs.h
	soapcpp2 -L -I gsoap -d generated src/mvs.h

obj/soapC.o : generated/soapC.cpp
	g++ ${WIN32} -o obj/soapC.o -c generated/soapC.cpp ${INCLUDES}

obj/mtom.o : src/mtom.cpp src/mtom.h
	g++ ${WIN32} -o obj/mtom.o -c src/mtom.cpp ${INCLUDES}

# ============================================================================

obj/stdsoap2.o : gsoap/stdsoap2.cpp
	g++ ${WIN32} -o obj/stdsoap2.o -c gsoap/stdsoap2.cpp -Igsoap # -DDEBUG 
	
# ============================================================================

clean:
	rm bin/* obj/* generated/* *~ src/*~

install:
	cp bin/jcl_webservice.cgi ${INSTALLDIR}
	cp generated/*.wsdl ${INSTALLDIR}
	cp generated/*.xsd ${INSTALLDIR}

