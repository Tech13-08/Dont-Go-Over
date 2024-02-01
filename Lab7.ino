#include "Timer.h"
#include "pitches.h"

int speakerPin = 13;
int ThermistorPin = A5;
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

int ledPins[] = {9,10,11,12};
int ledAmount[] = {5,2,4,4}; // How many "+1"s, "+2"s, etc are left
int currLed = 0;
bool currLedState = true;
int ledTimeElapsed = 15;

int gSegPins[] = {A2, A3, A4, 2, 3, 4, 5};
int D1 = 6;
int D2 = 7;

const int Ax = A0, Ay=A1, sw=8;
int xValue = 0, yValue = 0, buttonState = 1;

int gameTimeElapsed = 0;
const int maxPlayers = 4;

bool gameRun = false;

int numPlayers = 2;
int playersLeft = 2;
int currentPlayer = 0;
int players[maxPlayers];

int displayNum = numPlayers; // Number that gets displayed on the 4 digit 7 segment display

int targetNumber = 0;
int currentNumber = 0;

bool turnDone = false;

int currentBeat = 0;


unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long currentMillis = 0;

// constants won't change:
const long beat = 250;  // interval at which to blink (milliseconds)

const int numBeats = 32;
int badApple[numBeats] = {
    NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, 0, NOTE_DS5, NOTE_CS5, NOTE_AS4, 0, NOTE_DS4, 0,
    NOTE_AS4, NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_GS4, NOTE_AS4, 0, 
    NOTE_GS4, NOTE_FS4, NOTE_F4, NOTE_DS4, NOTE_F4, NOTE_FS4, NOTE_F4, NOTE_DS4, NOTE_D4, NOTE_F4};

enum gameStates {reset, waitJoyStickY, addPlayers, subtractPlayers, waitRelease, play, waitReset, handleTurn} gameState = reset;
void GameTick(){
  switch(gameState){
    case reset:
      // reset game vars for a new game
      currentBeat = 0;
      displayNum = numPlayers;
      gameState = waitJoyStickY;
      setRandomNumber();
      currentNumber = 0;
      currentPlayer = 0;
      for(int i = 0; i < maxPlayers; ++i){
        players[i] = 0;
      }
      break;
    case waitJoyStickY:
      if(yValue < 400 && numPlayers < maxPlayers){
        numPlayers += 1;
        displayNum = numPlayers;
        gameState = addPlayers;
      }
      else if(yValue > 700 && numPlayers > 2){
        numPlayers -= 1;
        displayNum = numPlayers;
        gameState = subtractPlayers;
      }
      else if(!buttonState){
        for(int i = 0; i < numPlayers; ++i){
          players[i] = 1;
        }
        playersLeft = numPlayers;
        gameState = waitRelease;
      }
      else{
        gameState = waitJoyStickY;
      }
      break;

    case addPlayers:
      if(yValue < 400){
        gameState = addPlayers;
      }
      else{
        gameState = waitJoyStickY;
      }
      break;
    
    case subtractPlayers:
      if(yValue > 700){
        gameState = subtractPlayers;
      }
      else{
        gameState = waitJoyStickY;
      }
      break;

    case waitRelease:
      if(!buttonState){
        gameState = waitRelease;
      }
      else{
        gameState = play;
      }
      break;

    case play:
      gameRun = true;
      displayNum = currentNumber;
      gameState = play;
      if(turnDone){
        gameState = handleTurn;
      }
      if(playersLeft == 1){
        displayNum = currentPlayer+1;
        gameState = waitReset;
        gameRun = false;
        for(int i = 0; i < 5; ++i){
          digitalWrite(ledPins[i], 1);
        }
      }
      break;
    
    case waitReset:
      if (!buttonState){
        displayNum = 0;
        gameState = reset;
      }
      else{
        playSong();
        gameState = waitReset;
      }
      break;
    
    case handleTurn:
      if(currentNumber > targetNumber){
        players[currentPlayer] = 0;
        playersLeft -= 1;
        setRandomNumber();
        currentNumber = 0;
      }
      while(1){
        if(currentPlayer < numPlayers-1){
          currentPlayer += 1;
        }
        else{
          currentPlayer = 0;
        }

        if(players[currentPlayer] == 1){
          break;
        }
      }
      gameState = play;
      turnDone = false;
      break;
  }
  
}

enum ledStates {waitJoyStickX, left, right, press, pressRelease} ledState = waitJoyStickX;
void LedTick(){
  switch(ledState){
    case waitJoyStickX:
      // Game play code for users to select increment options 
      if(xValue < 400){
        ledState = left;
      }
      else if(xValue > 700){
        ledState = right;
      }
      else if(!buttonState){
        if(ledAmount[currLed] == 0){
          ledState = waitJoyStickX;
        }
        else{
          ledState = press;
        }
      }
      else{
        ledState = waitJoyStickX;
      }
      break;

    case left:
      if(xValue > 700){
        ledState = right;
      }
      else if(xValue > 400){
        ledState = waitJoyStickX;
      }
      else{
        ledState = left;
      }
      break;
    
    case right:
      if(xValue < 400){
        ledState = left;
      }
      else if(xValue < 700){
        ledState = waitJoyStickX;
      }
      else{
        ledState = right;
      }
      break;
    
    case press:
      ledState = pressRelease;
      break;
    
    case pressRelease:
      if(!buttonState){
        ledState = pressRelease;
      }
      else{
        ledState = waitJoyStickX;
      }
      break;
    
  }
  switch(ledState){
    case waitJoyStickX:
      for(int i = 0; i < 5; ++i){
        digitalWrite(ledPins[i], ledAmount[i]);
      }
      currLedState = !currLedState;
      digitalWrite(ledPins[currLed], currLedState);
      break;

    case left:
      digitalWrite(ledPins[currLed], ledAmount[currLed]);
      if(currLed > 0){
        currLed -= 1;
      }
      else{
        currLed = 3;
      }
      break;
    
    case right:
      digitalWrite(ledPins[currLed], ledAmount[currLed]);
      if(currLed < 3){
        currLed += 1;
      }
      else{
        currLed = 0;
      }
      break;

    case press:
      switch(currLed){
        case 0:
          currentNumber += 1;
          break;
        case 1:
          currentNumber += 2;
          break;
        case 2:
          currentNumber += 5;
          break;
        case 3:
          currentNumber += 10;
          break;
      }
      ledAmount[currLed] -= 1;
      turnDone = true;
      break;
    
    case pressRelease:
      break;
  }
}

enum segStates {digitOne, digitTwo} segState = digitOne;
void SegTick(){
  switch(segState){
    case digitOne:
      segState = digitTwo;
      break;

    case digitTwo:
      segState = digitOne;
      break;
  }
  switch(segState){
    case digitOne:
      digitalWrite(D2, HIGH);
      displayNumTo7Seg(displayNum % 10, D1);
      break;

    case digitTwo:
      digitalWrite(D1, HIGH);
      displayNumTo7Seg((displayNum-(displayNum % 10))/10, D2);
      break;
  }
}

void displayNumTo7Seg(unsigned int targetNum, int digitPin) {


    // A map of integers to the respective values needed to display
    // a value on one 7 seg digit.
    unsigned char encodeInt[10] = {
        // 0     1     2     3     4     5     6     7     8     9
        0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67,
    };




    // Make sure the target digit is off while updating the segments iteratively
    digitalWrite(digitPin, HIGH);


    // Update the segments
    for (int k = 0; k < 7; ++k) {
        digitalWrite(gSegPins[k], encodeInt[targetNum] & (1 << k));
    }


    // Turn on the digit again
    digitalWrite(digitPin, LOW);


}

void playSong(){
  currentMillis = millis();
  if (currentMillis - previousMillis >= beat) {
    previousMillis = currentMillis;

    tone(speakerPin, badApple[currentBeat], beat);
    
    currentBeat = currentBeat + 1;
    if (currentBeat >= numBeats) {
      currentBeat = 0;
    }
  }
}

void setRandomNumber(){
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  T = (T * 9.0)/ 5.0 + 32.0; 

  randomSeed(T);
  targetNumber = random(20, 100);
  int tempNumber = targetNumber;
  int randomChoice = 0;
  for(int i = 0; i < 5; ++i){
    ledAmount[i] = 1;
  }
  while(tempNumber > 0){
    randomChoice = random(0,4);
    switch(randomChoice){
      case 0:
        tempNumber -= 1;
        ledAmount[0] += (tempNumber % 3) + 1;
        break;
      
      case 1:
        tempNumber -= 2;
        ledAmount[1] += (tempNumber % 4) + 1;
        break;
      
      case 2:
        tempNumber -= 5;
        ledAmount[2] += 1;
        break;
      
      case 3:
        tempNumber -= 10;
        ledAmount[3] += 1;
        break;
    }
  }
  for(int i = 0; i < 5; ++i){
    digitalWrite(ledPins[i], ledAmount[i]);
  }
}

void setup() {
  // put your setup code here, to run once:
  for(int i = 0; i < 5; ++i){
    pinMode(ledPins[i], OUTPUT);
  }
  for(int i = 0; i < 7; ++i){
    pinMode(gSegPins[i], OUTPUT);
  }
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(sw, INPUT_PULLUP);
  pinMode(Ax, INPUT);
  pinMode(Ay, INPUT);
  pinMode(speakerPin, OUTPUT);

  TimerSet(10); 
  TimerOn();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  xValue = analogRead(Ax); 
  yValue = analogRead(Ay); 
  buttonState = digitalRead(sw);

  if(gameTimeElapsed >= 10){
    GameTick();
    gameTimeElapsed = 0;
  }
  if(gameRun){
    if(ledTimeElapsed >= 15){
      LedTick();
      ledTimeElapsed = 0;
    }
    ledTimeElapsed += 1;
  }
  SegTick();
  gameTimeElapsed += 1;
  while(!TimerFlag){}
  TimerFlag = 0;
}
