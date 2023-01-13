/*

Error codes:
  - error 200 autohome mislukt (eindeloop boven)
  - error 190 liftMove eindeloop geraakt
  - error 180 liftmove geen geldige waarde als positie of target in liftMove


addresses EEPROM
  - 0 aantal byte locaties voor count
  - 1-9 voorbehouden voor laatste error codes
  - 10+ voorbehouden voor count  

  
etage nummers
  - int etage3 = 4; 
  - int etage2 = 3;
  - int etage1 = 2;
  - int etage0 = 1;
  - int kelder = 0;
  
*/


//toevoegen van libary
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // library lcd scherm
#include <EEPROM.h>             // library eeprom geheugen (permantent geheugen)


//definiëren van pins
#define up 22
#define down 23
#define magnet0 24
#define magnet1 25
#define magnet2 26
#define magnet3 27
#define greenCave 28
#define redIndicator 29
#define green0 30
#define green1 31
#define green2 32
#define green3 33
#define btnCall3 36
#define btnCall2 37
#define btnCall1 38
#define btnCall0 39
#define btnSendDown 40
#define eindeloopBeneden 41
#define eindeloopBoven 42
#define cntLift3 43
#define cntLift2 44
#define cntLift1 45
#define cntLift0 46
#define cntLiftKelder 47
#define cntDeuren 48
#define debugSwitch 49
#define motorbeveiliging 50
#define cntKast 51

bool kastNC = true;
int loopCount = 0;

//active en inactive in plaats van high en low wegens inverse waarde
int active = 0;
int inactive = 1;
int location = 10;                   //address eerste byte van counter in EEPROM
bool isOpen = true;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // lcd aanmaken


void setup() {

  lcd.init();       // initialiseer het LCD scherm
  lcd.backlight();  // zet de backlight aan

  lcd.clear();  // lcd leegmaken
  lcd.setCursor(0, 0);
  lcd.print("ACTIVATING");

  pinMode(up, OUTPUT);  //motor L
  digitalWrite(up, inactive);
  pinMode(down, OUTPUT);  //motor R
  digitalWrite(down, inactive);



  // inputs en outputs benoemen
  pinMode(redIndicator, OUTPUT);
  digitalWrite(redIndicator, inactive);
  pinMode(btnCall3, INPUT_PULLUP);
  pinMode(btnCall2, INPUT_PULLUP);
  pinMode(btnCall1, INPUT_PULLUP);
  pinMode(btnCall0, INPUT_PULLUP);
  pinMode(btnSendDown, INPUT_PULLUP);
  pinMode(cntDeuren, INPUT_PULLUP);
  //pinMode(cntDeur1,INPUT);
  //pinMode(cntDeur2,INPUT);
  //pinMode(cntDeur3,INPUT);
  pinMode(cntLiftKelder, INPUT_PULLUP);
  pinMode(cntLift0, INPUT_PULLUP);
  pinMode(cntLift1, INPUT_PULLUP);
  pinMode(cntLift2, INPUT_PULLUP);
  pinMode(cntLift3, INPUT_PULLUP);
  pinMode(eindeloopBoven, INPUT_PULLUP);
  pinMode(eindeloopBeneden, INPUT_PULLUP);
  pinMode(debugSwitch, INPUT);
  pinMode(motorbeveiliging, INPUT_PULLUP);
  pinMode(cntKast,INPUT);
  pinMode(magnet0, OUTPUT);
  digitalWrite(magnet0, inactive);
  pinMode(magnet1, OUTPUT);
  digitalWrite(magnet1, inactive);
  pinMode(magnet2, OUTPUT);
  digitalWrite(magnet2, inactive);
  pinMode(magnet3, OUTPUT);
  digitalWrite(magnet3, inactive);
  pinMode(green0, OUTPUT);
  digitalWrite(green0, inactive);
  pinMode(green1, OUTPUT);
  digitalWrite(green1, inactive);
  pinMode(green2, OUTPUT);
  digitalWrite(green2, inactive);
  pinMode(green3, OUTPUT);
  digitalWrite(green3, inactive);
  pinMode(greenCave, OUTPUT);
  digitalWrite(greenCave, inactive);

  //pinMode(btnSendDown, INPUT_PULLUP); // Input pullup means that the signal is HIGH if circuit is switched off!
  Serial.begin(57600);

  
  liftStop();
  autohome();



  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("move count:");
  lcd.setCursor(0, 1);
  lcd.print(readEEPROM(location));
  delay(5000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("STARTUP");
  lcd.setCursor(0, 1);
  lcd.print("COMPLETE");
}

void loop(){
  if(digitalRead(motorbeveiliging)){
    int btnIn = controlleerButtons();
    int liftPosition = checkPosition();
    //lcd.clear();
    if(btnIn!=9){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Input:");
      if(btnIn==0){
        lcd.setCursor(14, 0);
      } else{
        lcd.setCursor(15, 0);
      }
      lcd.print(btnIn-1);
    } else{
      lcd.setCursor(0,0);
      lcd.print("No input");
    }
    if(liftPosition!=9){
      lcd.setCursor(0, 1);
      lcd.print("Lift position:");
      if(liftPosition==0){
        lcd.setCursor(14,1);
      } else{
        lcd.setCursor(15, 1);
      }
      lcd.print(liftPosition-1);
    } else{
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("pos unknown");
    }
    
    
    delay(200);
    if(btnIn !=9 && liftPosition != btnIn){
      if(!digitalRead(cntDeuren)){
        closeDoors();
      }
      if(!digitalRead(cntDeuren)){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERROR ");
        lcd.setCursor(0, 1);
        lcd.print("DOOR OPEN");
        delay(3000);
      } else{
        liftMove(btnIn);
        lcd.clear();
      }
    }
//    if((btnIn !=9 && liftPosition != btnIn)&&digitalRead(cntDeuren)){
//      liftMove(btnIn);
//      lcd.clear();
//    }else{
//      if(!digitalRead(cntDeuren)){
//        lcd.clear();
//        lcd.setCursor(0, 0);
//        lcd.print("ERROR ");
//        lcd.setCursor(0, 1);
//        lcd.print("DOOR OPEN");
//        delay(3000);
//      }
//    }
  }else{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR MOTOR");
    lcd.setCursor(0, 1);
    lcd.print("BEVEILIGING");
    delay(3000);
  }
}

bool isKastOpen(){
  isOpen = digitalRead(cntKast);
  if(kastNC){
    if(isOpen){
      return true;
    }else{
      return false;
    }
  }else{
    if(isOpen){
      return false;
    }else{
      return true;
    }
  }
}

int controlleerButtons() {
  int res;
  if (!digitalRead(btnCall3)) {
    res = 4;
  } else if (!digitalRead(btnCall2)) {
    res = 3;
  } else if (!digitalRead(btnCall1)) {
    res = 2;
  } else if (!digitalRead(btnCall0)) {
    res = 1;
  } else if (!digitalRead(btnSendDown)) {
    res = 0;
  } else res = 9;
  return res;
}

int checkPosition() {
  int res;
  if (!digitalRead(cntLift3)) {
    digitalWrite(green3,active);
    digitalWrite(green2,inactive);
    digitalWrite(green1,inactive);
    digitalWrite(green0,inactive);
    digitalWrite(greenCave,inactive);
    res = 4;
  } else if (!digitalRead(cntLift2)) {
    digitalWrite(green2,active);
    digitalWrite(green3,inactive);
    digitalWrite(green1,inactive);
    digitalWrite(green0,inactive);
    digitalWrite(greenCave,inactive);
    res = 3;
  } else if (!digitalRead(cntLift1)) {
    digitalWrite(green1,active);
    digitalWrite(green2,inactive);
    digitalWrite(green3,inactive);
    digitalWrite(green0,inactive);
    digitalWrite(greenCave,inactive);
    res = 2;
  } else if (!digitalRead(cntLift0)) {
    digitalWrite(green0,active);
    digitalWrite(green2,inactive);
    digitalWrite(green1,inactive);
    digitalWrite(green3,inactive);
    digitalWrite(greenCave,inactive);
    res = 1;
  } else if (!digitalRead(cntLiftKelder)) {
    digitalWrite(greenCave,active);
    digitalWrite(green2,inactive);
    digitalWrite(green1,inactive);
    digitalWrite(green0,inactive);
    digitalWrite(green3,inactive);
    res = 0;
  } else res = 9;
  return res;
}


void liftStop(){
  digitalWrite(up,inactive);
  digitalWrite(down,inactive);
  delay(1000);
}

void autohome(){
  lcd.init();
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("autohome");
  lcd.setCursor(0, 1);
  lcd.print("in progress");
  moving(true);
  liftStop();
  while(digitalRead(eindeloopBeneden)){
      digitalWrite(down,active);
  }
  liftStop();
  while(digitalRead(cntLiftKelder)){
    digitalWrite(up,active);
    if(!digitalRead(eindeloopBoven)){
      liftStop();
      break;
      addError(200);
      //error autohome
    }
  }
  liftStop();
  moving(false);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("autohome");
  lcd.setCursor(0, 1);
  lcd.print("complete");
  
}

void liftMove(int target){
  moving(true);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Lift moving");
  liftStop();
  int pos = checkPosition;
  while(pos==9){
    digitalWrite(down,active);
    if(!digitalRead(eindeloopBeneden)){
      liftStop();
      autohome();
      break;
    }
    pos = checkPosition();
  }
  liftStop();
  int lastPos = pos;
  bool richting = true; //true = up & false = down
  while(lastPos != target){
    if(!digitalRead(eindeloopBeneden) || !digitalRead(eindeloopBoven)){
      break;
      liftStop();
      addError(190);
    }
    if(target < lastPos){
      if(richting){
        liftStop();
      }
      digitalWrite(up,inactive);
      digitalWrite(down,active);                            //while target!= lastpos{while< delay;while> delay}
      richting = false;
    }
    else if(target > lastPos){
      if(!richting){
        liftStop();
      }
      digitalWrite(down,inactive);
      digitalWrite(up,active);
      richting = true;
    }
    else{
      liftStop();
      addError(180);
    }
    pos = checkPosition();
    if(pos !=9){
      lastPos = pos;
    }
  }
  liftStop();
  moving(false);

  
}

void moving(bool moving){
  if(moving){
    countUp();
    digitalWrite(redIndicator,active);
    digitalWrite(magnet0,active);
    digitalWrite(magnet1,active);
    digitalWrite(magnet2,active);
    digitalWrite(magnet3,active);
  }
  else{
    digitalWrite(redIndicator,inactive);
    digitalWrite(magnet0,inactive);
    digitalWrite(magnet1,inactive);
    digitalWrite(magnet2,inactive);
    digitalWrite(magnet3,inactive);
  }
  delay(1000);
}

void closeDoors(){
  digitalWrite(magnet0,active);
  digitalWrite(magnet1,active);
  digitalWrite(magnet2,active);
  digitalWrite(magnet3,active);
  delay(2000);
  digitalWrite(magnet0,inactive);
  digitalWrite(magnet1,inactive);
  digitalWrite(magnet2,inactive);
  digitalWrite(magnet3,inactive);
}

void writeEEPROM(int val, int address) {
  int count = 0;
  if (val <= 255) {
    EEPROM.update(address, val);
    EEPROM.update(0, 1);
  } else {
    while (val > 255) {
      EEPROM.update(address, 255);
      val -= 255;
      address++;
      count++;
    }
    EEPROM.update(address, val);
    count++;
    EEPROM.update(0, count);
  }
}


int readEEPROM(int address) {
  int size = EEPROM.read(0);
  int res = 0;
  while (size > 0) {
    res += EEPROM.read(size + address - 1);
    size--;
  }
  return res;
}
  

void countUp() {
  int val = readEEPROM(location);
  writeEEPROM(val + 1, location);
}


void addError(int newError) {
  for (int i = 1; i < 9; i++) {
    EEPROM.update(i, EEPROM.read(i + 1));  //read -> get get.(location,data type)
  }
  EEPROM.write(9, newError);
}
