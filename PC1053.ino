// global defines
#define SelectT2 22
#define SelectT1 24
#define SelectR2A 26
#define SelectR1 28
#define SelectR5 30
#define SelectR2 32
#define SelectAux 34
#define LineFeed 36
#define Tab 38
#define CrLf 40
#define UpShift 42
#define DownShift 44
#define Space 46
#define BackSpace 48
#define BlackShift 50
#define RedShift 52
#define TwrEOL 3
#define TwrCBResp 5
#define TwrEndForms 6
#define TwrCRLFTinlk 7
#define OnesA 23
#define OnesB 25
#define OnesC 27
#define OnesD 29
#define TensA 31
#define TensB 33
#define TensC 35
#define TensD 37
#define HundredsA 39
#define TabButton 41
#define SpaceButton 43
#define CRButton 45
#define TabSet 47
#define TabClr 49

// define the sequences to print for ribbon color control
#define ESC '\x1b'
#define OPEN '['
#define BLACK "30"
#define RED "91"
#define WHITE "107"
#define BOLD "1"
#define BACK "0"
#define END 'm'

// define states for BCD 7seg controllers
#define LOFF LOW
#define LON  HIGH

// define states for relay drivers
#define RLOW HIGH
#define RHIGH  LOW

// define timers for short and long cycles
long Ctime = 0;
long Ltime = 0;

// save commands for typewriter
int doT2;
int doT1;
int doR2A;
int doR1;
int doR2;
int doR5;
int doAux;
int doTab;
int saveTab;
int doSpace;
int saveSpace;
int doBackSpace;
int saveBackSpace;
int doCrLf;
int saveCrLf;
int doLineFeed;
int saveLineFeed;
int doUpShift;
int saveUpShift;
int doDownShift;
int saveDownShift;
int doBlackShift;
int saveBlackShift;
int oldBlackShift;
int doRedShift;
int saveRedShift;
int oldRedShift;
int doSpaceButton;
int oldSpaceButton;
int doTabButton; 
int oldTabButton; 
int doCRButton;
int oldCRButton;
int doTabSet;
int doTabClr;

// define states for state machines
#define Idle 0
#define Wait 1
#define Grab 2
#define Trig 3
#define Break 4
#define Emit 5

// define state machines
int Cstate = Idle;
int Lstate = Idle;

// working variables
int anychar = 0;
int anycommand = 0;
int anyop = 0;
int Caps = 0;
int Cols = 1;
int lmargin = 1;
int rmargin = 120;

// select code table
int Cselect[44] = {15, 6,  7, 12, 13,  4,  5,  9,  8, 31,
              22, 23, 28, 29, 20, 21, 25, 24, 38, 39,
              44, 45, 36, 37, 41, 40, 63, 54, 55, 60,
              61, 52, 53, 57, 56, 49, 48, 47, 33, 32,
              17, 16, 1, 0};
              
// Upper case character table
char Uchar[44] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
              'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
              'U', 'V', 'W', 'X', 'Y', 'Z', '(', '+', '<', 'n',
              ')', ';', '*', '\'', '"', '|', '=', '_', '?', ':', 
              '>', '!', '%', 'c'};
              
// Lower case character table
char Lchar[44] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
              'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
              'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4',
              '5', '6', '7', '8', '9', '0', '#', '/', '-', ',', 
              '&', '$', '@', '.'};

// tab stop table
int stops[120];

void setup() {
  int i;
  
  // set up output pins
  pinMode(TwrEOL,  OUTPUT);
  pinMode(TwrCBResp,  OUTPUT);
  pinMode(TwrEndForms,  OUTPUT);
  pinMode(TwrCRLFTinlk,  OUTPUT);
  pinMode(OnesA,  OUTPUT);
  pinMode(OnesB,  OUTPUT);
  pinMode(OnesC,  OUTPUT);
  pinMode(OnesD,  OUTPUT);
  pinMode(TensA,  OUTPUT);
  pinMode(TensB,  OUTPUT);
  pinMode(TensC,  OUTPUT);
  pinMode(TensD,  OUTPUT);
  pinMode(HundredsA,  OUTPUT);

  // set output pins to initial state of OFF
  digitalWrite(TwrEOL,  RLOW);
  digitalWrite(TwrCBResp,  RHIGH);
  digitalWrite(TwrEndForms,  RHIGH);
  digitalWrite(TwrCRLFTinlk,  RLOW);
  digitalWrite(OnesA,  LON);
  digitalWrite(OnesB,  LOFF);
  digitalWrite(OnesC,  LOFF);
  digitalWrite(OnesD,  LOFF);
  digitalWrite(TensA,  LOFF);
  digitalWrite(TensB,  LOFF);
  digitalWrite(TensC,  LOFF);
  digitalWrite(TensD,  LOFF);
  digitalWrite(HundredsA,  LOFF);

  // set up input pins
  pinMode(SelectT2, INPUT_PULLUP);
  pinMode(SelectT1, INPUT_PULLUP);
  pinMode(SelectR2A, INPUT_PULLUP);
  pinMode(SelectR1, INPUT_PULLUP);
  pinMode(SelectR5, INPUT_PULLUP);
  pinMode(SelectR2, INPUT_PULLUP);
  pinMode(SelectAux, INPUT_PULLUP);
  pinMode(LineFeed, INPUT_PULLUP);
  pinMode(Tab, INPUT_PULLUP);
  pinMode(CrLf, INPUT_PULLUP);
  pinMode(UpShift, INPUT_PULLUP);
  pinMode(DownShift, INPUT_PULLUP);
  pinMode(Space, INPUT_PULLUP);
  pinMode(BackSpace, INPUT_PULLUP);
  pinMode(BlackShift, INPUT_PULLUP);
  pinMode(RedShift, INPUT_PULLUP);
  pinMode(TabButton, INPUT_PULLUP);
  pinMode(SpaceButton, INPUT_PULLUP);
  pinMode(CRButton, INPUT_PULLUP);
  pinMode(TabClr, INPUT_PULLUP);
  pinMode(TabSet, INPUT_PULLUP);

  // initialize typewriter with no tab stops
  for (i=0; i < 119; i++) {
    stops[i] = 0;
  }

  // set up ribbon shift old state as off
  oldBlackShift = 0;
  oldRedShift = 0;
 

  // set up three faceplate buttons old state as off
  oldTabButton = 0;
  oldSpaceButton = 0;
  oldCRButton = 0;

  // start communications with PC
  Serial.begin(9600);
  Serial.println("PC1053 is active");
  digitalWrite(TwrEndForms,  RLOW);

  // set up terminal to black ribbon
  Serial.print(ESC);
  Serial.print(OPEN);
  Serial.print(BACK);
  Serial.print(END);
  Serial.print(ESC);
  Serial.print(OPEN);
  Serial.print(WHITE);
  Serial.print(END);
  Serial.print(ESC);
  Serial.print(OPEN);
  Serial.print(BLACK);
  Serial.print(END);
}

// routine to convert Selectric selection code to
// ASCII character corresponding to typeball
char convChar() {
  int i;
  int select;

  // AUX will be on for any real print selection, otherwise it is a dummy cycle
  if (doAux == 0) {
    return '~';
  }
  select = 0;
  if (doT2 > 0) {
    select += 32;
  }
  if (doT1 > 0) {
    select += 16;
  }
  if (doR1 > 0) {
    select += 8;
  }
  if (doR2A > 0) {
    select += 4;
  }
  if (doR2 > 0) {
    select += 2;
  }
  if (doR5 > 0) {
    select += 1;
  }
  // loop through select table to match select code
  for (i=0; i < 44; i++) {
    if (Cselect[i] == select) {
      if (Caps == 1) {
        return Uchar[i];
      } else {
        return Lchar[i];
      }
    }
  }
  return ('~');
}

// figure out distance to tab and print spaces for that length
void doTabbing() {
  int i;
  for (i = Cols; i < rmargin; i++) {
    if (stops[i] == 0) {
      Serial.print(' ');
      updateLED();
      Cols += 1;
    }
    if (stops[i] == 1) {
      Cols += 1;
      return;
    }
  }
  return;
}

// update the top stops based on command from PC
void updateStops(int mypos, int val) {
  int i;
  if (mypos == 0) {
    return;
  }
  for (i = 0; i < 120; i++) {
    if (i == (mypos - 1)) {
      stops[i] = val;
    }
  }
  return;
}

// return distance to tab for timing calculation
int calcTab() {
  int i;
  for (i = Cols; i < 120; i++) {
    if (stops[i] == 1) {
      return i - Cols + 1;
    }
  }
  return rmargin - Cols + 1;
}

// extracts column number from margin command
int getcolumn(char * command, int margin) {
  int total;
  if (isDigit(command[3]) and isDigit(command[4]) and isDigit(command[5])) {
    total = (int(command[3]) - 48) * 100;
    total += (int(command[4])-48) * 10;
    total += int(command[5]) - 48;
    if (( total <= 0) or (total > 120)) {
      Serial.print("Margin column number out of range - "); Serial.println(total);
      return margin;
    }
  } else {
    Serial.println("Not a valid column number, ignoring command");
    return margin;
  }
  return total;
}

void updateLED() {
  int ColTens;
  int ColOnes;

  if (Cols > 99) {
    digitalWrite(HundredsA, LON);
  } else {
    digitalWrite(HundredsA, LOFF);
  }
  ColOnes = Cols % 10;
  if (ColOnes == 0) {
    digitalWrite(OnesA, LOFF);
    digitalWrite(OnesB, LOFF);
    digitalWrite(OnesC, LOFF);
    digitalWrite(OnesD, LOFF);
  } else if (ColOnes == 1) {
    digitalWrite(OnesA, LON);
    digitalWrite(OnesB, LOFF);
    digitalWrite(OnesC, LOFF);
    digitalWrite(OnesD, LOFF);      
  } else if (ColOnes == 2) {
    digitalWrite(OnesA, LOFF);
    digitalWrite(OnesB, LON);
    digitalWrite(OnesC, LOFF);
    digitalWrite(OnesD, LOFF);      
  } else if (ColOnes == 3) {
    digitalWrite(OnesA, LON);
    digitalWrite(OnesB, LON);
    digitalWrite(OnesC, LOFF);
    digitalWrite(OnesD, LOFF);      
  } else if (ColOnes == 4) {
    digitalWrite(OnesA, LOFF);
    digitalWrite(OnesB, LOFF);
    digitalWrite(OnesC, LON);
    digitalWrite(OnesD, LOFF);      
  } else if (ColOnes == 5) {
    digitalWrite(OnesA, LON);
    digitalWrite(OnesB, LOFF);
    digitalWrite(OnesC, LON);
    digitalWrite(OnesD, LOFF);      
  } else if (ColOnes == 6) {
    digitalWrite(OnesA, LOFF);
    digitalWrite(OnesB, LON);
    digitalWrite(OnesC, LON);
    digitalWrite(OnesD, LOFF);      
  } else if (ColOnes == 7) {
    digitalWrite(OnesA, LON);
    digitalWrite(OnesB, LON);
    digitalWrite(OnesC, LON);
    digitalWrite(OnesD, LOFF);      
  } else if (ColOnes == 8) {
    digitalWrite(OnesA, LOFF);
    digitalWrite(OnesB, LOFF);
    digitalWrite(OnesC, LOFF);
    digitalWrite(OnesD, LON);      
  } else if (ColOnes == 9) {
    digitalWrite(OnesA, LON);
    digitalWrite(OnesB, LOFF);
    digitalWrite(OnesC, LOFF);
    digitalWrite(OnesD, LON);      
  }
  ColTens = (Cols / 10) % 10;
  if (ColTens == 0) {
    digitalWrite(TensA, LOFF);
    digitalWrite(TensB, LOFF);
    digitalWrite(TensC, LOFF);
    digitalWrite(TensD, LOFF);
  } else if (ColTens == 1) {
    digitalWrite(TensA, LON);
    digitalWrite(TensB, LOFF);
    digitalWrite(TensC, LOFF);
    digitalWrite(TensD, LOFF);      
  } else if (ColTens == 2) {
    digitalWrite(TensA, LOFF);
    digitalWrite(TensB, LON);
    digitalWrite(TensC, LOFF);
    digitalWrite(TensD, LOFF);      
  } else if (ColTens == 3) {
    digitalWrite(TensA, LON);
    digitalWrite(TensB, LON);
    digitalWrite(TensC, LOFF);
    digitalWrite(TensD, LOFF);      
  } else if (ColTens == 4) {
    digitalWrite(TensA, LOFF);
    digitalWrite(TensB, LOFF);
    digitalWrite(TensC, LON);
    digitalWrite(TensD, LOFF);      
  } else if (ColTens == 5) {
    digitalWrite(TensA, LON);
    digitalWrite(TensB, LOFF);
    digitalWrite(TensC, LON);
    digitalWrite(TensD, LOFF);      
  } else if (ColTens == 6) {
    digitalWrite(TensA, LOFF);
    digitalWrite(TensB, LON);
    digitalWrite(TensC, LON);
    digitalWrite(TensD, LOFF);      
  } else if (ColTens == 7) {
    digitalWrite(TensA, LON);
    digitalWrite(TensB, LON);
    digitalWrite(TensC, LON);
    digitalWrite(TensD, LOFF);      
  } else if (ColTens == 8) {
    digitalWrite(TensA, LOFF);
    digitalWrite(TensB, LOFF);
    digitalWrite(TensC, LOFF);
    digitalWrite(TensD, LON);      
  } else if (ColTens == 9) {
    digitalWrite(TensA, LON);
    digitalWrite(TensB, LOFF);
    digitalWrite(TensC, LOFF);
    digitalWrite(TensD, LON);      
  }
  return;  
}

void loop() {
  char value;
  int ColTens;
  int ColOnes;
  char command[7] = {0,0,0,0,0,0,0};
  int cpointer = 0;
  int tabcol;
  bool gotTabButton = false;
  bool gotCRButton = false;
  bool gotSpaceButton = false;
  char savechar;
  
  while (1) {

    // do a quick check for incoming command from PC
    // must be one of these these commands exactly
    //  LM xxx     
    //  RM xxx
    //  TS xxx
    //  TC xxx
    //  TABS =
    //  TYPE t
    //    xxx is three digit column number with leading zeroes if needed
    //    t is either N for normal or A for APL typeball
    while (Serial.available() > 0) {
      command[cpointer++] = Serial.read();
      if (cpointer > 6) {
        Serial.println("input command too long, ignored");
        cpointer = 0;
      }
      if ((command [cpointer-1] == '\r') or (command[cpointer-1] == '\n')) {
        if (cpointer > 1) {
          Serial.println("short command, ignored");
        }
        cpointer = 0;
      }
      if (cpointer == 6) {
        // command is complete, process it
        if ((command[0] == 'L') and (command[1] == 'M') and (command[2] == ' ')){
          lmargin = getcolumn(command,lmargin);
          Serial.print("Left Margin set to "); Serial.println(lmargin);
        } else if ((command[0] == 'R') and (command[1] == 'M') and (command[2] == ' ')){
          rmargin = getcolumn(command,rmargin);
          Serial.print("Right Margin set to "); Serial.println(rmargin);
        } else if ((command[0] == 'T') and (command[1] == 'Y') and (command[2] == 'P') and (command[3] == 'E') and (command[4] == ' ')){
          Serial.println("Typeball switching not yet supported");
        } else if ((command[0] == 'T') and (command[1] == 'A') and (command[2] == 'B') and (command[3] == 'S') and (command[4] == ' ') and (command[5] == '=')){
          Serial.print("Tab stops are set at columns");
          int i;
          for (i = 0; i < 120; i++) {
            if (stops[i] == 1) {
              Serial.print(" "); Serial.print(i+1); 
            }
          }
          Serial.println('\n');
        } else if ((command[0] == 'T') and (command[1] == 'S') and (command[2] == ' ')){
          tabcol = getcolumn(command,0);
          Serial.print("Tab set at column "); Serial.println(tabcol);
          updateStops(tabcol, 1);
        } else if ((command[0] == 'T') and (command[1] == 'C') and (command[2] == ' ')){
          tabcol = getcolumn(command,0);
          Serial.print("Tab cleared at column "); Serial.println(tabcol);
          updateStops(tabcol, 0);
        } else {
          Serial.println("Not a valid command");
        }
        cpointer = 0;
      }
    }

    if (Cols > rmargin) {
      Serial.print('\b');
      Cols = rmargin;
    }

    // First check for input commands if idle
    if ((Cstate == Idle) and (Lstate == Idle)) {
      doT2 = not digitalRead(SelectT2);
      doT1 = not digitalRead(SelectT1);
      doR2A = not digitalRead(SelectR2A);
      doR1 = not digitalRead(SelectR1);
      doR2 = not digitalRead(SelectR2);
      doR5 = not digitalRead(SelectR5);
      doAux = not digitalRead(SelectAux);
      doLineFeed = not digitalRead(LineFeed);
      doSpace = not digitalRead(Space);
      doBackSpace = not digitalRead(BackSpace);
      doTab = not digitalRead(Tab);
      doCrLf = not digitalRead(CrLf);
      doUpShift = not digitalRead(UpShift);
      doDownShift = not digitalRead(DownShift);
      doBlackShift = not digitalRead(BlackShift);
      doRedShift = not digitalRead(RedShift);
      doSpaceButton = not digitalRead(SpaceButton);
      doTabButton = not digitalRead(TabButton);
      doCRButton = not digitalRead(CRButton);
      doTabSet = not digitalRead(TabSet);
      doTabClr = not digitalRead(TabClr);
      anychar = doT1 + doT2 + doR2A + doR2 + doR1 + 
        doR5 + doAux;
      anyop = doTab + 
        doSpace + doBackSpace +
        doUpShift + doDownShift;
      if ((doSpaceButton == 1) and (oldSpaceButton == 0)) {
        gotSpaceButton = true;
        anyop += 1;
      }
      if ((doTabButton == 1) and (oldTabButton == 0)) {
        gotTabButton = true;
        anyop += 1;
      }
      anycommand = doLineFeed + 
        doTab + 
        doCrLf;
        if ((doTabButton == 1) and (oldTabButton == 0)) {
          anycommand += 1;
          gotTabButton = true;
        }
        if ((doCRButton == 1) and (oldCRButton == 0)) {
          anycommand += 1;
          gotCRButton = true;
        }
    }
    if (Cstate == Wait) {
      // pick up any straggler selection codes
      doT2 += not digitalRead(SelectT2);
      doT1 += not digitalRead(SelectT1);
      doR2A += not digitalRead(SelectR2A);
      doR1 += not digitalRead(SelectR1);
      doR2 += not digitalRead(SelectR2);
      doR5 += not digitalRead(SelectR5);
      doAux += not digitalRead(SelectAux);
    }
    
    // Manipulate state machines
    if ((Cstate == Idle) and ((anychar > 0) or anyop > 0)) {
      savechar = 0;
      Cstate = Wait;
      Ctime = millis();
      oldSpaceButton = doSpaceButton;
    } else if ((Cstate == Wait) and ((millis()-Ctime)> 5)) {
      Cstate = Grab;
    } else if (Cstate == Grab) {
      // save the triggering commands or characters for processing later
      savechar = convChar();
      if (savechar == '~') {
        savechar = 0;
      }
      saveSpace = doSpace;
      saveBackSpace = doBackSpace;
      saveUpShift = doUpShift;
      saveDownShift = doDownShift;
      Cstate = Trig;
    } else if ((Cstate == Trig) and ((millis()-Ctime) > 43)) {
      Cstate = Break;
    } else if ((Cstate == Break) and ((millis()-Ctime) > 65)) {
      Cstate = Emit;
    } else if (Cstate == Emit) {
      Cstate = Idle;
    }
    if ((Lstate == Idle) and (anycommand>0)) {
      Lstate = Wait;
      Ltime = millis();
      oldTabButton = doTabButton;
      oldCRButton = doCRButton;
      saveLineFeed = doLineFeed;
      saveTab = doTab;
      saveCrLf = doCrLf;
    } else if ((Lstate == Wait) and ((millis()-Ltime)> 5)) {
      Lstate = Trig;
    } else if (Lstate == Trig) {
      if ((saveTab == 1) and (millis()-Ltime) > (calcTab()*1000/40)) {
        Lstate = Break;
      } else if ((saveCrLf == 1) and (Cols >= lmargin) and (millis() - Ltime) > ((Cols - lmargin)*1000/100)) {
        Lstate = Break;
      } else if ((saveCrLf == 1) and (millis() - Ltime) > ((Cols - 0)*1000/100)) {
        Lstate = Break;
      } else if (gotTabButton and (millis() - Ltime) > (calcTab()*1000/40)) {
        Lstate = Break;
      } else if (gotCRButton and (Cols >= lmargin) and (millis() - Ltime) > ((Cols - lmargin)*1000/100)) {
        Lstate = Break;
      } else if (gotCRButton and (millis() - Ltime) > ((Cols - 0)*1000/100)) {
        Lstate = Break;
      } else if ((saveLineFeed == 1) and (millis() - Ltime) > 100) {
        Lstate = Break;
      }
    } else if (Lstate == Break) {
      Lstate = Idle;
    }

    // Write characters if typed
    if (Cstate == Emit){
      if (savechar != 0) {
        value = savechar;
        if (value == 'c') {
          Serial.print("¢");
        } else if (value == 'n') {
          Serial.print("¬");
        } else {
          Serial.print(value);
        }        
        anychar = 0;
        Cols += 1;
      } else if (saveSpace == 1) {
        Serial.print(' ');
        Cols += 1;
       } else if (gotSpaceButton){
        Serial.print(' ');
        gotSpaceButton = false;
        Cols += 1;
       } else if (saveBackSpace == 1) {
        Serial.print('\b');
        if (Cols > lmargin) {
          Cols -= 1;
        }
       } else if (saveUpShift == 1) {
        Caps = 1;
       } else if (saveDownShift == 1) {
        Caps = 0;
       } 
       anyop = 0; 
       saveSpace = 0;
       saveBackSpace = 0;
       saveUpShift = 0;
       savechar = 0; 
       saveDownShift = 0;     
    }
    if (Lstate == Break) {
      int j;
      if (saveLineFeed == 1) {
        Serial.print('\n');
        saveLineFeed = 0;
       } else if (saveCrLf == 1) {
        Serial.print('\r');
        if (Cols < lmargin) {
          Cols = 1;
        } else {
          for (j = 1; j < lmargin; j++) {
          Serial.print(' ');
          } 
          Cols = lmargin;         
        }
        saveCrLf = 0;
       } else if (gotCRButton) {
        Serial.print('\r');
        if (Cols < lmargin) {
          Cols = 1;
        } else {
          for (j = 1; j < lmargin; j++) {
          Serial.print(' ');
          } 
          Cols = lmargin;         
        }
        gotCRButton = false;
       } else if (saveTab == 1) {
        doTabbing();
        saveTab = 0;
       } else if (gotTabButton){
        doTabbing();
        gotTabButton = false;
       } 
      }
      oldTabButton = doTabButton;
      oldSpaceButton = doSpaceButton;
      oldCRButton = doCRButton;

    // Produce output signals
    if (Cstate == Trig) {
      digitalWrite(TwrCBResp,  RLOW);
    } else {
      digitalWrite(TwrCBResp,  RHIGH);
    }
    if (Lstate == Trig) {
      digitalWrite(TwrCRLFTinlk,  RHIGH);
    } else {
      digitalWrite(TwrCRLFTinlk,  RLOW);      
    }
    if (Cols == rmargin) {
      digitalWrite(TwrEOL,  RHIGH);      
    } else {
      digitalWrite(TwrEOL,  RLOW);
    }

    // update tab stops if button pushed
    if (doTabSet == 1) {
      stops[Cols - 1] = 1;
    } else if (doTabClr == 1) {
      stops[Cols - 1] = 0;
    }

    // perform ribbon shifts if commanded
    if ((doBlackShift == 1) and (oldBlackShift == 0)) {
        Serial.print(ESC);
        Serial.print(OPEN);
        Serial.print(BACK);
        Serial.print(END);
        Serial.print(ESC);
        Serial.print(OPEN);
        Serial.print(WHITE);
        Serial.print(END);
        Serial.print(ESC);
        Serial.print(OPEN);
        Serial.print(BLACK);
        Serial.print(END);      
    } else if ((doRedShift == 1) and (oldRedShift == 0)) {
        Serial.print(ESC);
        Serial.print(OPEN);
        Serial.print(BOLD);
        Serial.print(END);
        Serial.print(ESC);
        Serial.print(OPEN);
        Serial.print(RED);
        Serial.print(END);
    }
    oldBlackShift = doBlackShift;
    oldRedShift = doRedShift;

    // display current column on seven segment displays
    updateLED();
  }
}
