// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C C O N S O L E
// *************************************************************************
// Work Name   : HercConsole
// Description : This provides a user web console for Hercules Hosting
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : terminal.h
// Description : Terminal Header file
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

#ifndef herchosting_terminal_h
#define herchosting_terminal_h

// Internal buffer size for line results from s3270
#define IBS  4096

// Max number of result lines from s3270 - equals max lines on screen
#define RSZ  100

// Max size of text in a 3270 field - 150*3 = 450
#define TBS  450

// Max number of hosts
#define MAXHOSTS 20

#include <Wt/WContainerWidget>
#include <Wt/WSignalMapper>
#include <Wt/WTimer>
#include <Wt/WImage>

#include <list>

#include "console.h"

using namespace Wt;

class Field;
class DownloadFile;
class UploadFile;

class MyTerminal : public BasePage
{
 friend class Field;
 friend class DownloadFile;
 friend class UploadFile;

 public:
  MyTerminal(char* user);
  ~MyTerminal();
  void activate();
  void deactivate();

 private:
  char* userID;

  // Hosts
  int activeHost;
  int s3270Pid[MAXHOSTS]; // s3270 Process
  char host[MAXHOSTS+1][9];
  char server[MAXHOSTS][41];
  int port[MAXHOSTS];
  WComboBox* hostSelect;
  WPushButton* connectButton;
  void ConnectAction();
  void changeHostAction();

  // Socket and FDs to communicate to s3270
  int sockfd[MAXHOSTS];
  FILE *inf[MAXHOSTS];
  FILE *outf[MAXHOSTS];

  // Browser Type - is IE?
  bool isIE;

  /* Connect to a Unix-domain socket. */
  int uSock();

  /* Start and Stop s3270 */
  int startS3270();
  int stopS3270();

  /* Do a single command, and interpret the results. */
  int command3270(char *cmd);

  // Private work buffer for command3270()
  char buf[IBS];

  // Results storage of command3270()
  char *result3270[RSZ];

  // Clears the 3270 results (i.e. free memory)
  void clearResult3270();

  // s3270 Status
  void updateStatusLine();
  char keyboardState;    // If the keyboard is unlocked, the letter U. 
                         // If the keyboard is locked waiting for a response from the host, or if not connected to a host, the letter L. 
                         // If the keyboard is locked because of an operator error (field overflow, protected field, etc.), the letter E. 
  char screenFormatting; // If the screen is formatted, the letter F. If unformatted or in NVT mode, the letter U. 
  char fieldProtection;  // If the field containing the cursor is protected, the letter P. If unprotected or unformatted, the letter U. 
  char connectionState;  // If connected to a host, C Otherwise, the letter N. Note: the string C(hostname) from s3270 is truncated to C 
  char emulatorMode;     // If connected in 3270 mode, the letter I. If connected in NVT line mode, the letter L. 
                         // If connected in NVT character mode, the letter C. If connected in unnegotiated mode (no BIND active from the host), the letter P. 
                         // If not connected, the letter N. 
  int modelNumber;       // 2-5
  int numberOfRows;      // The current number of rows defined on the screen. The host can request that the emulator use a 24x80 screen, so this number
                         // may be smaller than the maximum number of rows possible with the current model. 
  int numberOfColumns;   // The current number of columns defined on the screen, subject to the same difference for rows, above. 
  int cursorRow;         // The current cursor row (zero-origin). 
  int cursorColumn;      // The current cursor column (zero-origin).

  // Screen Buffer Parsing Stuff
  int parsePos;
  int parseLine;
  int screenCol;
  int fieldno;
  char textBuffer[TBS+1];
  int textBufferSize;
  int textBufferLength;
  void doParseLine();
  void doParseSF();
  void doParseSA();
  void completeField();
  void addFieldStart();
  void doParseAttr(bool ea);
  void getLastSF();

  // Screen Stuff
  void renderScreen();
  void setCursorPos();
  void processScreenUpdates();
  JSlot* onFieldFocusSlot;
  std::string setFocusFieldFunc;
  JSlot* keyDownSlot;
  JSlot* keyUpSlot;
  JSlot* cursorSelectSlot;
  JSlot* noopSlot;
  JSignal<std::string>* keySignal;
  void doAidSlot(std::string command);

  WContainerWidget* screen;     // Screen
  WContainerWidget* line[RSZ];  // Line Storage
  WContainerWidget* statusLine; // Status Line
  WLineEdit* lockStateField;
  WLineEdit* keyboardStateField;
  WImage* waitImage;
  WLineEdit* connectionModeField;
  WLineEdit* numberOfRowsField;
  WLineEdit* numberOfColumnsField;
  WLineEdit* cursorColumnField; // Used to communcate the cursor position (or focus field) between browser and server
  WLineEdit* cursorRowField;
  WText* messageLine;           // Message Line
  WContainerWidget* topKeyboard;   // Top Keyboard
  WTimer *redrawTimer;          // RedrawTimer

  // Field Attributes
  bool basic_protected;
  bool basic_numeric;
  bool basic_detectable;
  bool basic_intensified;
  bool basic_nondisplay;
  bool basic_modified;
  bool field_reverse;
  bool field_blink;
  bool field_underline;
  char* field_colour;
  bool basic_ea_intensified;
  bool field_ea_reverse;
  bool field_ea_blink;
  bool field_ea_underline;
  char* field_ea_colour;
  bool basic_ea_intensified_override;
  bool field_ea_reverse_override;
  bool field_ea_blink_override;
  bool field_ea_underline_override;

  // Style Sheets
  char* protFieldStyle;
  char* fieldStyle;
  char* lineStyle;

  // Key Functions
  void doAid(char* command);
  void doTimer();

  // Ugly Hex to Bin (2 digits only)
  int hex2Bin(char* c);

  // Ugly Hex to Bin (1 digit)
  int hex2Bin(char c);

  // Escape spaces to &nbsp;
  char* escSpace(const char* text);

  // Escape " to \";
  char escSpeachMarksBuffer[TBS*2+1];
  char* escSpeachMarks(const char* text);

  // IND$FILE Stuff
  void downloadFile();
  void uploadFile();
};

#endif
