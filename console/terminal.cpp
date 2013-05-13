// *************************************************************************
// A B O U T   T H I S   W O R K  -   H E R C C O N S O L E
// *************************************************************************
// Work Name   : HercConsole
// Description : This provides a user web console for Hercules Hosting
// Copyright   : Copyright (C) 2009 Adrian Sutherland
// *************************************************************************
// A B O U T   T H I S   F I L E
// *************************************************************************
// File Name   : terminal.cpp
// Description : terminal section
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
// You should have downloadd a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// For the avoidance of doubt:
// - Version 3 of the license (i.e. not earlier nor later versions) apply.
// - a copy of the license text should be in the "license" directory of the
//   source distribution.
// - Requests for use under other licenses will be treated sympathetically,
//   please see contact details.
//
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

#include <Wt/WPushButton>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WTimer>

#include <Wt/WBreak>
#include <Wt/WFont>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WJavaScript>
#include <Wt/WDoubleValidator>
#include <Wt/WDialog>
#include <Wt/WFileResource>
#include <Wt/WAnchor>
#include <Wt/WComboBox>
#include <Wt/WLabel>
#include <Wt/WFileUpload>

#include <iostream> // cout[6~
#include <string>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>

#ifndef STANDALONE
#include <herchosting.h> // Common Functionality
#endif
#include <console.h>
#include <terminal.h>

using namespace Wt;

// Internal Field Class
class Field: public WLineEdit
{
  public:
    Field(MyTerminal* terminal, WContainerWidget *parent, const char* value, int length ,bool prot);
    ~Field();
    int screenRow() {return screenRow_;};
    int screenCol() {return screenCol_;};
    int fieldLength() {return fieldLength_;};
    WString originalValue() {return originalValue_;};
    bool isProtected() {return isProtected_;};
    bool notInUse() {return notInUse_;};
    void setScreenRow(int r) {screenRow_=r;};
    void setScreenCol(int c) {screenCol_=c;};
    void setOriginalValue(const char* s);
    void setNewValue(const char* s, int length);
    void setIsProtected(bool p);
    void setNotInUse(bool n);

    void setNumeric(bool v);
    void setDetectable(bool v);
    void setIntensified(bool v);
    void setNondisplay(bool v);
    void setColour(char* v);
    void setReverse(bool v);
    void setBlink(bool v);
    void setUnderline(bool v);

  private:
    void setStyle();
    int cleanValue(char* s);

    int screenRow_;
    int screenCol_;
    int fieldLength_;
    WString originalValue_;
    int isProtected_;
    bool notInUse_;
    MyTerminal* terminal_;
    bool attrNumeric_;
    bool attrDetectable_;
    bool attrIntensified_;
    bool attrNondisplay_;
    bool attrReverse_;
    bool attrBlink_;
    bool attrUnderline_;
    char* attrColour_;
};

Field::Field(MyTerminal* terminal, WContainerWidget *parent, const char* value, int length, bool prot) : WLineEdit(parent)
{
  terminal_ = terminal;
  fieldLength_ = -1;
  originalValue_ = "";
  isProtected_ = -1;
  notInUse_ = false;
  attrNumeric_ = false;
  attrDetectable_ = false;
  attrIntensified_ = false;
  attrNondisplay_ = false;
  attrReverse_ = false;
  attrBlink_ = false;
  attrUnderline_ = false;
  attrColour_ = 0;

  setScreenRow(0);
  setScreenCol(0);
  setNewValue(value, length);
  setIsProtected(prot);
  focussed().connect( *terminal_->onFieldFocusSlot );
  keyWentDown().connect( *terminal_->keyDownSlot );
  keyWentUp().connect( *terminal_->keyUpSlot );

  setInline(true);
};


Field::~Field()
{
  setNumeric(false);
};


// Remove trailing spaces
// Returns the original length (i.e. with the trailing spaces)
int Field::cleanValue(char* buf)
{
  int len = strlen(buf);

  for (int in=len-1; in>=0; in--)
  {
     if (buf[in]==' ') buf[in] = 0;
     else break;
  }

  return len;
};


void Field::setOriginalValue(const char* s)
{
  char buf[TBS];
  strcpy(buf,s);
  cleanValue(buf);
  originalValue_=buf;
};

void Field::setStyle()
{
  std::string style;

  if (isProtected_) style = terminal_->protFieldStyle;
  else style = terminal_->fieldStyle;

  if (attrReverse_)
  {
    bool donerev = false;
    if (attrDetectable_)
    {
      style = style + " obpm-field-rdetectable";
      donerev = true;
    }
    if (attrIntensified_)
    {
      if (isProtected_) style = style + " obpm-pfield-rintensified";
      else style = style + " obpm-field-rintensified";
      donerev = true;
    }
    if (attrColour_)
    {
      style = (style + " obpm-field-r") + attrColour_;
      donerev = true;
    }
    if (!donerev)
    {
      if (isProtected_) style = style + " obpm-pfield-rev";
      else style = style + " obpm-field-rev";
    }
  }
  else
  {
    if (attrDetectable_) style = style + " obpm-field-detectable";
    if (attrIntensified_) 
    {
      if (isProtected_) style = style + " obpm-pfield-intensified";
      else style = style + " obpm-field-intensified";
    }
    if (attrColour_) style = (style + " obpm-field-") + attrColour_;
  }

  if (attrUnderline_) style = style + " obpm-field-underline";

  if (attrBlink_) style = style + " obpm-field-blink";

  // Only support nondisplay on unprotect fields on IE (work-around as firefox does not show the text cursor!)
  if (attrNondisplay_)
  {
    if (isProtected_ || terminal_->isIE) style = style + " obpm-field-nondisplay";
  }
  
  setStyleClass(style);
}

void Field::setIsProtected(bool p)
{
  int pi;
  if (p) pi = 1;
  else pi = 0;

  if (pi != isProtected_)
  {
    isProtected_ = pi;
    if (isProtected_) 
    {
      setReadOnly(true);
      doubleClicked().connect( *terminal_->cursorSelectSlot );
    }
    else 
    {
      if (notInUse_) setReadOnly(true);
      else setReadOnly(false);
      doubleClicked().connect( *terminal_->noopSlot );
    }
    setStyle();
  }
};


void Field::setNumeric(bool v)
{
  if (attrNumeric_ != v)
  {
    attrNumeric_ = v;
    // FIXME
    //WValidator *val;
    //val = validator();

    //if (attrNumeric_) setValidator(new WDoubleValidator());
    //else setValidator(new WValidator(false));

    //if (val) delete val;
    //setStyle();
  }
};

void Field::setDetectable(bool v)
{
  if (attrDetectable_ != v)
  {
    attrDetectable_ = v;
//    if (attrDetectable_) doubleClicked().connect( *terminal_->cursorSelectSlot );
//    else doubleClicked().connect( *terminal_->noopSlot );
    setStyle();
  }
};

void Field::setIntensified(bool v)
{
  if (attrIntensified_ != v)
  {
    attrIntensified_ = v;
    setStyle();
  }
};

void Field::setColour(char* v)
{
  if (attrColour_ != v)
  {
    attrColour_ = v;
    setStyle();
  }
};


void Field::setReverse(bool v)
{
  if (attrReverse_ != v)
  {
    attrReverse_ = v;
    setStyle();
  }
};


void Field::setBlink(bool v)
{
  if (attrBlink_ != v)
  {
    attrBlink_ = v;
    setStyle();
  }
};


void Field::setUnderline(bool v)
{
  if (attrUnderline_ != v)
  {
    attrUnderline_ = v;
    setStyle();
  }
};


void Field::setNondisplay(bool v)
{
  if (attrNondisplay_ != v)
  {
    attrNondisplay_ = v;
    setStyle();
  }
};

void Field::setNotInUse(bool n)
{
  if (notInUse_ != n)
  {
    notInUse_ = n;
    attrColour_ = 0;
    isProtected_ = -1;
    setIsProtected(true);
    setNewValue("",0);

    if (notInUse_)
    {
      setEnabled(false);
      setHidden(true);
    }
    else
    {
      setEnabled(true);
      setHidden(false);
    }
  }
};

void Field::setNewValue(const char* s, int length)
{
  char buf[TBS];

  strcpy(buf,s);
  cleanValue(buf);
  std::string nv((char*)buf);
  std::string ov = originalValue_.toUTF8();
  if (ov != nv)
  {
    originalValue_ = WString::fromUTF8(nv);
    setText( WString::fromUTF8(nv)  );
  }

  if (length != fieldLength_)
  {
    setMaxLength(length);
    setTextSize(length);
    fieldLength_ = length;
  }
};

MyTerminal::MyTerminal(char* user)
{
  int rc;

  userID = user;

#ifndef STANDALONE
  // FIXME - Temp Define Hosts:
  strcpy(host[0],"MVS380");
  strcpy(server[0],"localhost");
  port[0]=13270;
  strcpy(host[1],"DeZhi");
  strcpy(server[1],"fandezhi.efglobe.com");
  port[1]=23;
  strcpy(host[2],"");
#else
  // Uses /etc/wt/wt_config.xml properties e.g.
  // <property name="hostName_1"  >DeZhi</property>
  // <property name="hostServer_1">fandezhi.efglobe.com</property>
  // <property name="hostPort_1"  >23</property>
  char hn[20], hs[20], hp[20];
  std::string hostNameP, hostServerP, hostPortP;
  std::string hostNameV, hostServerV, hostPortV;
  int i;
  for (i=0; i<MAXHOSTS; i++)
  {
    snprintf(hn,20,"hostName_%d",i+1);
    snprintf(hs,20,"hostServer_%d",i+1);
    snprintf(hp,20,"hostPort_%d",i+1);
    hostNameP = hn;
    hostServerP = hs;
    hostPortP = hp;
    if ( !WApplication::readConfigurationProperty(hostNameP,hostNameV) ) break;
    if ( !WApplication::readConfigurationProperty(hostServerP,hostServerV) ) break;
    if ( !WApplication::readConfigurationProperty(hostPortP,hostPortV) ) break;
    strcpy(host[i],hostNameV.c_str());
    strcpy(server[i],hostServerV.c_str());
    port[i]=atoi( hostPortV.c_str() );
  }
  strcpy(host[i],"");
#endif

  // Zero FDs etc. used to communicate to s3270
  for (int i=0; i<MAXHOSTS; i++)
  {
    sockfd[i] = 0;
    inf[i] = NULL;
    outf[i] = NULL;
    s3270Pid[i] = 0;
  }
  activeHost=0;

  // Init 3270 results and lines
  for (int i=0; i<RSZ; i++)
  {
     result3270[i] = (char*)NULL;
     line[i] = (WContainerWidget*)NULL;
  }

  // Define CSS Styles based on browser - i.e. is it IE?
  char* statusStyle; // Only needed in the constructor
  std::string::size_type loc = WApplication::instance()->environment().userAgent().find("MSIE", 0 );
  if( loc != std::string::npos ) isIE = true;
  else isIE = false;

  if ( isIE ) {
    protFieldStyle = "obpm-pfield-ie";
    fieldStyle = "obpm-field-ie";
    lineStyle = "obpm-line-ie";
    statusStyle = "obpm-status-line-ie";
  } else {
    protFieldStyle = "obpm-pfield";
    fieldStyle = "obpm-field";
    lineStyle = "obpm-line";
    statusStyle = "obpm-status-line";
  }

  // Redraw Timer
  redrawTimer = new WTimer(this);
  redrawTimer->stop();
  redrawTimer->timeout().connect(SLOT(this, MyTerminal::doTimer));

  // Setup display
  setStyleClass("obpm-screencontainer");

  // Host Section / Switch
  WContainerWidget* hsc = new WContainerWidget(this);
  hsc->setStyleClass("obpm-hostselect");
  hsc->setInline(false);

  WLabel* hsl = new WLabel("Host: ", hsc);
  hsl->setStyleClass("obpm-hostselectlabel");

  hostSelect = new WComboBox(hsc);
  for (int i=0; strlen(host[i]); i++) hostSelect->addItem(host[i]);
  hostSelect->setCurrentIndex(0);
  hostSelect->setStyleClass("obpm-hostselectcombo");
  hostSelect->activated().connect(SLOT(this, MyTerminal::changeHostAction));

  connectButton = new WPushButton("Connect", hsc);
  connectButton->setStyleClass("obpm-hostselectbutton");
  connectButton->clicked().connect(SLOT(this, MyTerminal::ConnectAction));

  topKeyboard = new WContainerWidget(this);
  topKeyboard->setStyleClass("obpm-topkeyboard");
  topKeyboard->setInline(false);

  screen = new WContainerWidget(this);
  screen->setStyleClass("obpm-screen");
  screen->setInline(false);

  statusLine = new WContainerWidget(this);
  statusLine->setInline(false);
  statusLine->setStyleClass(statusStyle);

  lockStateField = new WLineEdit(statusLine);
  lockStateField->setStyleClass( ((std::string(fieldStyle)) + (std::string(" obpm-centre"))).c_str() );
  lockStateField->setReadOnly(true);
  lockStateField->setMaxLength(5);
  lockStateField->setTextSize(5);

  waitImage = new WImage("images/wait.png", statusLine);
  waitImage->setHidden(true);

  keyboardStateField = new WLineEdit(statusLine);
  keyboardStateField->setStyleClass( ((std::string(fieldStyle)) + (std::string(" obpm-centre"))).c_str() );
  keyboardStateField->setReadOnly(true);
  keyboardStateField->setMaxLength(20);
  keyboardStateField->setTextSize(20);

  connectionModeField = new WLineEdit(statusLine);
  connectionModeField->setStyleClass( ((std::string(fieldStyle)) + (std::string(" obpm-centre"))).c_str() );
  connectionModeField->setReadOnly(true);
  connectionModeField->setMaxLength(20);
  connectionModeField->setTextSize(20);

  numberOfRowsField = new WLineEdit(statusLine);
  numberOfRowsField->setStyleClass( ((std::string(fieldStyle)) + (std::string(" obpm-right"))).c_str() );
  numberOfRowsField->setReadOnly(true);
  numberOfRowsField->setMaxLength(3);
  numberOfRowsField->setTextSize(3);

  (new WText("x", statusLine))->setStyleClass(fieldStyle);

  numberOfColumnsField = new WLineEdit(statusLine);
  numberOfColumnsField->setStyleClass(fieldStyle);
  numberOfColumnsField->setReadOnly(true);
  numberOfColumnsField->setMaxLength(10);
  numberOfColumnsField->setTextSize(10);

  cursorRowField = new WLineEdit(statusLine);
  cursorRowField->setStyleClass( ((std::string(fieldStyle)) + (std::string(" obpm-right"))).c_str() );
  cursorRowField->setReadOnly(true);
  cursorRowField->setMaxLength(3);
  cursorRowField->setTextSize(3);

  (new WText("/", statusLine))->setStyleClass(fieldStyle);

  cursorColumnField = new WLineEdit(statusLine);
  cursorColumnField->setStyleClass(fieldStyle);
  cursorColumnField->setReadOnly(true);
  cursorColumnField->setMaxLength(10);
  cursorColumnField->setTextSize(10);

  new WBreak(this);

  messageLine = new WText(this);
  messageLine->setInline(true);

  // Key Press (AID) signal
  keySignal = new JSignal<std::string>(this, "keyAid");
  keySignal->connect(SLOT(this, MyTerminal::doAidSlot));

  // Setup the keyboards
  WPushButton *b;

  b = new WPushButton("SysReq", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"SysReq\");}",this)));
  b->setStyleClass("obpm-wide-key");

  b = new WPushButton("Attn", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"Attn\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("PF1", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(1)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF2", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(2)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF3", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(3)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF4", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(4)\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("PF5", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(5)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF6", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(6)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF7", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(7)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF8", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(8)\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("PF9", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(9)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF10", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(10)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF11", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(11)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF12", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(12)\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("PA1", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PA(1)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PA2", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PA(2)\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("Upload", topKeyboard);
  b->clicked().connect(SLOT(this, MyTerminal::uploadFile));
  b->setStyleClass("obpm-wide-key");

  new WBreak(topKeyboard);

  b = new WPushButton("Reset", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"Reset\");}",this)));
  b->setStyleClass("obpm-wide-key");

  b = new WPushButton("Clear", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"Clear\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("PF13", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(13)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF14", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(14)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF15", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(15)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF16", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(16)\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("PF17", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(17)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF18", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(18)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF19", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(19)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF20", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(20)\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("PF21", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(21)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF22", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(22)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF23", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(23)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("PF24", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PF(24)\");}",this)));
  b->setStyleClass("obpm-key-gap");
/*
  b = new WPushButton("Redraw Scrn", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"\");}",this)));
  b->setStyleClass("obpm-wide-key");
*/
  b = new WPushButton("PA3", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"PA(3)\");}",this)));
  b->setStyleClass("obpm-key");

  b = new WPushButton("Enter", topKeyboard);
  b->clicked().connect(*(new JSlot("function(sender,event){"+WApplication::instance()->javaScriptClass()+".sendAID(\"Enter\");}",this)));
  b->setStyleClass("obpm-key-gap");

  b = new WPushButton("Download", topKeyboard);
  b->clicked().connect(SLOT(this, MyTerminal::downloadFile));
  b->setStyleClass("obpm-wide-key");

  // Set Cursor position when a field is focussed
  std::string onFieldFocusFunc =
      "function(sender, event)"
      "{"
        "var c=0; var r=0;"
        "var p = sender.previousSibling;"
        "while (p)"
        "{"
          "if (p.nodeName==\"INPUT\")"
          "{"
            "c+=parseInt(p.getAttribute(\"maxLength\"));"
          "}"
          "p = p.previousSibling;"
        "};"
        "var l = sender.parentNode.previousSibling;"
        "while (l)"
        "{"
          "r++;"
          "l = l.previousSibling;"
        "};"
        + cursorColumnField->jsRef() + ".value = c+1;"
        + cursorRowField->jsRef() + ".value = r+1;"

        "sender.onhelp = function() { return false; };" // Disable windows IE help (F1)
      "}";

  onFieldFocusSlot = new JSlot(onFieldFocusFunc, this);


  // Set Cursor Select Function - Double Click
  std::string cursorSelectFunc =
      "function(sender, event)"
      "{"
        "if (!" + WApplication::instance()->javaScriptClass()+".isLocked())"
        "{"
          "var c=0; var r=0;"
          "var p = sender.previousSibling;"
          "while (p)"
          "{"
            "if (p.nodeName==\"INPUT\")"
            "{"
              "c+=parseInt(p.getAttribute(\"maxLength\"));"
            "}"
            "p = p.previousSibling;"
          "};"
          "var l = sender.parentNode.previousSibling;"
          "while (l)"
          "{"
            "r++;"
            "l = l.previousSibling;"
          "};"
          + cursorColumnField->jsRef() + ".value = c+1;"
          + cursorRowField->jsRef() + ".value = r+1;"
          + WApplication::instance()->javaScriptClass()+".sendAID(\"ENTER\");"
        "}"
      "}";

  cursorSelectSlot = new JSlot(cursorSelectFunc, this);

  // Set NOOP (No Operation) Function
  std::string noopFunc =
      "function(sender, event)"
      "{;"
      "}";
  noopSlot = new JSlot(noopFunc, this);

  // Focus the field with the cursor
  setFocusFieldFunc =
      "{"
        "var c=parseInt(" + cursorColumnField->jsRef() + ".value)-1;"
        "var r=parseInt(" + cursorRowField->jsRef() + ".value)-1;"
        + WApplication::instance()->javaScriptClass() + ".setCursorFocus(r,c);"
        + WApplication::instance()->javaScriptClass() + ".setOverwrite();"
        + topKeyboard->jsRef()+".disabled=false;"
      "}";

  // Handle Function keys, tab and cursor up/down 
  std::string keyDownFunc =
      "function(sender, event)"
      "{"
        "var kc=event.keyCode;"
        "var k=\"\";"
        "var suppress=false;"

        "if (" + WApplication::instance()->javaScriptClass()+".isLocked()) suppress=true;"

        "else if (kc==38) k=\"UP\";"
        "else if (kc==40) k=\"DOWN\";"
        "else if (kc==37) k=\"LEFT\";"
        "else if (kc==39) k=\"RIGHT\";"
        "else if (kc==9)  k=\"TAB\";"
        "else if (kc==13)  k=\"ENTER\";"
        "else if (kc==112) k=(event.shiftKey?\"PF(13)\":\"PF(1)\");"
        "else if (kc==113) k=(event.shiftKey?\"PF(14)\":\"PF(2)\");"
        "else if (kc==114) k=(event.shiftKey?\"PF(15)\":\"PF(3)\");"
        "else if (kc==115) k=(event.shiftKey?\"PF(16)\":\"PF(4)\");"
        "else if (kc==116) k=(event.shiftKey?\"PF(17)\":\"PF(5)\");"
        "else if (kc==117) k=(event.shiftKey?\"PF(18)\":\"PF(6)\");"
        "else if (kc==118) k=(event.shiftKey?\"PF(19)\":\"PF(7)\");"
        "else if (kc==119) k=(event.shiftKey?\"PF(20)\":\"PF(8)\");"
        "else if (kc==120) k=(event.shiftKey?\"PF(21)\":\"PF(9)\");"
        "else if (kc==121) k=(event.shiftKey?\"PF(22)\":\"PF(10)\");"
        "else if (kc==122) k=(event.shiftKey?\"PF(23)\":\"PF(11)\");"
        "else if (kc==123) k=(event.shiftKey?\"PF(24)\":\"PF(12)\");"

        "if (k!=\"\")"
        "{"
          // Function Keys
          "if (k.charAt(0)=='P')"
          "{"
             "suppress=true;"
             + WApplication::instance()->javaScriptClass()+".sendAID(k);"
          "}"

          // ENTER Key
          "else if (k=='ENTER')"
          "{"
             "suppress=true;"
             + WApplication::instance()->javaScriptClass()+".sendAID(\"Enter\");"
          "}"

          // Tab Key
          "else if (k==\"TAB\" && !event.shiftKey)"
          "{"
            "suppress=true;"
            "var done=false;"
            // Next Tab field on this line?
            "var p = sender.nextSibling;"
            "while (p)"
            "{"
              "if (p.nodeName==\"INPUT\" && !p.readOnly)"
              "{"
                "done=true;"
                "break;"
              "}"
              "p=p.nextSibling;"
            "};"
            // Try the other lines
            "if (!done)"
            "{"
              "var l = sender.parentNode.nextSibling;"
              "while (!done)"
              "{"
                "if (!l) l=" + screen->jsRef() + ".firstChild;"
                "p = l.firstChild;"
                "while (p && !done)"
                "{"
                  "if (p==sender || (p.nodeName==\"INPUT\" && !p.readOnly) )"
                  "{"
                    "done=true;"
                    "break;"
                  "}"
                  "p = p.nextSibling;"
                "}"
                "l = l.nextSibling;"
              "};"
            "};"
            // Always put the cursor at the beginning
            + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,0);"
//            "if (p.value.length==p.maxLength) " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,0);"
//            "else " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,p.value.length);"
          "}"

          // Shift Tab Key
          "else if (k==\"TAB\" && event.shiftKey)"
          "{"
            "suppress=true;"
            "var done=false;"
            // Next Tab field on this line?
            "var p = sender.previousSibling;"
            "while (p)"
            "{"
              "if (p.nodeName==\"INPUT\" && !p.readOnly)"
              "{"
                "done=true;"
                "break;"
              "}"
              "p=p.previousSibling;"
            "};"
            // Try the other lines
            "if (!done)"
            "{"
              "var l = sender.parentNode.previousSibling;"
              "while (!done)"
              "{"
                "if (!l) l=" + screen->jsRef() + ".lastChild;"
                "p = l.lastChild;"
                "while (p && !done)"
                "{"
                  "if (p==sender || (p.nodeName==\"INPUT\" && !p.readOnly) )"
                  "{"
                    "done=true;"
                    "break;"
                  "}"
                  "p = p.previousSibling;"
                "}"
                "l = l.previousSibling;"
              "};"
            "};"
            // Always put the cursor at the beginning
            + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,0);"
            // "if (p.value.length==p.maxLength) " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,0);"
            // "else " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,p.value.length);"
          "}"

          // Down Key
          "else if (k==\"DOWN\")"
          "{"
            "suppress=true;"
            "var c=parseInt(" + cursorColumnField->jsRef() + ".value)-1+" + WApplication::instance()->javaScriptClass() + ".getCursorPos(sender);"
            "var done=false;"
            "var l = sender.parentNode.nextSibling;"
            "while (l && !done)"
            "{"
              "var p = l.firstChild;"
              "var x=0;"
              "while (p)"
              "{"
                "if (p.nodeName==\"INPUT\")"
                "{"
                  "var len=parseInt(p.getAttribute(\"maxLength\"));"
                  "if (c>=x && c<x+len)"
                  "{"
                    "if (!p.readOnly)"
                    "{"
                      + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,c-x);"
                      "done=true;"
                    "}"
                    "break;"
                  "}"
                  "x+=len;"
                "}"
                "p = p.nextSibling;"
              "};"
              "l = l.nextSibling;"
            "};"
          "}"

          // Up Key
          "else if (k==\"UP\")"
          "{"
            "suppress=true;"
            "var c=parseInt(" + cursorColumnField->jsRef() + ".value)-1+" + WApplication::instance()->javaScriptClass() + ".getCursorPos(sender);"
            "var done=false;"
            "var l = sender.parentNode.previousSibling;"
            "while (l && !done)"
            "{"
              "var p = l.firstChild;"
              "var x=0;"
              "while (p)"
              "{"
                "if (p.nodeName==\"INPUT\")"
                "{"
                  "var len=parseInt(p.getAttribute(\"maxLength\"));"
                  "if (c>=x && c<x+len)"
                  "{"
                    "if (!p.readOnly)"
                    "{"
                      + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,c-x);"
                      "done=true;"
                    "}"
                    "break;"
                  "}"
                  "x+=len;"
                "}"
                "p = p.nextSibling;"
              "};"
              "l = l.previousSibling;"
            "};"
          "}"

          // Right Key
          "else if (k==\"RIGHT\")"
          "{"
            "if (" + WApplication::instance()->javaScriptClass() + ".getCursorPos(sender)==sender.value.length)"
            "{"
              "suppress=true;"
              "var done=false;"
              // Next Tab field on this line?
              "var p = sender.nextSibling;"
              "while (p)"
              "{"
                "if (p.nodeName==\"INPUT\" && !p.readOnly)"
                "{"
                  "done=true;"
                  "break;"
                "}"
                "p=p.nextSibling;"
              "};"
              // Try the other lines
              "if (!done)"
              "{"
                "var l = sender.parentNode.nextSibling;"
                "while (!done)"
                "{"
                  "if (!l) l=" + screen->jsRef() + ".firstChild;"
                  "p = l.firstChild;"
                  "while (p && !done)"
                  "{"
                    "if (p==sender || (p.nodeName==\"INPUT\" && !p.readOnly) )"
                    "{"
                      "done=true;"
                      "break;"
                    "}"
                    "p = p.nextSibling;"
                  "}"
                  "l = l.nextSibling;"
                "};"
              "};"
              + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,0);"
            "}"
          "}"

          // Left Key
          "else if (k==\"LEFT\")"
          "{"
            "if (" + WApplication::instance()->javaScriptClass() + ".getCursorPos(sender)==0)"
            "{"
              "suppress=true;"
              "var done=false;"
              // Prevoius Tab field on this line?
              "var p = sender.previousSibling;"
              "while (p)"
              "{"
                "if (p.nodeName==\"INPUT\" && !p.readOnly)"
                "{"
                  "done=true;"
                  "break;"
                "}"
                "p=p.previousSibling;"
              "};"
              // Try the other lines
              "if (!done)"
              "{"
                "var l = sender.parentNode.previousSibling;"
                "while (!done)"
                "{"
                  "if (!l) l=" + screen->jsRef() + ".lastChild;"
                  "p = l.lastChild;"
                  "while (p && !done)"
                  "{"
                    "if (p==sender || (p.nodeName==\"INPUT\" && !p.readOnly) )"
                    "{"
                      "done=true;"
                      "break;"
                    "}"
                    "p = p.previousSibling;"
                  "}"
                  "l = l.previousSibling;"
                "};"
              "};"
              "if (p.value.length==p.maxLength) " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,p.value.length-1);"
              "else " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,p.value.length);"
            "}"
          "}"

          "else"
          "{"
"alert(\"Key=\" + k);"
          "}"
        "}"

        "if (suppress)"
        "{"
          // We want this key event to do absolutely nothing else
          "event.cancelBubble=true;"
          "if (event.stopPropagation) event.stopPropagation();"
          "if (event.preventDefault)  event.preventDefault();"
          "try { event.keyCode=0; } catch(e) {;};"
          "event.returnValue=false;"
          WT_CLASS ".cancelEvent(event);"
        "}"
      "}";

  keyDownSlot = new JSlot(keyDownFunc, this);

  std::string keyUpFunc =
      "function(sender, event)"
      "{"
        "if (!" + WApplication::instance()->javaScriptClass()+".isLocked() && "
              + WApplication::instance()->javaScriptClass() + ".getCursorPos(sender)==sender.maxLength)"
        "{"
          "var done=false;"
          // Next Tab field on this line?
          "var p = sender.nextSibling;"
          "while (p)"
          "{"
            "if (p.nodeName==\"INPUT\" && !p.readOnly)"
            "{"
              "done=true;"
              "break;"
            "}"
            "p=p.nextSibling;"
          "};"
          // Try the other lines
          "if (!done)"
          "{"
            "var l = sender.parentNode.nextSibling;"
            "while (!done)"
            "{"
              "if (!l) l=" + screen->jsRef() + ".firstChild;"
              "p = l.firstChild;"
              "while (p && !done)"
              "{"
                "if (p==sender || (p.nodeName==\"INPUT\" && !p.readOnly) )"
                "{"
                  "done=true;"
                  "break;"
                "}"
                "p = p.nextSibling;"
              "}"
              "l = l.nextSibling;"
            "};"
          "};"
          + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,0);"
        "}"
      "}";

  keyUpSlot = new JSlot(keyUpFunc, this);

  // Declare Global Functions
  if (!((HercConsole*)WApplication::instance())->loadedTerminalJS)
  {
    ((HercConsole*)WApplication::instance())->loadedTerminalJS=true;

    //  getCursorPos
    std::string getCursorPos;
    if (isIE) getCursorPos =
     "function getCursorPos(inputBox)"
     "{"
       "var bookmark = document.selection.createRange().getBookmark();"
       "var selection = inputBox.createTextRange();"
       "selection.moveToBookmark(bookmark);"
       "var before = inputBox.createTextRange();"
       "before.collapse(true);"
       "before.setEndPoint(\"EndToStart\", selection);"
       "return beforeLength = before.text.length;"
     "}";
    else getCursorPos =
     "function getCursorPos(inputBox)"
     "{"
       "if (inputBox.selectionStart)" 
       "{"
         "var r = inputBox.selectionStart;"
         "if (!r) r=0;"
         "return r;"
       "}"
       "else return 0;"
     "}";
    WApplication::instance()->declareJavaScriptFunction("getCursorPos", getCursorPos);

    // setCursorPos
    std::string setCursorPos;
    if (isIE) setCursorPos =
     "function setCursorPos(inputBox, pos)"
     "{"
       "if (pos==-1) inputBox.select();"
       "else"
       "{" 
         "var range = inputBox.createTextRange();"
         "range.move(\"character\", pos);"
         "range.select();"
       "}"
     "}";
    else setCursorPos =
     "function setCursorPos(inputBox, pos)"
     "{"
       "inputBox.focus();"
       "if (pos==-1) inputBox.select();"
       "else inputBox.setSelectionRange(pos, pos);"
     "}";
    WApplication::instance()->declareJavaScriptFunction("setCursorPos", setCursorPos);

    std::string setCursorFocus =
     "function setCursorFocus(r, c)"
     "{"
       "var l=" + screen->jsRef() + ".firstChild;"
       "for (i=0;i<r;i++) l=l.nextSibling;"  // Get to the right line
       "var p = l.firstChild;"
       "var x=0;"
       "while (p)"
       "{"
         "if (p.nodeName==\"INPUT\")"
         "{"
           "var len=parseInt(p.getAttribute(\"maxLength\"));"
           "if (c>=x && c<x+len)"
           "{"
             // Replace this functionality - can't remember why I wanted it to  be in CursorPos 0?
             //"if (p.value.length==p.maxLength) " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,0);"
             //"else if (c-x>p.value.length) " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,p.value.length);"
             //"else " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,c-x);"
             "if (c-x>p.value.length) " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,p.value.length);"
             "else " + WApplication::instance()->javaScriptClass() + ".setCursorPos(p,c-x);"
             "break;"
           "}"
           "x+=len;"
         "}"
         "p = p.nextSibling;"
       "};"
     "}";
    WApplication::instance()->declareJavaScriptFunction("setCursorFocus", setCursorFocus);

    std::string setOverwrite;
    if (isIE) setOverwrite =
     "function setOverwrite()"
     "{"
       "document.execCommand('OverWrite', false, true);" // IE function to turn on overwrite
     "}";
    else setOverwrite =
     "function setOverwrite()"
     "{"
       ";"  // Firefox does not support overwrite at all!
     "}";
    WApplication::instance()->declareJavaScriptFunction("setOverwrite", setOverwrite);

    std::string sendAID =
     "function sendAID(aid)"
     "{"
       + lockStateField->jsRef() + ".value=\"X\";"
       + WApplication::instance()->javaScriptClass() + ".setLock(true,\"\");"
       + keySignal->createCall("aid") + ";"
     "}";
    WApplication::instance()->declareJavaScriptFunction("sendAID", sendAID);

    std::string setLock;
    if (isIE) setLock =
     "function setLock(lock, message)"
     "{"
        + topKeyboard->jsRef()+".disabled=lock;"
     "}";
    else setLock =
     "function setLock(lock, message)"
     "{"
        "var p="+ topKeyboard->jsRef()+".firstChild;"
        "while (p)"
        "{"
          "p.disabled=lock;"
          "p=p.nextSibling;"
        "}"
     "}";

    WApplication::instance()->declareJavaScriptFunction("setLock", setLock);

    std::string isLocked;
    if (isIE) isLocked =
     "function isLocked()"
     "{"
        "return " + topKeyboard->jsRef()+".disabled;"
     "}";
    else isLocked =
     "function isLocked()"
     "{"
        "return " + topKeyboard->jsRef()+".firstChild.disabled;"
     "}";
    WApplication::instance()->declareJavaScriptFunction("isLocked", isLocked);
  }

  topKeyboard->hide();
  screen->hide();
  statusLine->hide();
}

MyTerminal::~MyTerminal()
{
  // Kill the s3270 process
  int rc;

  for (int i=0; i<MAXHOSTS; i++)
  {
    if (s3270Pid[i])
    {
      activeHost=i;
      stopS3270();
    }
  }

  for (int i=0; i<MAXHOSTS; i++)
  {
    if (sockfd[i])
    {
      fclose(inf[i]);
      fclose(outf[i]);
      close(sockfd[i]);
    }
  }

  clearResult3270();
  delete onFieldFocusSlot;
  delete keyDownSlot;
  delete keySignal;
}

void MyTerminal::activate()
{
  if (s3270Pid[activeHost]) 
  {
    topKeyboard->show();
    screen->show();
    statusLine->show();
    connectButton->setText("Disconnect");
    renderScreen();
    setCursorPos();
  }
  else
  {
    topKeyboard->hide();
    screen->hide();
    statusLine->hide();
    connectButton->setText("Connect");
    redrawTimer->stop();
  }
}

void MyTerminal::deactivate()
{
  redrawTimer->stop();
}

void MyTerminal::clearResult3270()
{
  for (int i=0; i<RSZ; i++) 
  {
   if (result3270[i])
   {
     free( result3270[i] );
     result3270[i] = (char*)NULL;
   }
   else break;
  }
}

/* Connect to a Unix-domain socket. */
int MyTerminal::uSock()
{
  struct sockaddr_un ssun;
  int fd;

  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    syslog (LOG_ERR, "Could not create socket - %s", strerror(errno));
    return 0;
  }

  (void) memset(&ssun, '\0', sizeof(struct sockaddr_un));
  ssun.sun_family = AF_UNIX;
  (void) sprintf(ssun.sun_path, "/tmp/x3sck.%d", s3270Pid[activeHost]);
  for (int loop=0; loop<30*5; loop++)
  {
    if (connect(fd, (struct sockaddr *)&ssun, sizeof(ssun)) < 0) 
    {
      if (errno != ENOENT) // ENOENT is File not found - i.e. s3270 not ready yet
      {
        // If not ENOENT then give up
        syslog (LOG_ERR, "Could not connect() socket (%s) - %s", ssun.sun_path, strerror(errno));
        close (fd);
        return 0;
      }
    }
    else return fd; // Connected OK

    // Sleep a bit
    timespec tmReq;
    tmReq.tv_sec = 0;
    tmReq.tv_nsec = 200 * 1000000; // 200ms

    // we're not interested in remaining time nor in return value
    (void)nanosleep(&tmReq, (timespec *)NULL);
  }

  // If we get here s3270 has not started after 30 seconds - so give up
  syslog (LOG_ERR, "Could not connect() socket (%s) after 30 seconds - %s", ssun.sun_path, strerror(errno));
  close (fd);
  return 0;
}

/* Do a single command, and interpret the results            */
/* Return Codes: 0=OK, -1=Command Error, -2=S3270 seems dead, -3 Internal error */
int MyTerminal::command3270(char *cmd)
{
  int xs = -1;

  clearResult3270();

  if (!s3270Pid[activeHost]) return -2; // No s3270 defined?

  // Ignore broken pipes - FIXME Do we need this?
  // (void) signal(SIGPIPE, SIG_IGN);

  /* Open files if need be */
  if (!sockfd[activeHost])
  {
    sockfd[activeHost] = uSock();
    if (!sockfd[activeHost]) return -2; // s3270 not alive?

    inf[activeHost] = fdopen(sockfd[activeHost], "r");
    outf[activeHost] = fdopen(dup(sockfd[activeHost]), "w");

    if (inf[activeHost] == (FILE *)NULL) {
      syslog (LOG_ERR, "Could not create input fd for s3270 - %s", strerror(errno));
      close(sockfd[activeHost]);
      sockfd[activeHost] = 0; inf[activeHost] = NULL; outf[activeHost] = NULL;
      return -3;
    }

    if (outf[activeHost] == (FILE *)NULL) {
      syslog (LOG_ERR, "Could not create output fd for s3270 - %s", strerror(errno));
      fclose(inf[activeHost]);
      close(sockfd[activeHost]);
      sockfd[activeHost] = 0; inf[activeHost] = NULL; outf[activeHost] = NULL;
      return -3;
    }
  }

  /* Speak to s3270. */
  if (fprintf(outf[activeHost], "%s\n", (cmd != NULL)? cmd: "") < 0 || fflush(outf[activeHost]) < 0) {
    syslog (LOG_ERR, "Could not send command to s3270 - %s", strerror(errno));
    fclose(inf[activeHost]);
    fclose(outf[activeHost]);
    close(sockfd[activeHost]);
    sockfd[activeHost] = 0; inf[activeHost] = NULL; outf[activeHost] = NULL;
    return -2; // Asume this means that s3270 has died
  }

  /* Get the answer. */
  int dataContinuation = 0;
  int r = 0;
  while (fgets(buf, IBS, inf[activeHost]) != (char *)NULL)
  {
    int nlFound = 0;
    int sl = strlen(buf);

    if (sl > 0 && buf[sl-1] == '\n') 
    {
      buf[--sl] = '\0';
      nlFound = 1;
    }

    if (dataContinuation)
    {
      result3270[r] = (char*)realloc(result3270[r], strlen(result3270[r]) + sl + 1);
      strcat(result3270[r], buf);

      if (nlFound) 
      {
        dataContinuation = 0;
        r++;
      }
      else dataContinuation = 1;
    }

    else if (!strcmp(buf, "ok"))
    {
      xs = 0;
      break;
    }

    else if (!strcmp(buf, "error"))
    {
      if ( strcmp(cmd,"Wait(1,Unlock)")) // Suppress Wait(1,Unlock) log messages - not needed or wanted
      {
        syslog (LOG_ERR, "s3270 returned an error in response to command: %s (any details follow)", cmd);
        for (int i=0; i<RSZ; i++) 
        {
          if (result3270[i]) syslog(LOG_ERR, "s3270 Error Details: %s", result3270[i]);
          else break;
        }
      }
      xs = 1;
      break;
    }

    else if (!strncmp(buf, "data: ", 6))
    {
      result3270[r] = (char*)malloc(sl + 1 - 6);
      strcpy(result3270[r], buf + 6);

      if (nlFound) 
      {
        dataContinuation = 0;
        r++;
      }
      else dataContinuation = 1;
    }

    else if (sl) // Process Status
    {
      char *sf = (char *)NULL;
      char *tok = " \t";
      char *saveptr;

      sf = strtok_r(buf, tok, &saveptr);
      keyboardState = ((sf != (char *)NULL) ? sf[0] : ' ');

      sf = strtok_r(NULL, tok, &saveptr);
      screenFormatting = ((sf != (char *)NULL) ? sf[0] : ' '); 

      sf = strtok_r(NULL, tok, &saveptr);
      fieldProtection = ((sf != (char *)NULL) ? sf[0] : ' '); 

      sf = strtok_r(NULL, tok, &saveptr);
      connectionState = ((sf != (char *)NULL) ? sf[0] : ' '); 

      sf = strtok_r(NULL, tok, &saveptr);
      emulatorMode = ((sf != (char *)NULL) ? sf[0] : ' '); 

      sf = strtok_r(NULL, tok, &saveptr);
      modelNumber = ((sf != (char *)NULL) ? atoi(sf) : 0);

      sf = strtok_r(NULL, tok, &saveptr);
      numberOfRows = ((sf != (char *)NULL) ? atoi(sf) : 0);

      sf = strtok_r(NULL, tok, &saveptr);
      numberOfColumns = ((sf != (char *)NULL) ? atoi(sf) : 0); 

      sf = strtok_r(NULL, tok, &saveptr);
      cursorRow = ((sf != (char *)NULL) ? atoi(sf) : 0); 

      sf = strtok_r(NULL, tok, &saveptr);
      cursorColumn = ((sf != (char *)NULL) ? atoi(sf) : 0);
    }
  }

  /* If fgets() failed, log error */
  if (xs == -1) {
    if (feof(inf[activeHost]))
    {
      if (!strcasecmp(cmd,"quit")) xs=0; // EOF ok with a quit command
      else syslog (LOG_ERR, "Unexpected EOF when reading from s3270");
    }
    else syslog (LOG_ERR, "Error reading from s3270 - %s", strerror(errno));
  }

  if (xs==-1) 
  {
    fclose(inf[activeHost]);
    fclose(outf[activeHost]);
    close(sockfd[activeHost]);
    sockfd[activeHost] = 0; inf[activeHost] = NULL; outf[activeHost] = NULL;
    return -2; // Assume that s3270 died
  }
  else if (xs) return -1;
  else return 0;
}

// Ugly Hex to Bin (2 digits only)
int MyTerminal::hex2Bin(char* c)
{
  return hex2Bin(c[0])*16 + hex2Bin(c[1]); 
}

// Ugly Hack - Hex to Bin (1 digit)
int MyTerminal::hex2Bin(char c)
{
  int r=0;
  switch(c)
  {
    case '0':
     r=0;
     break;
    case '1':
     r=1;
     break;
    case '2':
     r=2;
     break;
    case '3':
     r=3;
     break;
    case '4':
     r=4;
     break;
    case '5':
     r=5;
     break;
    case '6':
     r=6;
     break;
    case '7':
     r=7;
     break;
    case '8':
     r=8;
     break;
    case '9':
     r=9;
     break;
    case 'a':
     r=10;
     break;
    case 'b':
     r=11;
     break;
    case 'c':
     r=12;
     break;
    case 'd':
     r=13;
     break;
    case 'e':
     r=14;
     break;
    case 'f':
     r=15;
     break;
    case 'A':
     r=10;
     break;
    case 'B':
     r=11;
     break;
    case 'C':
     r=12;
     break;
    case 'D':
     r=13;
     break;
    case 'E':
     r=14;
     break;
    case 'F':
     r=15;
     break;
    default: ;
  }
  return r;
}

void MyTerminal::doParseLine()
{
  parsePos=0;
  screenCol=0;
  textBufferSize=0;
  textBufferLength=0;
  fieldno = 0;

  for (;(result3270[parseLine])[parsePos]==' ';parsePos++); // Remove Leading Spaces

  do
  {
    if ((result3270[parseLine])[parsePos]=='S')
    { 
      parsePos++;
      if ((result3270[parseLine])[parsePos]=='F') doParseSF();
      else if ((result3270[parseLine])[parsePos]=='A') doParseSA();
      else for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!=' ';parsePos++); // Skip this unknown stuff 'till we get a space
    }
    else if ((result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos+1]) // Just hex character(s)
    {
      while ((result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos+1])
      {
        char c = hex2Bin((result3270[parseLine]) + parsePos);
        parsePos += 2;
        if (!c) c=' ';
        if (textBufferSize<TBS) textBuffer[ textBufferSize++ ] = c;
        if ((result3270[parseLine])[parsePos] == ' ') break;
      }
      textBufferLength++;
    }

    for (;(result3270[parseLine])[parsePos]==' ';parsePos++); // Remove Training Spaces
  }
  while ((result3270[parseLine])[parsePos]); // Until we reach the end of the line

  completeField();

  // Hide any unused fields
  for (int j=fieldno; j<line[parseLine]->count(); j++) 
  {
    Field* field = dynamic_cast<Field*>( line[parseLine]->widget(j) );
    if (field) field->setNotInUse(true);
  }
}

void MyTerminal::doParseSF()
{
  if (textBufferSize && basic_protected)
  { 
    // Add the extra char position to the existing field
    if (textBufferSize<TBS) textBuffer[ textBufferSize++ ] = ' ';
    textBufferLength++; 
    // Complete prev. field
    completeField();
  }
  else
  { 
    // Use a FieldStart
    // Complete prev. field
    completeField();
    // Add field start position 
    addFieldStart();
  }

  // Clear Extended Attributes
  field_ea_reverse = false;
  field_ea_blink = false;
  field_ea_underline = false;
  basic_ea_intensified = false;
  field_ea_colour = 0;
  field_ea_reverse_override = false;
  field_ea_blink_override = false;
  field_ea_underline_override = false;
  basic_ea_intensified_override = false;

  // It seems we need these - not sure why
  field_reverse = false;
  field_blink = false;
  field_underline = false;

  // Skip to the "("
  for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!='(';parsePos++);

  // Process Attributes
  do doParseAttr(false);
  while ((result3270[parseLine])[parsePos]==',');

  // Skip to the next space character
  for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!=' ';parsePos++);
}

void MyTerminal::doParseSA()
{
  if (basic_protected) completeField();

  // Skip to the "("
  for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!='(';parsePos++);

  // If we have only spaces in the field we apply the extended attributes otherwise forget it
  // (as I have no way to handle them in html fields)
  bool empty=true;
  for (int i=0; i<textBufferSize; i++)
  {
    if (textBuffer[i]!=' ')
    {
      empty=false;
      break;
    }
  }

  if (empty)
  {
    // Process Attributes
    do doParseAttr(true);
    while ((result3270[parseLine])[parsePos]==',');
  }
  else
  {
    // Skip to the ")"
    for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!=')';parsePos++);
  }
  
  // Skip to the next space character
  for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!=' ';parsePos++);
}

void MyTerminal::doParseAttr(bool ea)
{
  // Skip the '(' or ','
  parsePos++;

  // Read Attr 
  int attr = hex2Bin((result3270[parseLine]) + parsePos);
  parsePos += 2;

  // Skip the '='
  parsePos++;

  // Read Value 
  int  val = hex2Bin((result3270[parseLine]) + parsePos);
  parsePos += 2;

  if (attr == 0xc0) // Basic Attributes
  {
    basic_protected = val & 0x20; // protected
    basic_numeric = val & 0x10; // numeric
    val = val & 0x0f;
    basic_detectable = (val == 0x04); // detectable
    basic_intensified = (val == 0x08); // intensified
    basic_nondisplay = (val == 0x0c); // non-display
    basic_modified = (val == 0x01); // modified
  }

  else if (attr == 0x41) // Highlighting
  {
    if (ea)
    {
      field_ea_reverse = val & 0x02; // reverse
      field_ea_blink = val & 0x01; // blink
      field_ea_underline = val & 0x04; // underscore
      basic_ea_intensified = val & 0x08; // intensified
      field_ea_reverse_override = true;
      field_ea_blink_override = true;
      field_ea_underline_override = true;
      basic_ea_intensified_override = true;
    }  
    else
    {
      field_reverse = val & 0x02; // reverse
      field_blink = val & 0x01; // blink
      field_underline = val & 0x04; // underscore
      basic_intensified = basic_intensified | (val & 0x08); // intensified
    }   
  }

  else if (attr == 0x42) // Foreground 
  {
    char *colour=0;
    switch(val)
    {
     case 0xf0: // neutral black
       colour = "neutralblack";
       break;
     case 0xf1: // blue
       colour = "blue";
       break;
     case 0xf2: // red
       colour = "red";
       break;
     case 0xf3: // pink
       colour = "pink";
       break;
     case 0xf4: // green
       colour = "green";
       break;
     case 0xf5: // turquoise
       colour = "turquoise";
       break;
     case 0xf6: // yellow
       colour = "yellow";
       break;
     case 0xf7: // neutral white
       colour = "neutralwhite";
       break;
     case 0xf8: // black
       colour = "black";
       break;
     case 0xf9: // deep blue
       colour = "deepblue";
       break;
     case 0xfa: // orange
       colour = "orange";
       break;
     case 0xfb: // purple
       colour = "purple";
       break;
     case 0xfc: // pale green
       colour = "palegreen";
       break;
     case 0xfd: // pale turquoise
       colour = "paleturquoise";
       break;
     case 0xfe: // grey
       colour = "grey";
       break;
     case 0xff: // white
       colour = "white";
       break;
     default:
       colour = 0;
    }
    if (ea) field_ea_colour = colour;
    else field_colour = colour;
  }
}

void MyTerminal::addFieldStart()
{
  Field* field;
  if ( fieldno < line[parseLine]->count() )
  {  // Reuse Field
     field = dynamic_cast<Field*>( line[parseLine]->widget(fieldno) );
     field->setNotInUse(false);
     field->setIsProtected(true);
     field->setNewValue(" ", 1);
  }
  else field = new Field(this, line[parseLine], " ", 1, true);

  field->setNumeric(false);
  field->setDetectable(false);
  field->setIntensified(false);
  field->setNondisplay(false);
  field->setReverse(false);
  field->setBlink(false);
  field->setUnderline(false);
  field->setColour(0);

  field->setScreenRow(parseLine);
  field->setScreenCol(screenCol);

  screenCol++;
  fieldno++;
}

void MyTerminal::completeField()
{
  if (textBufferSize)
  {
    textBuffer[textBufferSize] = 0;

    Field* field;
    if ( fieldno < line[parseLine]->count() )
    { // Reuse Field
      field = dynamic_cast<Field*>( line[parseLine]->widget(fieldno) );
      field->setNotInUse(false);
      field->setIsProtected(basic_protected);
      field->setNewValue(textBuffer, textBufferLength);
    }
    else field = new Field(this, line[parseLine], textBuffer, textBufferLength, basic_protected);

    field->setNumeric(basic_numeric);
    field->setDetectable(basic_detectable);
    field->setNondisplay(basic_nondisplay);
    
    if (basic_ea_intensified_override) field->setIntensified(basic_ea_intensified);
    else field->setIntensified(basic_intensified);
    
    if (field_ea_reverse_override) field->setReverse(field_ea_reverse);
    else field->setReverse(field_reverse);

    if (field_ea_blink_override) field->setBlink(field_ea_blink);
    else field->setBlink(field_blink);
    
    if (field_ea_underline_override) field->setUnderline(field_ea_underline);
    else field->setUnderline(field_underline);
    
    if (field_ea_colour) field->setColour(field_ea_colour);
    else field->setColour(field_colour);

    field->setScreenRow(parseLine);
    field->setScreenCol(screenCol);

    screenCol += textBufferLength;
    fieldno++;
    textBufferSize=0;
    textBufferLength=0;
  }
}


// Escape spaces to &nbsp; 
char* MyTerminal::escSpace(const char* text)
{
  // Escape spaces to &nbsp; 
  char* buffer = (char*)malloc(strlen(text)*6+1);
  int out=0;
  for (int in=0; text[in]; in++)
  {
    if (text[in]==' ')
    {
      memcpy(buffer+out,"&nbsp;",6);
      out+=6;
    }
    else  buffer[out++] = text[in];
  }
  buffer[out]=0;
  return buffer;
}


// Escape " to \";
char* MyTerminal::escSpeachMarks(const char* text)
{
  int out=0;
  for (int in=0; text[in]; in++)
  {
    if (text[in]=='\"')
    {
      memcpy(escSpeachMarksBuffer+out,"\\\"",2);
      out+=2;
    }
/*    else if (text[in]=='¬')
    {
      memcpy(escSpeachMarksBuffer+out,"\\¬",2);
      out+=2;
    }
*/    else  escSpeachMarksBuffer[out++] = text[in];
  }
  escSpeachMarksBuffer[out]=0;
  return escSpeachMarksBuffer;
}

// Connect or Disconnect Host
void MyTerminal::ConnectAction()
{
  if (s3270Pid[activeHost]) stopS3270();
  else startS3270();
}

// Change Current Host
void MyTerminal::changeHostAction()
{
  activeHost = hostSelect->currentIndex();

  if (s3270Pid[activeHost]) 
  {
    topKeyboard->show();
    screen->show();
    statusLine->show();
    connectButton->setText("Disconnect");
    renderScreen();
    setCursorPos();
  }
  else
  {
    topKeyboard->hide();
    screen->hide();
    statusLine->hide();
    connectButton->setText("Connect");
    redrawTimer->stop();
  }

  messageLine->setText(""); // FIXME - Save the message per session/host?
}

// Start S3270 
// Return 0 for OK
int MyTerminal::startS3270()
{
  struct sockaddr_un ssun;
  int fd;
  int rc;
  
  // Get the s3270 location from the wt_config.xml config file
  std::string s3270;
  if ( !WApplication::readConfigurationProperty("s3270Location",s3270) ) s3270="s3270";

  // Fork etc.
  s3270Pid[activeHost] = fork();

  if (s3270Pid[activeHost] == 0)
  {
    // We are the child process

    // Become a new Session Group Leader - don't want to signals to get mixed up 
    setsid();

    // Close any open FDs - ugly but there it is ...
    for (int i=0; i<sysconf(_SC_OPEN_MAX); i++) close(i); 

    char host[200];
    sprintf(host, "%s:%d", server[activeHost], port[activeHost]);

    // exec s3270 
    execlp(s3270.c_str(), "s3270", "-socket", "-clear", "aidWait", "-model", "3279-3-E", "-set", "blankFill", host, NULL);

    // If exec failed 
    syslog (LOG_ERR, "Could not exec s3270 - %s", strerror(errno));
    exit(-1);
  }

  if (s3270Pid[activeHost] == -1)
  {
    // Error
    syslog (LOG_ERR, "Could not fork() - %s", strerror(errno));
    messageLine->setText("Failed to start s3270 - Could not fork(). Details have been logged, please record time and report");
    s3270Pid[activeHost]=0;
    stopS3270();
    return -1;
  }

  rc=command3270("Attn");
  if (rc<-1)
  {
    messageLine->setText("Failed to start s3270 - Attempted Attn");
    stopS3270();
    return -1;
  }
  rc=command3270("Clear");
  if (rc<-1)
  {
    messageLine->setText("Failed to start s3270 - Attempted Clear");
    stopS3270();
    return -1;
  }
  rc=command3270("Wait(3,3270Mode)");
  if (rc<-1)
  {
    messageLine->setText("Failed to start s3270 - Attempted Waiting for 3270 Mode");
    stopS3270();
    return -1;
  }
  rc=command3270("Wait(3,Unlock)");
  if (rc<-1)
  {
    messageLine->setText("Failed to start s3270 - Attempted Waiting for Unlock");
    stopS3270();
    return -1;
  }

  messageLine->setText("");
  topKeyboard->show();
  screen->show();
  statusLine->show();
  connectButton->setText("Disconnect");
  renderScreen();
  setCursorPos();
  return 0;
}

// Start S3270 
// Return 0 for OK
int MyTerminal::stopS3270()
{
  int rc;
  timespec tmReq;
  
  // Hide any screen lines/fields - firefox needs this
  for (int i=0; i<RSZ; i++) 
  {
   if ( line[i] && !(line[i]->isHidden()) ) 
   {
     for (int j=0; j<line[i]->count(); j++) 
     {
       Field* field = dynamic_cast<Field*>( line[i]->widget(j) );
       if (field) field->setNotInUse(true);
     }
     line[i]->setHidden(true);
   }
   else break;
  }

  if (s3270Pid[activeHost])
  {
    command3270("disconnect");
    command3270("quit");

    // Wait a bit and QUIT
    tmReq.tv_sec = 0;
    tmReq.tv_nsec = 500 * 1000000; // 500ms
    (void)nanosleep(&tmReq, (timespec *)NULL);
    kill(s3270Pid[activeHost], SIGQUIT);

    // Wait a bit more and KILL
    tmReq.tv_sec = 0;
    tmReq.tv_nsec = 500 * 1000000; // 500ms
    (void)nanosleep(&tmReq, (timespec *)NULL);
    kill(s3270Pid[activeHost], SIGKILL);

    waitpid(s3270Pid[activeHost], &rc, 0);
  }
  if (sockfd[activeHost])
  {
    fclose(inf[activeHost]);
    fclose(outf[activeHost]);
    close(sockfd[activeHost]);
  }
  sockfd[activeHost] = 0;
  inf[activeHost] = NULL;
  outf[activeHost] = NULL;
  s3270Pid[activeHost] = 0;
  topKeyboard->hide();
  screen->hide();
  statusLine->hide();
  connectButton->setText("Connect");
  redrawTimer->stop();
}

void MyTerminal::updateStatusLine()
{
  char buffer[10];

  switch (keyboardState)
  {
    case 'U':
      lockStateField->setText("");
      keyboardStateField->setText("");
      waitImage->setHidden(true);
      break;
    case 'L':
      lockStateField->setText("");
      waitImage->setHidden(false);
      keyboardStateField->setText("");
      break;
    default:
      lockStateField->setText("");
      waitImage->setHidden(false);
//      sprintf(buffer,"(C) Error=%c",keyboardState);
//      keyboardStateField->setText(buffer);
      keyboardStateField->setText("SYSTEM-or-ERROR");

  }

  if (connectionState == 'C')
  {
    switch (emulatorMode)
    {
      case 'I':
        connectionModeField->setText("3270 Mode");
        break;
      case 'L':
        connectionModeField->setText("VT Line Mode");
        break;
      case 'C':
        connectionModeField->setText("VT Char Mode");
        break;
      default:
        connectionModeField->setText("Unnegotiated");
    }
  }
  else connectionModeField->setText("Disconnected");

  sprintf(buffer,"%d",numberOfRows);
  numberOfRowsField->setText(buffer);

  sprintf(buffer,"%d",numberOfColumns);
  numberOfColumnsField->setText(buffer);
}


void MyTerminal::renderScreen()
{
  int i;

  if (!s3270Pid[activeHost]) return;

//  basic_protected = true;
  basic_protected = false;
  basic_numeric = false;
  basic_detectable = false;
  basic_intensified = false;
  basic_nondisplay = false;
  basic_modified = false;
  field_reverse = false;
  field_blink = false;
  field_underline = false;
  field_colour = 0;
  basic_ea_intensified = false;
  field_ea_reverse = false;
  field_ea_blink = false;
  field_ea_underline = false;
  field_ea_colour = 0;
  basic_ea_intensified_override = false;
  field_ea_reverse_override = false;
  field_ea_blink_override = false;
  field_ea_underline_override = false;
      
  int c3270rc;
  c3270rc = command3270("readbuffer(ascii)");
  if (c3270rc<-1)
  {
    messageLine->setText("S3270 Died - Attempting Readbuffer");
    stopS3270();
    return;
  }

  if (c3270rc)
  {
    char buf[100];
    if (result3270[0])
    {
      sprintf(buf, "S3270 Error - %s", result3270[0]);
      messageLine->setText(buf);
    }
    else messageLine->setText("Unknown S3270 Error");

    // Update Status Line - FIXME Do we want to do this?
    updateStatusLine();

    return;
  }

  if (connectionState!='C')
  {
    messageLine->setText("Host Disconnected");
    stopS3270();
    return;
  }

  messageLine->setText("");

  // OK first we have to work backwards to find the last SF (which defines the attributes of the begining of the screen)
  // Get last line
  int lastLine=-1;
  for (parseLine=0; parseLine<RSZ; parseLine++) 
  {
   if (result3270[parseLine]) lastLine=parseLine;
   else break;
  }

  // Work Backwards
  for (parseLine=lastLine; parseLine>=0; parseLine--) 
  {
    getLastSF();
    if (parsePos>-1) // SF Found
    {
      // Skip to the "("
      for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!='(';parsePos++);

      // Process Attributes
      do doParseAttr(false);
      while ((result3270[parseLine])[parsePos]==',');

      // We're Done!
      break;
    }
  }


  // Now Process Screen Proper
  for (parseLine=0; parseLine<RSZ; parseLine++) 
  {
   if (result3270[parseLine])
   { 
     if (!line[parseLine])
     {
       line[parseLine] = new WContainerWidget(screen);
       line[parseLine]->setInline(false);
       line[parseLine]->setStyleClass(lineStyle);
     }
     if (line[parseLine]->isHidden()) line[parseLine]->setHidden(false);

     doParseLine();
   }
   else break;
  }

  // Hide any extra lines
  for (i=parseLine; i<RSZ; i++) 
  {
   if ( line[i] && !(line[i]->isHidden()) ) 
   {
     for (int j=0; j<line[i]->count(); j++) 
     {
       Field* field = dynamic_cast<Field*>( line[i]->widget(j) );
       if (field) field->setNotInUse(true);
     }
     line[i]->setHidden(true);
   }
   else break;
  }

  // Update Status Line
  updateStatusLine();

  // Refresh Timer
  int waitSecs;
  if (keyboardState == 'L' || keyboardState == 'E') waitSecs = 2;
  else waitSecs = 6;

  WApplication::instance()->doJavaScript( "{" + WApplication::instance()->javaScriptClass() + ".setLock(false,\"\");}" );

  redrawTimer->setInterval(1000 * waitSecs);
  if (waitSecs) redrawTimer->start();
  else redrawTimer->stop();

  // Screen Width
  int width;
  if (isIE) width=9*numberOfColumns;
  else width=9*numberOfColumns + 1;
  resize(width, WLength::Auto);
}


// Get the last SF field on the line
void MyTerminal::getLastSF()
{
  parsePos=0;
  int lastSF=-1;

  for (;(result3270[parseLine])[parsePos]==' ';parsePos++); // Remove Leading Spaces

  do
  {
    if ((result3270[parseLine])[parsePos]=='S')
    { 
      parsePos++;
      if ((result3270[parseLine])[parsePos]=='F') lastSF=parsePos;
    }
    for (;(result3270[parseLine])[parsePos] && (result3270[parseLine])[parsePos]!=' ';parsePos++); // Skip 'till we get a space
    for (;(result3270[parseLine])[parsePos]==' ';parsePos++); // Remove Training Spaces
  }
  while ((result3270[parseLine])[parsePos]); // Until we reach the end of the line

  parsePos=lastSF;
}

void MyTerminal::setCursorPos()
{
  char buffer[10];
  sprintf(buffer,"%d",cursorColumn+1);
  cursorColumnField->setText(buffer);
  sprintf(buffer,"%d",cursorRow+1);
  cursorRowField->setText(buffer);

  // Javascript to set field focus
  //  setFocusFieldSlot->exec(); // Note: Does not work (in the version of Witty I was using at the time - reported fixed)
  WApplication::instance()->doJavaScript( setFocusFieldFunc );
}

void MyTerminal::processScreenUpdates()
{
  int curRow = -1;
  int curCol = -1;
  char cbuffer[TBS*2+20];
  int c3270rc;

  if (!s3270Pid[activeHost]) return;

  // Loop through Each Line
  for (int i=0; i<RSZ; i++) 
  {
   if (line[i])
   { 
     // Loop through each field
     for (int j=0; j<line[i]->count(); j++) 
     {
       Field* field = dynamic_cast<Field*>( line[i]->widget(j) );
       if (field)
       {
         if ( !field->isProtected() )
         {
           // OK has this field changed value? 
           if ( field->originalValue() != field->text() )
           {
              sprintf(cbuffer, "MoveCursor(%d, %d)", field->screenRow(), field->screenCol());
              c3270rc = command3270(cbuffer);
              if (c3270rc<-1)
              {
                messageLine->setText("S3270 Died - Attempting MoveCursor(FieldUpdate)");
                stopS3270();
                return;
              }

              c3270rc = command3270("EraseEOF");
              if (c3270rc<-1)
              {
                messageLine->setText("S3270 Died - Attempting EraseEOF");
                stopS3270();
                return;
              }

              // Esc out " chars
              char* escBuffer = escSpeachMarks( field->text().toUTF8().c_str() );
              sprintf(cbuffer, "String(\"%s\")", escBuffer);
              c3270rc = command3270(cbuffer);
              if (c3270rc<-1)
              {
                messageLine->setText("S3270 Died - Attempting String");
                stopS3270();
                return;
              }

              field->setOriginalValue(field->text().toUTF8().c_str());
           }
         } 
       }
     }
   }
   else break;
  }

  // Set Cursor Position
  int r=atoi( cursorRowField->text().narrow().c_str() ) - 1;
  int c=atoi( cursorColumnField->text().narrow().c_str() ) - 1;
  sprintf(cbuffer, "MoveCursor(%d, %d)", r, c);
  c3270rc = command3270(cbuffer);
  if (c3270rc<-1)
  {
    messageLine->setText("S3270 Died - Attempting MoveCursor(CurrentField)");
    stopS3270();
    return;
  }
}

void MyTerminal::doTimer()
{
  int oldCol = cursorColumn;
  int oldRow = cursorRow;
  renderScreen();
  if (keyboardState == 'L' || keyboardState == 'E') setCursorPos();
  else if ((oldCol!=cursorColumn) || (oldRow!=cursorRow)) setCursorPos();
}

// Key Functions
void MyTerminal::doAid(char *command)
{
  int c3270rc;

  if (!s3270Pid[activeHost]) return;

  processScreenUpdates();
  if (command && strlen(command)) 
  {
    c3270rc = command3270(command);
    if (c3270rc<-1)
    {
      messageLine->setText("S3270 Died - Attempting Aid");
      stopS3270();
      return;
    }
  }
  c3270rc = command3270("Wait(1,Unlock)");
  if (c3270rc<-1)
  {
    messageLine->setText("S3270 Died - Attempting Wait(Unlock)");
    stopS3270();
    return;
  }

  renderScreen();
  setCursorPos();
}

void MyTerminal::doAidSlot(std::string command)
{
  if (command.length()) doAid((char*)command.c_str());
  else doAid(0);
}

// IND&FILE Stuff

/* IND$FILE Options
Transfer(keyword=value,...)
	Keyword		Required?	Default		Other Values
	Direction	No  		send		receive
	HostFile	Yes
	LocalFile	Yes
	Host		No		tso		vm
	Mode		No		ascii		binary
	Cr		No		remove		add, keep
	Exist		No		keep		replace, append
	Recfm		No				fixed, variable, undefined
	Lrecl		No
	Blksize		No
	Allocation	No				tracks, cylinders, avblock
	PrimarySpace	No
	SecondarySpace	No
	BufferSize	No		4096
*/


/* download - download - File Functionality */
class DownloadFile : public WDialog
{
  public:
    DownloadFile(MyTerminal* s);
    ~DownloadFile();
    void indFile();

  private:
   MyTerminal* session_;
   bool doneIndFile; // Stops multiple requests
   WText* errorMessage;
   WText* successMessage;
   WPushButton* done;
   WPushButton* transferButton;
   char tempFileName[100];

   WComboBox *hostTypeField;

   WLineEdit* fileNameField;

   WComboBox *modeField;
   void modeChanged();

   WComboBox *crField;

   WAnchor *downloadLink;
   WFileResource *downloadFile;
};

DownloadFile::~DownloadFile()
{
  if (downloadFile) delete downloadFile;
  unlink(tempFileName);
}

DownloadFile::DownloadFile(MyTerminal* s) : WDialog("Download File")
{
  session_=s;
  doneIndFile = false;
  downloadFile = 0;

  // Spool file
  snprintf(tempFileName,100,"/tmp/console.tmp.download.%s.%d.%d.%d", session_->userID,getpid(),rand(),time(NULL));
  struct stat statInfo;
  if (stat(tempFileName,&statInfo) == 0) unlink(tempFileName); // File exists ... so delete it (why not?)

  resize(400, WLength::Auto);

  // Form widgets
  hostTypeField = new WComboBox();
  hostTypeField->addItem("tso");
  hostTypeField->addItem("vm");
  hostTypeField->setCurrentIndex(0);

  fileNameField = new WLineEdit();

  modeField = new WComboBox();
  modeField->addItem("ascii");
  modeField->addItem("binary");
  modeField->setCurrentIndex(0);
  modeField->activated().connect(SLOT(this, DownloadFile::modeChanged));

  crField = new WComboBox();
  crField->addItem("remove");
  crField->addItem("add");
  crField->addItem("keep");
  crField->setCurrentIndex(0);

  errorMessage = new WText();

  transferButton = new WPushButton("Execute IND$FILE");
  transferButton->clicked().connect(SLOT(this, DownloadFile::indFile));

  successMessage = new WText();

  downloadLink = new WAnchor();
  downloadLink->setTarget(TargetNewWindow);
  downloadLink->hide();

  done = new WPushButton("Cancel");
  done->clicked().connect(SLOT(this, WDialog::accept));

  // Grid
  WGridLayout *layout = new WGridLayout();
  WLabel *l;
  int row = 0;

  layout->addWidget(l = new WLabel("Host Type: "), row, 0);
  layout->addWidget(hostTypeField, row, 1);
  l->setBuddy(hostTypeField);
  ++row;

  layout->addWidget(l = new WLabel("Host File Name: "), row, 0);
  layout->addWidget(fileNameField, row, 1);
  l->setBuddy(fileNameField);
  ++row;

  layout->addWidget(l = new WLabel("Mode: "), row, 0);
  layout->addWidget(modeField, row, 1);
  l->setBuddy(modeField);
  ++row;

  layout->addWidget(l = new WLabel("CR Action: "), row, 0);
  layout->addWidget(crField, row, 1);
  l->setBuddy(crField);
  ++row;

  WContainerWidget *c = new WContainerWidget();
  c->resize(WLength::Auto, 15);
  c->addWidget(errorMessage);
  layout->addWidget(c, row, 0, 0, 2, AlignCenter);
  ++row;

  c = new WContainerWidget();
  c->addWidget(transferButton);
  c->addWidget(successMessage);
  c->addWidget(downloadLink);
  layout->addWidget(c, row, 0, 0, 2, AlignCenter);
  ++row;

  layout->addWidget(done, row, 1, AlignRight);

  layout->setColumnStretch(1, 1);

  contents()->setLayout(layout, AlignTop | AlignJustify);
}

void DownloadFile::modeChanged()
{
  if (modeField->currentText() == "ascii") crField->enable();
  else crField->disable();
}

void DownloadFile::indFile()
{
  if (!doneIndFile)
  {
    doneIndFile=true;
    char* error=0;
    char* commandTemplate = "Transfer(Host=%s,\"LocalFile=%s\",Direction=receive,"
                            "\"HostFile=%s\","
                            "Mode=%s,"
                            "Cr=%s"
                            ")";
    char command[1000];
    char fileName[100]; fileName[99]=0;

    strncpy(fileName, session_->escSpeachMarks( fileNameField->text().narrow().c_str() ), 99);

    snprintf(command, 1000, commandTemplate, 
            hostTypeField->currentText().narrow().c_str(),
            tempFileName, fileName,
            modeField->currentText().narrow().c_str(),
            crField->currentText().narrow().c_str() );

    session_->command3270(command);
    for (int i=0; i<RSZ; i++) 
    {
     if ((session_->result3270)[i]) error = (session_->result3270)[i];
     else break;
    }

    if (!error)
    {
     struct stat statInfo;
     if (stat(tempFileName,&statInfo) != 0) // File does not seem to exist
     {
       error = "Unspecified IND&amp;FILE failure - file missing?";
     }
    }

    if (error)
    {
      errorMessage->setText(error);
      doneIndFile=false;
    }
    else
    {
      errorMessage->setText("");
      fileNameField->disable();
      modeField->disable();
      crField->disable();
      hostTypeField->disable();

      transferButton->hide();
      successMessage->setText("IND&amp;FILE Successful. Click to download -> ");

      downloadFile = new WFileResource("text/csv", tempFileName);
      downloadFile->suggestFileName(fileName);
      downloadFile->setMimeType("text/plain");
      downloadLink->setResource(downloadFile);
      downloadLink->setText(fileName);
      downloadLink->show();

      done->setText("Close");
    }
  }
}

void MyTerminal::downloadFile()
{
  DownloadFile dialog(this);

  redrawTimer->stop();

  dialog.exec();

  renderScreen();
  setCursorPos();
}

/* Upload - send - File Functionality */
class UploadFile : public WDialog
{
  public:
    UploadFile(MyTerminal* s);
    ~UploadFile();

  private:
   void transferAction();

   MyTerminal* session_;
   bool doneUploadFile; // Stops multiple simultanios requests
   WText* message;
   WPushButton* done;
   WPushButton* transferButton;

   WComboBox *hostTypeField;

   WLineEdit* fileNameField;

   WComboBox *modeField;
   void modeChanged();

   WComboBox *crField;

   WFileUpload *uploadFile;
   void fileUploaded();
   void fileTooLarge();

   WText *uploadFileMessage;

   WComboBox *recFm;
   WLineEdit *lRecL;
   WLineEdit *blkSize;
   WComboBox *allocation;
   WLineEdit *primarySpace;
   WLineEdit *secondarySpace;
};

UploadFile::~UploadFile()
{
}

UploadFile::UploadFile(MyTerminal* s) : WDialog("Upload File")
{
  session_=s;
  doneUploadFile = false;

  resize(500, WLength::Auto);

  // Form widgets
  uploadFile = new WFileUpload();
  uploadFile->setFileTextSize(40);
  // Upload automatically when the user entered a file.
  uploadFile->changed().connect(SLOT(this, UploadFile::transferAction));
  // React to a succesfull upload.
  uploadFile->uploaded().connect(SLOT(this, UploadFile::fileUploaded));
  // React to a fileupload problem.
  uploadFile->fileTooLarge().connect(SLOT(this, UploadFile::fileTooLarge));

  uploadFileMessage = new WText();

  hostTypeField = new WComboBox();
  hostTypeField->addItem("tso");
  hostTypeField->addItem("vm");
  hostTypeField->setCurrentIndex(0);

  fileNameField = new WLineEdit();

  modeField = new WComboBox();
  modeField->addItem("ascii");
  modeField->addItem("binary");
  modeField->setCurrentIndex(0);
  modeField->activated().connect(SLOT(this, UploadFile::modeChanged));

  crField = new WComboBox();
  crField->addItem("remove");
  crField->addItem("add");
  crField->addItem("keep");
  crField->setCurrentIndex(0);

  message = new WText();
  message->setText("Select File to Upload");

  transferButton = new WPushButton("IND$FILE");
  transferButton->clicked().connect(SLOT(this, UploadFile::transferAction));

  done = new WPushButton("Cancel");
  done->clicked().connect(SLOT(this, WDialog::accept));

  recFm = new WComboBox();
  recFm->addItem("");
  recFm->addItem("fixed");
  recFm->addItem("variable");
  recFm->addItem("undefined");
  recFm->setCurrentIndex(0);

  lRecL = new WLineEdit();

  blkSize = new WLineEdit();

  allocation = new WComboBox();
  allocation->addItem("");
  allocation->addItem("tracks");
  allocation->addItem("cylinders");
  allocation->addItem("avblock");
  allocation->setCurrentIndex(0);

  primarySpace = new WLineEdit();

  secondarySpace = new WLineEdit();

  // Grid
  WGridLayout *layout = new WGridLayout();
  WLabel *l;
  int row = 0;

  layout->addWidget(l = new WLabel("Local File: "), row, 0);
  uploadFile->setInline(true);
  uploadFileMessage->setInline(true);
  WContainerWidget *c1 = new WContainerWidget();
  c1->resize(WLength::Auto, 20);
  c1->addWidget(uploadFile);
  c1->addWidget(uploadFileMessage);
  layout->addWidget(c1, row, 1);
  ++row;

  layout->addWidget(l = new WLabel("Host Type: "), row, 0);
  layout->addWidget(hostTypeField, row, 1);
  l->setBuddy(hostTypeField);
  ++row;

  layout->addWidget(l = new WLabel("Host File Name: "), row, 0);
  layout->addWidget(fileNameField, row, 1);
  l->setBuddy(fileNameField);
  ++row;

  layout->addWidget(l = new WLabel("Mode: "), row, 0);
  layout->addWidget(modeField, row, 1);
  l->setBuddy(modeField);
  ++row;

  layout->addWidget(l = new WLabel("CR Action: "), row, 0);
  layout->addWidget(crField, row, 1);
  l->setBuddy(crField);
  ++row;

  layout->addWidget(l = new WLabel("Record Format: "), row, 0);
  layout->addWidget(recFm, row, 1);
  l->setBuddy(recFm);
  ++row;

  layout->addWidget(l = new WLabel("Record Length: "), row, 0);
  layout->addWidget(lRecL, row, 1);
  l->setBuddy(lRecL);
  ++row;

  layout->addWidget(l = new WLabel("Block Size: "), row, 0);
  layout->addWidget(blkSize, row, 1);
  l->setBuddy(blkSize);
  ++row;

  layout->addWidget(l = new WLabel("Allocation: "), row, 0);
  layout->addWidget(allocation, row, 1);
  l->setBuddy(allocation);
  ++row;

  layout->addWidget(l = new WLabel("Primary Space: "), row, 0);
  layout->addWidget(primarySpace, row, 1);
  l->setBuddy(primarySpace);
  ++row;

  layout->addWidget(l = new WLabel("Secondary Space: "), row, 0);
  layout->addWidget(secondarySpace, row, 1);
  l->setBuddy(secondarySpace);
  ++row;

  WContainerWidget *c2 = new WContainerWidget();
  c2->resize(WLength::Auto, 20);
  c2->addWidget(message);
  layout->addWidget(c2, row, 0, 0, 2, AlignCenter);
  ++row;

  layout->addWidget(transferButton, row, 0, 0, 2, AlignCenter);
  ++row;

  layout->addWidget(done, row, 1, AlignRight);

  layout->setColumnStretch(1, 1);

  contents()->setLayout(layout, AlignTop | AlignJustify);

  fileNameField->disable();
  modeField->disable();
  crField->disable();
  hostTypeField->disable();
  transferButton->disable();

  recFm->disable();
  lRecL->disable();
  blkSize->disable();
  allocation->disable();
  primarySpace->disable();
  secondarySpace->disable();
}

void UploadFile::modeChanged()
{
  if (modeField->currentText() == "ascii") crField->enable();
  else crField->disable();
}

void UploadFile::fileUploaded()
{
  struct stat statInfo;
  if (stat(uploadFile->spoolFileName().c_str(),&statInfo) == 0) // File Exists ...
  {
    if (statInfo.st_size == 0)
    {
      uploadFileMessage->setText("Error: Empty File");
      uploadFile->hide();
      message->setText("Error - Please CANCEL and retry");
      return;
    }
  }
  else 
  {
    uploadFileMessage->setText("Error: Spool File Not Found - Empty File?");
    uploadFile->hide();
    message->setText("Error - Please CANCEL and retry");
    return;
  }

  transferButton->enable();
  message->setText("File Uploaded. Now execute IND$FILE");
  fileNameField->enable();
  modeField->enable();
  hostTypeField->enable();
  uploadFile->hide();
  doneUploadFile=true;

  recFm->enable();
  lRecL->enable();
  blkSize->enable();
  allocation->enable();
  primarySpace->enable();
  secondarySpace->enable();

  // Get file name
  WString fileName;
  const char* nm = uploadFile->clientFileName().narrow().c_str();
  for (int x=strlen(nm)-1; x; x--)
  {
    if (nm[x]=='\\' || nm[x]=='/')
    {
      fileName=nm+x+1;
      break;
    }
  }
  fileNameField->setText(fileName);
  uploadFileMessage->setText(fileName);

  // Is it ASCII?
  if (uploadFile->contentDescription()=="text/plain")
  {
    modeField->setCurrentIndex(0);
    crField->enable();
  }
  else
  {
    modeField->setCurrentIndex(1);
    crField->disable();
  }
}

void UploadFile::fileTooLarge()
{
  uploadFileMessage->setText("Error: File to large");
  uploadFile->hide();
  message->setText("Error - Please CANCEL and retry");

  transferButton->disable();
  uploadFile->hide();
  fileNameField->disable();
  modeField->disable();
  crField->disable();
  hostTypeField->disable();

  recFm->disable();
  lRecL->disable();
  blkSize->disable();
  allocation->disable();
  primarySpace->disable();
  secondarySpace->disable();
}

void UploadFile::transferAction()
{
  if (doneUploadFile)
  {
    char* error=0;
    std::string command = "Transfer(";

    command = command + "Host=" + hostTypeField->currentText().narrow();
    command = command + ",Direction=send";
    command = command + ",\"LocalFile=" + session_->escSpeachMarks( uploadFile->spoolFileName().c_str() ) + "\"";
    command = command + ",\"HostFile=" + session_->escSpeachMarks( fileNameField->text().narrow().c_str() ) + "\"";
    command = command + ",\"Mode=" + session_->escSpeachMarks( modeField->currentText().narrow().c_str() )  + "\"";
    command = command + ",\"Cr=" + session_->escSpeachMarks( crField->currentText().narrow().c_str() )  + "\"";
    if ( recFm->currentText() != "")
      command = command + ",\"recfm=" + session_->escSpeachMarks( recFm->currentText().narrow().c_str() )  + "\"";
    if ( lRecL->text() != "")
      command = command + ",\"lRecL=" + session_->escSpeachMarks( lRecL->text().narrow().c_str() )  + "\"";
    if ( blkSize->text() != "")
      command = command + ",\"blkSize=" + session_->escSpeachMarks( blkSize->text().narrow().c_str() )  + "\"";
    if ( allocation->currentText() != "")
      command = command + ",\"recfm=" + session_->escSpeachMarks( allocation->currentText().narrow().c_str() )  + "\"";
    if ( primarySpace->text() != "")
      command = command + ",\"primarySpace=" + session_->escSpeachMarks( primarySpace->text().narrow().c_str() )  + "\"";
    if ( secondarySpace->text() != "")
      command = command + ",\"secondarySpace=" + session_->escSpeachMarks( secondarySpace->text().narrow().c_str() )  + "\"";
    command = command + ")";

    session_->command3270( (char*)command.c_str() );
    for (int i=0; i<RSZ; i++) 
    {
     if ((session_->result3270)[i]) error = (session_->result3270)[i];
     else break;
    }

    if (error)
    {
      message->setText(error);
      return;
    }
    else 
    {
      message->setText("IND$FILE Complete");
      done->setText("Close");
    }
  }
  else
  {
     uploadFileMessage->setText("Uploading File - Please Wait ... ");
     message->setText("");
     uploadFile->upload();
  }
  transferButton->disable();
  uploadFile->hide();
  fileNameField->disable();
  modeField->disable();
  crField->disable();
  hostTypeField->disable();

  recFm->disable();
  lRecL->disable();
  blkSize->disable();
  allocation->disable();
  primarySpace->disable();
  secondarySpace->disable();
}

void MyTerminal::uploadFile()
{

  UploadFile dialog(this);
  redrawTimer->stop();

  dialog.exec();

  renderScreen();
  setCursorPos();
}
