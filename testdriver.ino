//
// test driver to check out the PC1053 emulator of the 1053 console printer for an IBM 1130
//    
//    type any valid character on the typeball to cause it to print
//    enter special characters to cause these actions to be commanded:
//      spacebar to drive space
//      \ to backspace
//      ^ to tab
//      ` to feed one line down
//      ~ to do carrier return
//      [ to shift to upper case side of ball
//      ] to shift to lower case side of ball
//      { to shift to black ribbon
//      } to shift to red ribbon

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

void setup() {
  // put your setup code here, to run once:
  pinMode(SelectT1, OUTPUT);
  pinMode(SelectT2, OUTPUT);
  pinMode(SelectR1, OUTPUT);
  pinMode(SelectR2, OUTPUT);
  pinMode(SelectR2A, OUTPUT);
  pinMode(SelectR5, OUTPUT);
  pinMode(SelectAux, OUTPUT);
  pinMode(Space, OUTPUT);
  pinMode(BackSpace, OUTPUT);
  pinMode(BlackShift, OUTPUT);
  pinMode(RedShift, OUTPUT);
  pinMode(UpShift, OUTPUT);
  pinMode(DownShift, OUTPUT);
  pinMode(Tab, OUTPUT);
  pinMode(LineFeed, OUTPUT);
  pinMode(CrLf, OUTPUT);

  digitalWrite(SelectT1, HIGH);
  digitalWrite(SelectT2, HIGH);
  digitalWrite(SelectR1, HIGH);
  digitalWrite(SelectR2, HIGH);
  digitalWrite(SelectR2A, HIGH);
  digitalWrite(SelectR5, HIGH);
  digitalWrite(SelectAux, HIGH);
  digitalWrite(Space, HIGH);
  digitalWrite(BackSpace, HIGH);
  digitalWrite(BlackShift, HIGH);
  digitalWrite(RedShift, HIGH);
  digitalWrite(UpShift, HIGH);
  digitalWrite(DownShift, HIGH);
  digitalWrite(Tab, HIGH);
  digitalWrite(LineFeed, HIGH);
  digitalWrite(CrLf, HIGH);

  Serial.begin(9600);
  Serial.println("Ready for testing");
}

void loop() {
  char character;
  int i;
  int select;
  int doT1, doT2, doR1, doR2, doR2A, doR5;
  
  while (Serial.available() > 0) {
    character = Serial.read();

    if ((character == '\n') or (character == '\r')) {
      continue;
    }

    if (character == '~') {
      Serial.println("Firing CR");
      digitalWrite(CrLf, LOW);
      delay (15);
      digitalWrite(CrLf, HIGH);
      Serial.println(">");
      continue;
    } else if (character == '{') {
      Serial.println("Shifting to black portion of ribbon");
      digitalWrite(BlackShift, LOW);
      delay (15);
      digitalWrite(BlackShift, HIGH);
      Serial.println(">");
      continue;
    } else if (character == '}') {
      Serial.println("Shifting to red portion of ribbon");
      digitalWrite(RedShift, LOW);
      delay (15);
      digitalWrite(RedShift, HIGH);
      Serial.println(">");
      continue;
    } else if (character == '[') {
      Serial.println("Shifting to upper case side of typeball");
      digitalWrite(UpShift, LOW);
      delay (15);
      digitalWrite(UpShift, HIGH);
      Serial.println(">");
      continue;
    } else if (character == ']') {
      Serial.println("Shifting to lower case side of typeball");
      digitalWrite(DownShift, LOW);
      delay (15);
      digitalWrite(DownShift, HIGH);
      Serial.println(">");
      continue;
    } else if (character == '\\') {
      Serial.println("Backspacing one position");
      digitalWrite(BackSpace, LOW);
      delay (15);
      digitalWrite(BackSpace, HIGH);
      Serial.println(">");
      continue;
    } else if (character == ' ') {
      Serial.println("Spacing over one position");
      digitalWrite(Space, LOW);
      delay (15);
      digitalWrite(Space, HIGH);
      Serial.println(">");
      continue;
    } else if (character == '`') {
      Serial.println("Sending line feed");
      digitalWrite(LineFeed, LOW);
      delay (15);
      digitalWrite(LineFeed, HIGH);
      Serial.println(">");
      continue;
    } else if (character == '^') {
      Serial.println("Commanding a tab");
      digitalWrite(Tab, LOW);
      delay (15);
      digitalWrite(Tab, HIGH);
      Serial.println(">");
      continue;
    }

    if (isAlpha(character)) {
      String convert = " ";
      convert[0] = character;
      convert.toUpperCase();
      character = convert[0];
    }
    // try lower case first
    select = -1;
    for (i=0; i<44; i++) {
      if (Lchar[i] == character) {
        select = Cselect[i];
        break;
      }
    }
    if (select == -1) {
      for (i=0; i<44; i++) {
        if (Uchar[i] == character) {
          select = Cselect[i];
          break;
        }
      }
    }

    if (select == -1) {
      Serial.print(character);
      Serial.println(" is not a character on this typeball");
      continue;
    } else {
      Serial.print("Typing "); Serial.println(character);
    }

    // convert to the relevant signals
    doR5 = select & 0x01;
    doR2 = (select & 0x02) / 2;
    doR2A = (select & 0x04) / 4;
    doR1 = (select & 0x08) / 8;
    doT1 = (select & 0x10) / 16;
    doT2 = (select & 0x20) / 32;

    // fire up the tile/rotate and aux signals then wait for 8 ms
    if (doT1 == 1) {
      digitalWrite(SelectT1, LOW);
    }
    if (doT2 == 1) {
      digitalWrite(SelectT2, LOW);
    }
    delay(2);           // CVC temporary test of extreme skew
    if (doR1 == 1) {
      digitalWrite(SelectR1, LOW);
    }
    if (doR2 == 1) {
      digitalWrite(SelectR2, LOW);
    }
    if (doR2A == 1) {
      digitalWrite(SelectR2A, LOW);
    }
    if (doR5 == 1) {
      digitalWrite(SelectR5, LOW);
    }
    digitalWrite(SelectAux, LOW);

    delay(8);

    // switch off signals
    digitalWrite(SelectT1, HIGH);
    digitalWrite(SelectT2, HIGH);
    digitalWrite(SelectR1, HIGH);
    digitalWrite(SelectR2, HIGH);
    digitalWrite(SelectR2A, HIGH);
    digitalWrite(SelectR5, HIGH);
    digitalWrite(SelectAux, HIGH);

    // wait for cycle to complete
    delay(200);

    Serial.println(">");
    
  }
}
