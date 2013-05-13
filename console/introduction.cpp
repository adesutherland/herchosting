// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C C O N S O L E
// *************************************************************************
// Work Name   : HercConsole
// Description : This provides a user web console for Hercules Hosting
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : intoduction.cpp
// Description : introduction section
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

#ifndef STANDALONE
#include <herchosting.h> // Common Functionality
#endif

#include <console.h>

#include <Wt/WScrollArea> 

#include <Wt/WLineEdit>

using namespace Wt;

MyIntro::MyIntro()
{
  WString contents = 
#ifdef STANDALONE
      "<h1>Web3270</h1>"
      "<p>This is an AJAX Web 3270 Terminal Emulator which has been configured to connect to certain hosts. Please select the 3270 Terminal link on the left to proceed.</p>"
      "<p>Currently this is in BETA and may be replaced (and possibly removed totally) at any point - for information and to report any defects please e-mail: adrian@open-bpm.org</p>"
      "<p>This is part of a wider project to provide a framework for hosting Hercules S/390  - see below</p>"
      "<h1>DeZhi Mainframe</h1>"
      "<p>This emulator has access to the DeZhi Mainframe. Use the <a href=\"http://www.efglobe.com/cgi-bin/mainframe/mainuser\" target=\"_blank\">DeZhi Mainframe User Support Site</a> to get a TSO userid.</p>"
#endif
      "<h1>Hosted Hercules</h1>"
      "<p>The <a href=\"http://www.open-bpm.org/index.php/hercules.html\" target=\"_blank\">project</a> aims to provide hosted IBM mainframe instances so that members can:</p>"
      "<p>- Log in to Mainframes (via 3270 terminals) for development and training purposes.</p>"
      "<p>- Submit JCL jobs via a custom command line tool (e.g. for development purposed)</p>"
      "<p>Our vision is to provide an easy to use hosted platform that will allow people to easily learn, experiment with and develop software for MVS and VM/370.</p>"
      "<p>Open-BPM can only host freely available Mainframe Operating Systems - like MVS 3.8j and VM/370 - on the Hercules S/390 emulator</p>"
      "<h1>Notes and Known Issues</h1>"
      "<ul>"
      "<li>Primary testing has been done in IE and Windows. Firefox with Windows and Linux has been tested.  Also reported to work Safari/Mac.</li>" 
      "<li>Firefox does not support OVERWRITE mode (i.e. rather than INSERT).</li>"
      "<li>In Firefox hidden fields are visible</li>"
      "</ul>" 
      "<h1>Open-BPM</h1>"
      "<p>The <a href=\"http://www.open-bpm.org/\" target=\"_blank\">Open-BPM Architectural Portal</a> has been setup to facilitate the discussion and development of IT Architecture especially in the area of BPM and Open Source Developments. One area of interest is IBM Mainframes.</p>"
      "<h1>Technology and Credits</h1>"
      "<h2>S/390 Hardware</h2>"
      "<p><a href=\"http://www.hercules-390.org/\" target=\"_blank\">Hercules S/390 emulator</a></p>"
      "<h2>Operating Systems</h2>"
      "<p>MVS 3.8J - For the POC/BETA we are using the <a href=\"http://www.bsp-gmbh.com/turnkey/index.html\" target=\"_blank\">Turnkey</a> distribution</p>"
      "<p><a href=\"http://mvs380.sourceforge.net/\" target=\"_blank\">MVS/380</a> - 31 bit extentions</p>"
      "<p>VM/370</p>"
      "<h2>TN3270</h2>"
      "<p>This is provided by s3270 which is part of the <a href=\"http://x3270.bgp.nu/\" target=\"_blank\">x3270 suite</a></p>"
      "<h2>Development Languages and Libraries</h2>"
      "<p>Platform development language is C++ and REXX (for scripting needs)</p>"
      "<p><a href=\"http://gsoap2.sourceforge.net/\" target=\"_blank\">GSOAP</a> C/C++ Web Service Library</p>"
      "<p><a href=\"http://www.webtoolkit.eu/wt#/\" target=\"_blank\">Wt (Witty)</a> C++ AJAX Library</p>"
      "<p><a href=\"http://www.boost.org/\" target=\"_blank\">BOOST</a> C++ Libraries (for Wt)</p>"
      "<p><a href=\"http://rexxobjects.sourceforge.net/\" target=\"_blank\">REXXOBJECTS</a> Libraries (for REXX)</p>"
      "<p><a href=\"http://regina-rexx.sourceforge.net/\" target=\"_blank\">Regina</a> REXX Interpreter</p>"
      "<h2>Artwork</h2>"
      "<p><a href=\"http://www.famfamfam.com/lab/icons/silk/\" target=\"_blank\">Silk icon set</a> by Mark James</p>"
      "<h1>Version and Copyright Information</h1>"
#ifndef STANDALONE
      "<p>The Hercules Hosting Console:</p>"
#else
      "<p>Web3270:</p>"
#endif
      "<p> - Version   : " VERSION "</p>"
      "<p> - Copyright : " COPYRIGHT "</p>";

      WText *t = new WText(contents, this);
}

MyIntro::~MyIntro()
{

}

