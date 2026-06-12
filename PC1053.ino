// PC1053 project to emulate an IBM 1053 Console Printer when hooked to an IBM 1130 or IBM 1800 computer system
//
//    Carl V Claunch
//
// repository on Github https://github.com/cclaunch/PC1053
//
// drives an Arduino Mega 2560 hooked to MOSFET switches, 7 segment display hardware, pushbuttons, and SMS style paddle
// connectors that plug into the 1130 or 1800. One of the paddle connectors provides power (ground, +12V and +48V) to
// the 1053, which is an I-O Selectric mechanism based around a Selectric 1 typewriter using solenoids and microswitches
// to interface it to a computer. Commonly used in the 1960s with the 1130, 1800 and as the main console for S/360 systems
//
// uses direct port access which ties this to the specific Arduino model (Mega 2560)
//
// connect via USB serial port at 9600 baud 8 n 1 1 using UTF-8 encoding and ANSI Colors
//
// supports simple commands from the serial terminal
//  LM nnn     
//       sets the column for the left margin
//
//  RM nnn
//       sets the column for the right margin
//
//  TS nnn
//       sets a tab stop at column nnn
//
//  TC nnn
//       clears a tab stop at column nnn
//
//       NOTE for all commands with nnn, a three digit column number with leading zeroes if needed
//
//  TABS =
//    displays all the column numbers where a tab stop is set 
//
//  TYPE t
//    switches between normal and APL typeball  NOT YET SUPPORTED
//    *******   t is either N for normal or A for APL typeball

#include <stdint.h>
#include <stddef.h>

// global defines
#define SelectT2      22
#define SelectT1      24
#define SelectR2A     26
#define SelectR1      28
#define SelectR5      30
#define SelectR2      32
#define SelectAux     34
#define LineFeed      36
#define Tab           38
#define CrLf          40
#define UpShift       42
#define DownShift     44
#define Space         46
#define BackSpace     48
#define BlackShift    50
#define RedShift      52
#define TwrEOL         3
#define TwrCBResp      5
#define TwrEndForms    6
#define TwrCRLFTinlk   7
#define OnesA         23
#define OnesB         25
#define OnesC         27
#define OnesD         29
#define TensA         31
#define TensB         33
#define TensC         35
#define TensD         37
#define HundredsA     39
#define TabButton     41
#define SpaceButton   43
#define CRButton      45
#define TabSet        47
#define TabClr        49

// define the sequences to print for ribbon color control
#define ESC     '\x1b'
#define OPEN    '['
#define BLACK   "30;1"
#define RED     "31;1"
#define WHITEBG "107"
#define BOLD    "1"
#define GREEN   "32;1"
#define END     'm'

// set up the special Zero Width Joiner unicode character
char  ZWJ[]  =  "\u200D ";

// define states for BCD 7seg controllers
//  LOW is LOW
//  HIGH is HIGH

// define states for MOSFET drivers
// HIGH is LOW
// LOW is HIGH

// direct access replacement for digitalRead statements
#define readT2   !(PINA & (1<<0))
#define readT1   !(PINA & (1<<2))
#define readR2A  !(PINA & (1<<4))
#define readR1   !(PINA & (1<<6))
#define readR5   !(PINC & (1<<7))
#define readR2   !(PINC & (1<<5))
#define readAUX  !(PINC & (1<<3))
#define readLF   !(PINC & (1<<1))
#define readTAB  !(PIND & (1<<7))
#define readCRLF !(PING & (1<<1))
#define readUP   !(PINL & (1<<7))
#define readDN   !(PINL & (1<<5))
#define readSP   !(PINL & (1<<3))
#define readBSP  !(PINL & (1<<1))
#define readBLK  !(PINB & (1<<3))
#define readRED  !(PINB & (1<<1))
#define readTBUT !(PING & (1<<0))
#define readPBUT !(PINL & (1<<6))
#define readRBUT !(PINL & (1<<4))
#define readSBUT !(PINL & (1<<2))
#define readCBUT !(PINL & (1<<0))

// direct port access to replace digitalWrite statements
#define highEOL  PORTE |= (1<<5)
#define lowEOL   PORTE &= ~(1<<5)
#define highCBR  PORTE |= (1<<3)
#define lowCBR   PORTE &= ~(1<<3)
#define highEOF  PORTH |= (1<<3)
#define lowEOF   PORTH &= ~(1<<3)
#define highCRLFT PORTH |= (1<<4)
#define lowCRLFT PORTH &= ~(1<<4)
#define onONEA   PORTA |= (1<<1)
#define offONEA  PORTA &= ~(1<<1)
#define onONEB   PORTA |= (1<<3)
#define offONEB  PORTA &= ~(1<<3)
#define onONEC   PORTA |= (1<<5)
#define offONEC  PORTA &= ~(1<<5)
#define onONED   PORTA |= (1<<7)
#define offONED  PORTA &= ~(1<<7)
#define onTENA   PORTC |= (1<<6)
#define offTENA  PORTC &= ~(1<<6)
#define onTENB   PORTC |= (1<<4)
#define offTENB  PORTC &= ~(1<<4)
#define onTENC   PORTC |= (1<<2)
#define offTENC  PORTC &= ~(1<<2)
#define onTEND   PORTC |= (1<<0)
#define offTEND  PORTC &= ~(1<<0)
#define onHUNA   PORTG |= (1<<2)
#define offHUNA  PORTG &= ~(1<<2)

// direct port access to speed up Serial.available()
#define Incoming (UCSR0A & (1 << RXC0))

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
#define Idle  0
#define Wait  1
#define Grab  2
#define Trig  3
#define Break 4
#define Emit  5

// define state machines
int Cstate = Idle;
int Lstate = Idle;
int oldCstate = Idle;
int oldLstate = Idle;

// working variables
int anychar =     0;
int anycommand =  0;
int anyop =       0;
int Caps =        0;
int Cols =        1;
int oldCols =     1;
int lmargin =     1;
int rmargin =   120;
int aplball =     0;
long leap =       0;
long retreat =    0;
long drop =       0;
int RedRibbon =   0;
int hemisphere =  0;

char tempchar[5];

// select code table
// int Cselect[44] = {15, 6,  7, 12, 13,  4,  5,  9,  8, 31,
//                   22, 23, 28, 29, 20, 21, 25, 24, 38, 39,
//                   44, 45, 36, 37, 41, 40, 63, 54, 55, 60,
//                   61, 52, 53, 57, 56, 49, 48, 47, 33, 32,
//                   17, 16, 1, 0};

// inverse select code table, built from above table
int TRselect[64] = {43,42,44,44,5,6,1,2,8,7,
                    44,44,3,4,44,0,41,40,44,44,
                    14,15,10,11,17,16,44,44,12,13,
                    44,9,39,38,44,44,22,23,18,19,
                    25,24,44,44,20,21,44,37,36,35,
                    44,44,31,32,27,28,34,33,44,44,
                    29,30,44,26};
              
// Upper case character table
char Hchar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ(+<¬);*'\"|=_?:>!%¢";
              
// Lower case character table
char Lchar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890#/-,&$@.";

// upper case APL (988 ball) character table
char AHchar[] = "⍺⊥∩⌊ε▁∇∆⍳∘'⎕∣⊤○⋆?⍴⌈∼↓∪⍵⊃↑⊂\"¯>≥\=≤<≠∨∧)\\→;-(÷:";

// lower case APL (988 ball) character table
char ALchar[] = ".HIGFEDBC[QRPONMKLYZXWVUST]8976542301,←/+J×A";

// helper indexes
char * Uindex[44];
char * Lindex[44];
char * AUindex[44];
char * ALindex[44];

// line buffer to support overtyping
// this holds the select code that was typed into a column previously,
int buffer[120];

// lookup table for APL codes that combine to a print a composite glyph
// has three select codes for each, the two that are typed by the 1130
// and the code for the new glyph
int APLlookup[18][3] = { 157, 140, 201,  // or + tilde
                         150, 140, 202,  // and + tilde
                         106, 129, 203,  // del + stile
                         110, 129, 204,  // delta + stile
                         129, 121, 205,  // stile + circle
                         121, 148, 206,  // circle + slope
                         121, 118, 207,  // circle + bar
                         121, 122, 208,  // circle + star
                         107, 130, 209,  // uptack + downtack
                         106, 140, 210,  // del + tilde
                         107, 132, 211,  // uptack + null
                         130, 132, 212,  // downtack + null
                         148, 118, 213,  // slope + bar
                         33,  118, 214,  // slash + bar
                         108, 132, 215,  // cap + null
                         123, 124, 216,  // quote + quad
                         123,  16, 217,  // quote + dot
                         124, 102, 218  // quad + divide
};

char composite[] = "⍱⍲⍒⍋⌽⍉⊖⍟⌶⍫⍎⍕⍀⌿⍝⍞!⌹";

// tab stop table
byte stops[120];

// helper function
// Returns the raw byte index for the N-th character in a UTF-8 array
int getUtf88ByteIndex(const char* str, int targetCharIndex) {
  int byteIdx = 0;
  int charCount = 0;

  while (str[byteIdx] != '\0') {
    if (charCount == targetCharIndex) {
      return byteIdx; // Found the start of our target character
    }
    
    unsigned char b = str[byteIdx];

    // Determine how many bytes this character uses
    if ((b & 0x80) == 0x00)        byteIdx += 1; // 1-byte ASCII
    else if ((b & 0xE0) == 0xC0)   byteIdx += 2; // 2-byte character
    else if ((b & 0xF0) == 0xE0)   byteIdx += 3; // 3-byte character
    else if ((b & 0xF8) == 0xF0)   byteIdx += 4; // 4-byte character
    else                           byteIdx += 1; // Fallback for invalid bytes
    
    charCount++;
  }
  return -1; // Index out of bounds
}

// Copies the UTF-8 character pointed to by input buffer into the output buffer
bool copyChar(const char* startByte, char* destBuffer, int bufferSize) {

  unsigned char b = *startByte;
  int len = 1;
  // Identify sequence length from the leading byte
  if ((b & 0xE0) == 0xC0) len = 2;
  else if ((b & 0xF0) == 0xE0) len = 3;
  else if ((b & 0xF8) == 0xF0) len = 4;
  if (len + 1 > bufferSize) return false; // Buffer too small

  // Copy sequence and null-terminate
  memcpy(destBuffer, startByte, len);
  destBuffer[len] = '\0';
  return true;
}

// initial setup code
void setup() {
  int i;
  
  // set up output pins

  pinMode(TwrEOL,       OUTPUT);
  pinMode(TwrCBResp,    OUTPUT);
  pinMode(TwrEndForms,  OUTPUT);
  pinMode(TwrCRLFTinlk, OUTPUT);
  pinMode(OnesA,        OUTPUT);
  pinMode(OnesB,        OUTPUT);
  pinMode(OnesC,        OUTPUT);
  pinMode(OnesD,        OUTPUT);
  pinMode(TensA,        OUTPUT);
  pinMode(TensB,        OUTPUT);
  pinMode(TensC,        OUTPUT);
  pinMode(TensD,        OUTPUT);
  pinMode(HundredsA,    OUTPUT);

  // set output pins to initial state
  lowEOL;
  highCBR;
  lowEOF;
  lowCRLFT;
  onONEA;
  offONEB;
  offONEC;
  offONED;
  offTENA;
  offTENB;
  offTENC;
  offTEND;
  offHUNA;

  // set up input pins
  pinMode(SelectT2,     INPUT_PULLUP);
  pinMode(SelectT1,     INPUT_PULLUP);
  pinMode(SelectR2A,    INPUT_PULLUP);
  pinMode(SelectR1,     INPUT_PULLUP);
  pinMode(SelectR5,     INPUT_PULLUP);
  pinMode(SelectR2,     INPUT_PULLUP);
  pinMode(SelectAux,    INPUT_PULLUP);
  pinMode(LineFeed,     INPUT_PULLUP);
  pinMode(Tab,          INPUT_PULLUP);
  pinMode(CrLf,         INPUT_PULLUP);
  pinMode(UpShift,      INPUT_PULLUP);
  pinMode(DownShift,    INPUT_PULLUP);
  pinMode(Space,        INPUT_PULLUP);
  pinMode(BackSpace,    INPUT_PULLUP);
  pinMode(BlackShift,   INPUT_PULLUP);
  pinMode(RedShift,     INPUT_PULLUP);
  pinMode(TabButton,    INPUT_PULLUP);
  pinMode(SpaceButton,  INPUT_PULLUP);
  pinMode(CRButton,     INPUT_PULLUP);
  pinMode(TabClr,       INPUT_PULLUP);
  pinMode(TabSet,       INPUT_PULLUP);

  // initialize typewriter with no tab stops
  for (i=0; i < 119; i++) {
    stops[i] = 0;
  }

  // set up ribbon shift old state as off
  oldBlackShift = 0;
  oldRedShift =   0;
 
  // set up three faceplate buttons old state as off
  oldTabButton =    0;
  oldSpaceButton =  0;
  oldCRButton =     0;

  // start communications with PC
  Serial.begin(9600);

  // set terminal to green ribbon for startup message
  Serial.print(ESC);
  Serial.print(OPEN);
  Serial.print(BOLD);
  Serial.print(END);
  Serial.print(ESC);
  Serial.print(OPEN);
  Serial.print(WHITEBG);
  Serial.print(END);
  Serial.print(ESC);
  Serial.print(OPEN);
  Serial.print(GREEN);
  Serial.print(END);
  Serial.println("");
  RedRibbon = 0;

  // startup hello message
  Serial.println("1053 Emulator is active");
     
  // Turn off End of Forms lamp and make typewriter ready
  //   signal -TWR END OF FORMS thus not asserted is high
  highEOF;


  // build ptrs to each UTF-8 char in the tables
  for (i=0; i < 44; i++) {
    Uindex[i] = Hchar+getUtf88ByteIndex(Hchar,i);
  }
  for (i=0; i < 44; i++) {
    Lindex[i] = Lchar+getUtf88ByteIndex(Lchar,i);
  }
  for (i=0; i < 44; i++) {
    AUindex[i] = AHchar+getUtf88ByteIndex(AHchar,i);
  }
  for (i=0; i < 44; i++) {
    ALindex[i] = ALchar+getUtf88ByteIndex(ALchar,i);
  }

  // clear out line buffer
  zerobuffer();      

  // send menu of commands to user
  Serial.println("Available commands are:");
  Serial.println("     LM xxx            set left margin at column xxx");
  Serial.println("     RM xxx            set right margin at column xxx");
  Serial.println("     TS xxx            set tab at column xxx");
  Serial.println("     TC xxx            clear tab at column xxx");
  Serial.println("     TABS =            display columns where tabs are set");
  Serial.println("     TYPE *            set typeball to * :");
  Serial.println("          N            Normal console typeball (969)");
  Serial.println("          A            APL typeball (988)");

  // ribbon back to black
  Serial.print(ESC);
  Serial.print(OPEN);
  Serial.print(BLACK);
  Serial.print(END);      

} // end of Setup()

// routine to convert Selectric selection code to
// ASCII character corresponding to typeball
void convChar(int select, int hemisphere) {
  int i;

  // look up character from Tilt-Rotate table
  int lookedup;
  lookedup = TRselect[select];

  // if it doesn't match a valid position, this is an invalid select code
  if (lookedup == 44) {
    tempchar[0]='^';
    tempchar[1]= 0;
    return;
  }

  // used lookedup to grab character from table
  if (hemisphere == 1) {  // upper case hemisphere
    if (aplball == 0) {
      if (copyChar(Uindex[lookedup], tempchar, 5)) {
      } else {
        tempchar[0]='^';
        tempchar[1]= 0;
        return;
      }
      return;    
    } else {
      if (copyChar(AUindex[lookedup], tempchar, 5)) {
      } else {
        tempchar[0]='^';
        tempchar[1]= 0;
        return;
      }
      return;
    }
  } else {          // lower case hemisphere
    if (aplball == 0) {
      if (copyChar(Lindex[lookedup], tempchar, 5)) {
      } else {
        tempchar[0]='^';
        tempchar[1]= 0;
        return;
      }
      return;
    } else {
      if (copyChar(ALindex[lookedup], tempchar, 5)) {
      } else {
        tempchar[0]='^';
        tempchar[1]= 0;
        return;
      }
      return;
    }
  }
  
} // end convChar()

// this zeroes out the line buffer when we move to a new line
void zerobuffer() {
  memset(buffer, 0, sizeof(buffer));
  return;
}

// this function adds a character to a given column of the line buffer
// we add 1 to the code so that we can represent empty with a zero
void addbuffer() {
  int select;
  int i;
  int savebuffer;
  int saveselect;

  select = 0;
  if (doT2 > 0)  bitSet(select,5);
  if (doT1 > 0)  bitSet(select,4);
  if (doR1 > 0)  bitSet(select,3);
  if (doR2A > 0) bitSet(select,2);
  if (doR2 > 0)  bitSet(select,1);
  if (doR5 > 0)  bitSet(select,0);

  // first adjust by which hemisphere
  if (Caps == 1) {
    saveselect = select+101;
  } else {
    saveselect = select+1;
  }

  // save the select code if nothing was previously typed 
  // in this column
  savebuffer = buffer[Cols];
  if (savebuffer == 0) {
    buffer[Cols] = saveselect;
    return;
  } else {
    // the apl typeball 988
    if (aplball == 1) {
      // See if we have a pair of select codes that match
      // an APL composite character. If so, indicate this
      for (i=0;i<18;i++){
        // do we have a pair from the table?
        if (((APLlookup[i][0] == savebuffer) and (APLlookup[i][1] == saveselect)) 
            or ((APLlookup[i][1] == savebuffer) and (APLlookup[i][0] == saveselect))) {
          buffer[Cols] = APLlookup[i][2];
          return;
        }
      }
      // we did not match a pair, just fall through
    }
    // save the character for printing
    buffer[Cols] = saveselect;
    return;
  }
}

// this function types the select codes in a column
// the codes are +1 so they need to be adjusted first
void typebuffer() {
  int i;
  int selectcode;
  int whichchar;

  selectcode = buffer[Cols];
  if (selectcode > 200) {
    selectcode -=200;
    // grab the composite character
    whichchar = getUtf88ByteIndex(composite, selectcode-1);
    copyChar(composite+whichchar, tempchar, 5);
    hemisphere = 1; // we pretend composites are upper case
    Serial.print(tempchar);
    return;
  } else if (selectcode > 100) {
    hemisphere = 1;
    selectcode -= 100;
  } else {
    hemisphere = 0;
  }
  if (selectcode == 0) return;
  convChar(selectcode-1, hemisphere);
  Serial.print(tempchar);
  return;
}
// routine to quickly identify the next column with a tab stop set
int find_nonzero_fast(const unsigned char *arr, size_t size) {
    size_t i = 0;
    
    // 1. Align pointer to 8-byte boundary if necessary
    while (i < size && ((uintptr_t)&arr[i] % sizeof(uint64_t) != 0)) {
        if (arr[i] != 0) return i;
        i++;
    }
    
    // 2. Scan 8 bytes at a time
    while (i + sizeof(uint64_t) <= size) {
        if (*(const uint64_t *)&arr[i] != 0) {
            // A non-zero byte exists in this 8-byte chunk; locate it
            for (size_t j = 0; j < sizeof(uint64_t); j++) {
                if (arr[i + j] != 0) return i + j;
            }
        }
        i += sizeof(uint64_t);
    }
    
    // 3. Clean up remaining trailing bytes
    while (i < size) {
        if (arr[i] != 0) return i;
        i++;
    }
    
    return -1;
}

// figure out distance to tab and print spaces for that length
void doTabbing() {
  int i;
  int j;
  i = find_nonzero_fast(stops+Cols, rmargin-Cols);
  if (i == -1) {   // no more tabs set, go to margin and force CR-LF
    Cols = rmargin;
    return;
  }
  // move the cursor to the spot
  for (int j = 0; j < i+1; j++) {
        // if no previous char, print space
        if (buffer[Cols] == 0){
          Serial.print(' ');
        } else {
          // if there was, show it instead
          typebuffer();
        }
        Cols += 1;
  }
  updateLED();
  return;

} // end doTabbing()

// update the tab stops based on command from PC
void updateStops(int mypos, byte val) {
  if (mypos == 0) {
    return;
  }
  stops[mypos - 1] = val;
  return;
} // end updateStops()

// return distance to tab for timing calculation
int calcTab() {
  int i;
  i = find_nonzero_fast(stops+Cols, rmargin-Cols);
  if (i == -1) {   // no more tabs set, go to margin and force CR-LF
     Cols = rmargin;
     return 1;
  }
  if (i == 0) {
    return 1;
  }
  return i+1;
} // end calcTab()

// extracts column number from margin command
int getcolumn(char * command, int margin) {
  int total;
  if (isDigit(command[3]) and isDigit(command[4]) and isDigit(command[5])) {
    total = (int(command[3]) - 48) * 100;
    total += (int(command[4])-48) * 10;
    total += int(command[5]) - 48;
    if (( total <= 0) or (total > 120)) {
      Serial.print("Column number out of range - "); Serial.println(total);
      return margin;
    }
  } else {
    Serial.println("Not a valid column number, ignoring command");
    return margin;
  }
  return total;
} // end getcolumn()

// sets signals for the 74LS48 chips that drive the 7 segment LED displays
// accepts binary coded decimal input (BCD) for each digit
// this displays the current global Cols variable value which is the active column
void updateLED() {
  int ColTens;
  int ColOnes;

  if (Cols > 99) {
    onHUNA;
  } else {
    offHUNA;
  }
  ColOnes = Cols % 10;
  if (ColOnes == 0) {
    offONEA;
    offONEB;
    offONEC;
    offONED;
  } else if (ColOnes == 1) {
    onONEA;
    offONEB;
    offONEC;
    offONED;
  } else if (ColOnes == 2) {
    offONEA;
    onONEB;
    offONEC;
    offONED;
  } else if (ColOnes == 3) {
    onONEA;
    onONEB;
    offONEC;
    offONED;
  } else if (ColOnes == 4) {
    offONEA;
    offONEB;
    onONEC;
    offONED;
  } else if (ColOnes == 5) {
    onONEA;
    offONEB;
    onONEC;
    offONED;
  } else if (ColOnes == 6) {
    offONEA;
    onONEB;
    onONEC;
    offONED;
  } else if (ColOnes == 7) {
    onONEA;
    onONEB;
    onONEC;
    offONED;
  } else if (ColOnes == 8) {
    offONEA;
    offONEB;
    offONEC;
    onONED;
  } else {   // ColOnes == 9
    onONEA;
    offONEB;
    offONEC;
    onONED;
  }
  ColTens = (Cols / 10) % 10;
  if (ColTens == 0) {
    offTENA;
    offTENB;
    offTENC;
    offTEND;
  } else if (ColTens == 1) {
    onTENA;
    offTENB;
    offTENC;
    offTEND;
  } else if (ColTens == 2) {
    offTENA;
    onTENB;
    offTENC;
    offTEND;
  } else if (ColTens == 3) {
    onTENA;
    onTENB;
    offTENC;
    offTEND;
  } else if (ColTens == 4) {
    offTENA;
    offTENB;
    onTENC;
    offTEND;
  } else if (ColTens == 5) {
    onTENA;
    offTENB;
    onTENC;
    offTEND;
  } else if (ColTens == 6) {
    offTENA;
    onTENB;
    onTENC;
    offTEND;
  } else if (ColTens == 7) {
    onTENA;
    onTENB;
    onTENC;
    offTEND;
  } else if (ColTens == 8) {
    offTENA;
    offTENB;
    offTENC;
    onTEND;
  } else {   // ColTens == 9
    onTENA;
    offTENB;
    offTENC;
    onTEND;
  }
  return;  
} // end updateLED()


// main processing loop
void loop() {
//  char value;
  int ColTens;
  int ColOnes;
  char command[7] =     {0,0,0,0,0,0,0};
  int cpointer =        0;
  int tabcol;
  bool gotTabButton =   false;
  bool gotCRButton =    false;
  bool gotSpaceButton = false;
  
  while (1) {

    // first do a quick check for incoming command from PC
    // must be one of these these commands exactly
    //  LM xxx     
    //  RM xxx
    //  TS xxx
    //  TC xxx
    //  TABS =
    //  TYPE t
    //           xxx is three digit column number with leading zeroes if needed
    //           t is either N for normal or A for APL typeball
    while (Serial.available() > 0) {      // data available from serial port
      int j;
      command[cpointer++] = Serial.read();
      if (cpointer > 6) {
        Serial.print(ESC);
        Serial.print(OPEN);
        Serial.print(GREEN);
        Serial.print(END);
        Serial.print("input command ");
        Serial.print(command[0]);
        Serial.print(command[1]);
        Serial.print(command[2]);
        Serial.println(" too long, ignored");
        Serial.print(ESC);
        Serial.print(OPEN);
        if (RedRibbon == 1) {
          Serial.print(RED);
        } else {
          Serial.print(BLACK);
        }
        Serial.print(END);
        cpointer = 0;
      }
      if ((command [cpointer-1] == '\r') or (command[cpointer-1] == '\n')) {
        if (cpointer > 1) {
          Serial.print(ESC);
          Serial.print(OPEN);
          Serial.print(GREEN);
          Serial.print(END);
          Serial.print("short command ");
          Serial.println(" is ignored");
          Serial.print(ESC);
          Serial.print(OPEN);
          if (RedRibbon == 1) {
            Serial.print(RED);
          } else {
            Serial.print(BLACK);
          }
          Serial.print(END);
        }
        cpointer = 0;
      }
      if (cpointer == 6) {
        // command is complete, process it
        if ((command[0] == 'L') and (command[1] == 'M') and (command[2] == ' ')){
          tabcol = getcolumn(command,lmargin);
          if (tabcol > 0) {
            lmargin = tabcol;
            Serial.print(ESC);
            Serial.print(OPEN);
            Serial.print(GREEN);
            Serial.print(END);
            Serial.print("Left Margin set to "); Serial.println(lmargin);
            Serial.print('\r');
            Serial.print('\n');
            Serial.print(ESC);
            Serial.print(OPEN);
            if (RedRibbon == 1) {
              Serial.print(RED);
            } else {
              Serial.print(BLACK);
            }
            Serial.print(END);
            for (j = 1; j < lmargin; j++) {
              Serial.print(' ');
            }
            Cols = lmargin;         
          }   
        } else if ((command[0] == 'R') and (command[1] == 'M') and (command[2] == ' ')){
          tabcol = getcolumn(command,rmargin);
          if (tabcol > 0){
            rmargin = tabcol;
            Serial.print(ESC);
            Serial.print(OPEN);
            Serial.print(GREEN);
            Serial.print(END);
            Serial.print("Right Margin set to "); Serial.println(rmargin);
            Serial.print(ESC);
            Serial.print(OPEN);
            if (RedRibbon == 1) {
              Serial.print(RED);
            } else {
              Serial.print(BLACK);
            }
            Serial.print(END);
          }
        } else if ((command[0] == 'T') and (command[1] == 'Y') and (command[2] == 'P') and (command[3] == 'E') and (command[4] == ' ')){
          // set typeball to APL if TYPE A otherwise to normal if TYPE N
          if (command[5] == 'N') {
            aplball = 0;
            Serial.print(ESC);
            Serial.print(OPEN);
            Serial.print(GREEN);
            Serial.print(END);
            Serial.println("Using normal 969 typeball for 1053");
            Serial.print(ESC);
            Serial.print(OPEN);
            if (RedRibbon == 1) {
              Serial.print(RED);
            } else {
              Serial.print(BLACK);
            }
            Serial.print(END);
            zerobuffer();
            Cols = lmargin;
          } else if (command[5] == 'A') {
            aplball = 1;
            Serial.print(ESC);
            Serial.print(OPEN);
            Serial.print(GREEN);
            Serial.print(END);
            Serial.println("Using APL 988 typeball");
            Serial.print(ESC);
            Serial.print(OPEN);
            if (RedRibbon == 1) {
              Serial.print(RED);
            } else {
              Serial.print(BLACK);
            }
            Serial.print(END);
            zerobuffer();
            Cols = lmargin;         
          } else {
            Serial.print(ESC);
            Serial.print(OPEN);
            Serial.print(GREEN);
            Serial.print(END);
            Serial.print(command[5]);
            Serial.println(" is not a valid typeball");
            Serial.print(ESC);
            Serial.print(OPEN);
            if (RedRibbon == 1) {
              Serial.print(RED);
            } else {
              Serial.print(BLACK);
            }
            Serial.print(END);
          }
        } else if ((command[0] == 'T') and (command[1] == 'A') and (command[2] == 'B') and (command[3] == 'S') and (command[4] == ' ') and (command[5] == '=')){
          Serial.print(ESC);
          Serial.print(OPEN);
          Serial.print(GREEN);
          Serial.print(END);
          Serial.print("Tab stops are set at columns");
          int i;
          for (i = 0; i < 120; i++) {
            if (stops[i] == 1) {
              Serial.print(" "); Serial.print(i+1); 
            }
          }
          Serial.println('\n');
          Serial.print(ESC);
          Serial.print(OPEN);
          if (RedRibbon == 1) {
            Serial.print(RED);
          } else {
            Serial.print(BLACK);
          }
          Serial.print(END);
        } else if ((command[0] == 'T') and (command[1] == 'S') and (command[2] == ' ')){
          tabcol = getcolumn(command,0);
          if (tabcol > 0) {
            Serial.print(ESC);
            Serial.print(OPEN);
            Serial.print(GREEN);
            Serial.print(END);
            Serial.print("Tab set at column "); Serial.println(tabcol);
            Serial.print(ESC);
            Serial.print(OPEN);
            if (RedRibbon == 1) {
              Serial.print(RED);
            } else {
              Serial.print(BLACK);
            }
            Serial.print(END);
            updateStops(tabcol, 1);
          }
        } else if ((command[0] == 'T') and (command[1] == 'C') and (command[2] == ' ')){
          tabcol = getcolumn(command,0);
          if (tabcol > 0) {
            Serial.print(ESC);
            Serial.print(OPEN);
            Serial.print(GREEN);
            Serial.print(END);
            Serial.print("Tab cleared at column "); Serial.println(tabcol);
            Serial.print(ESC);
            Serial.print(OPEN);
            if (RedRibbon == 1) {
              Serial.print(RED);
            } else {
              Serial.print(BLACK);
            }
            Serial.print(END);
            updateStops(tabcol, 0);
          }
        } else {
          Serial.print(ESC);
          Serial.print(OPEN);
          Serial.print(GREEN);
          Serial.print(END);
          Serial.print(command[0]);
          Serial.print(command[1]);
          Serial.print(command[2]);
          Serial.println(" is not a valid command");
          Serial.print(ESC);
          Serial.print(OPEN);
          if (RedRibbon == 1) {
            Serial.print(RED);
          } else {
            Serial.print(BLACK);
          }
          Serial.print(END);
        }
        cpointer = 0;
      }
    }
    // emit bell sound if trying to pass the right margin and back up
    if (Cols > rmargin) {
      Serial.print('\a');
      Serial.print('\b');
      Cols = rmargin;
    }

    // we check for commands from 1130 system if idle
    if ((Cstate == Idle) and (Lstate == Idle)) {
      doT2 =          readT2;
      doT1 =          readT1;
      doR2A =         readR2A;
      doR1 =          readR1;
      doR2 =          readR2;
      doR5 =          readR5;
      doAux =         readAUX;
      doLineFeed =    readLF;
      doSpace =       readSP;
      doBackSpace =   readBSP;
      doTab =         readTAB;
      doCrLf =        readCRLF;
      doUpShift =     readUP;
      doDownShift =   readDN;
      doSpaceButton = readPBUT;
      doTabButton =   readTBUT;
      doCRButton =    readRBUT;
      doTabSet =      readSBUT;
      doTabClr =      readCBUT;

      // process ribbon shifts and make sure we do them just once while signal asserted
      doBlackShift = readBLK;
      if (doBlackShift == 0) {
        oldBlackShift = 0;
      }
      doRedShift = readRED;
      if (doRedShift == 0) {
        oldRedShift = 0;
      }

      // this is a request to type a character
      anychar = doT1 + doT2 + doR2A + doR2 + doR1 + 
        doR5 + doAux;

      // this will be a print cycle without printing a character
      anyop = doTab + 
        doSpace + doBackSpace +
        doUpShift + doDownShift;

      // pushbutton on face of emulator requesting space
      if ((doSpaceButton == 1) and (oldSpaceButton == 0)) {
        gotSpaceButton = true;
        anyop += 1;
      }

      // we will process a long movement command
      //   (tab, linefeed or carrier return)
      anycommand = doLineFeed + 
        doTab + 
        doCrLf;

      // Return button on emulator requesting return-line feed
      if ((doCRButton == 1) and (oldCRButton == 0)) {
        anycommand += 1;
        gotCRButton = true;
      }

      // Tab button on emulator requesting move to next tab stop
      if ((doTabButton == 1) and (oldTabButton == 0)) {
        gotTabButton = true;
        anyop += 1;
        anycommand += 1;
      }
    }

    // look again in case some signals arrive after the
    // initial character selection signals trigger a print
    if ((Cstate == Wait) and ((millis()-Ctime)> 4)) {
      // pick up any straggler selection codes
      doT2 =   readT2;
      doT1 =   readT1;
      doR2A =  readR2A;
      doR1 =   readR1;
      doR2 =   readR2;
      doR5 =   readR5;
      doAux =  readAUX;
      anychar = doT1 + doT2 + doR2A + doR2 + doR1 + 
        doR5 + doAux;
    }
    

    // Manipulate state machines, Cstate first
    if ((Cstate == Idle) and ((anychar > 0) or anyop > 0)) {
      Cstate = Wait;
      Ctime = millis();
      oldSpaceButton = doSpaceButton;
    } else if ((Cstate == Wait) and ((millis()-Ctime)> 5)) {
      // save the triggering commands or characters for processing later
      // AUX will be on for any real print selection, otherwise it is a dummy cycle
      if (doAux == 0) {
        tempchar[0]='^';
        tempchar[1]=0;
      } else {
        addbuffer(); 
        tempchar[0] = 0;
      }
      saveSpace =     doSpace;
      saveBackSpace = doBackSpace;
      saveUpShift =   doUpShift;
      saveDownShift = doDownShift;
      Cstate = Trig;
      lowCBR;
    } else if ((Cstate == Trig) and ((millis()-Ctime) > 43)) {
      Cstate = Break;
    } else if ((Cstate == Break) and ((millis()-Ctime) > 65)) {
      Cstate = Emit;
    } else if (Cstate == Emit) {
      Cstate = Idle;
    }

    // process Lstate for long movement operations
    if ((Lstate == Idle) and (anycommand>0)) {
      Lstate = Wait;
      Ltime = millis();
      oldTabButton =  doTabButton;
      oldCRButton =   doCRButton;
      saveLineFeed =  doLineFeed;
      saveTab =       doTab;
      saveCrLf =      doCrLf;
      if ((doTab > 0) or (doTabButton > 0)) {
        leap = calcTab()*3+100;
      }
      if ((doCrLf > 0) or (doCRButton > 0)) {
        retreat = (Cols - lmargin)*3 + 30;
      }
      if (doLineFeed > 0){
        drop = 160;
      }
    } else if ((Lstate == Wait) and ((millis()-Ltime)> 5)) {
      Lstate = Trig;
    } else if (Lstate == Trig) {
      if ((saveTab == 1) and (millis()-Ltime) > (leap)) {
        Lstate = Break;
      } else if ((saveCrLf == 1) and (millis() - Ltime) > (retreat)) {
        Lstate = Break;
      } else if (gotTabButton and (millis() - Ltime) > (leap)) {
        Lstate = Break;
      } else if (gotCRButton and (millis() - Ltime) > (retreat)) {
        Lstate = Break;
      } else if ((saveLineFeed == 1) and (millis() - Ltime) > drop) {
        Lstate = Break;
      }
    } else if (Lstate == Break) {
      Lstate = Idle;
    }

    // Write characters if typed
    if ((Cstate == Emit) and (oldCstate == Break)) {
      if (tempchar[0] != '^') {
        typebuffer();
        Cols += 1;
        anychar = 0;
      } else if (saveSpace == 1) {
        // if no previous char, print space
        if (buffer[Cols] == 0){
          Serial.print(' ');
        } else {
          // if there was, show it instead
          typebuffer();
        }
        Cols += 1;
       } else if (gotSpaceButton){
        gotSpaceButton = false;
        Cols += 1;
        // if no previous char, print space
        if (buffer[Cols] == 0){
          Serial.print(' ');
        } else {
          // if there was, show it instead
          typebuffer();
        }
       } else if (saveBackSpace == 1) {
        if (Cols > lmargin) {
          Serial.print('\b');
          Cols -= 1;
        }
       } else if (saveUpShift == 1) {
        Caps = 1;
       } else if (saveDownShift == 1) {
        Caps = 0;
       } 
       anyop =          0; 
       saveSpace =      0;
       saveBackSpace =  0;
       saveUpShift =    0;
       saveDownShift =  0;     
    }
    
    // process long actions now
    if ((Lstate == Break) and (oldLstate == Trig)) {
      int j;
      if (saveLineFeed == 1) {
        Serial.print('\n');
        saveLineFeed = 0;
        zerobuffer(); 
       } else if ((saveCrLf == 1) or gotCRButton) {
        Serial.print('\r');
        Serial.print('\n');
        for (j = 1; j < lmargin; j++) {
          Serial.print(' ');
        } 
        zerobuffer(); 
        Cols = lmargin;         
        saveCrLf = 0;
        if (gotCRButton) gotCRButton = false;
       } else if ((saveTab == 1) or gotTabButton) {
        doTabbing();
        saveTab = 0;
        if (gotTabButton) gotTabButton = false;
       } 
      }
      oldTabButton =    doTabButton;
      oldSpaceButton =  doSpaceButton;
      oldCRButton =     doCRButton;

    // Produce output signals
    if ((Cstate == Break) and (oldCstate == Trig)) {
      highCBR;
    }
    if ((Lstate == Trig) and (oldLstate == Wait)) {
      highCRLFT;
    }
    if (Lstate == Break) {
      lowCRLFT;
    }
    if (Cols >= rmargin) {
      highEOL;
    } else {
      lowEOL;
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
        Serial.print(BLACK);
        Serial.print(END);      
        RedRibbon = 0;
    } else if ((doRedShift == 1) and (oldRedShift == 0)) {
        Serial.print(ESC);
        Serial.print(OPEN);
        Serial.print(RED);
        Serial.print(END);
        RedRibbon = 1;
    }
    oldBlackShift = doBlackShift;
    oldRedShift =   doRedShift;
    oldLstate =     Lstate;
    oldCstate =     Cstate;

    // display current column on seven segment displays
    if (Cols != oldCols) updateLED();
    oldCols = Cols;
  }
} // end loop()
