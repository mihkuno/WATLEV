#include "./mp3tf16p.h"

#define SPKRX     10
#define SPKTX     11
#define POTPIN    A0
#define TRIGPIN   7
#define ECHOPIN   6
#define REDPIN    5
#define ORANGEPIN 4
#define YELLOWPIN 3
#define WHITEPIN  2

MP3Player mp3(SPKRX, SPKTX);

unsigned long prevLedMillis = 0;
unsigned long prevSoundMillis = 0;

bool isSoundPlaying = false;
int soundCount = 0;
int ledState = LOW;
int currLevel = 0;
int prevLevel = 0;
int volume = 25;

float getDistance() {
  digitalWrite(TRIGPIN, LOW);  
  delayMicroseconds(2);  
  digitalWrite(TRIGPIN, HIGH);  
  delayMicroseconds(10);  
  digitalWrite(TRIGPIN, LOW);  

  const int   duration = pulseIn(ECHOPIN, HIGH);  
  const float distance = ((duration*.0343)/2 /*cm*/) / 2.54 /*in*/; 

  return distance;
}


void offLight() {
  digitalWrite(REDPIN, LOW);
  digitalWrite(ORANGEPIN, LOW);
  digitalWrite(YELLOWPIN, LOW);
  digitalWrite(WHITEPIN, LOW);
}

void updateVolume() {
  const int potval = analogRead(POTPIN);
  volume = map(potval, 0, 1020, 0, 30);
  mp3.player.volume(volume);
}

void blinkLight(int ledPin = 0) {

    const unsigned long currLedMillis = millis();
    const unsigned int  interval = 500; 

    if (currLedMillis - prevLedMillis >= interval) {
        prevLedMillis = currLedMillis;
    
        ledState = !ledState;
        
        offLight();

        digitalWrite(WHITEPIN, ledState);
        if (ledPin != 0) {
          digitalWrite(ledPin, ledState);
        }
    }
}


void setup() {
  
  Serial.begin(9600);
  pinMode(POTPIN,    INPUT);
  pinMode(TRIGPIN,   OUTPUT);
  pinMode(ECHOPIN,   INPUT);
  pinMode(REDPIN,    OUTPUT);
  pinMode(ORANGEPIN, OUTPUT);
  pinMode(YELLOWPIN, OUTPUT);
  pinMode(WHITEPIN,  OUTPUT);

  mp3.initialize();
  mp3.player.EQ(DFPLAYER_EQ_BASS);
  mp3.player.volume(volume);
}



void loop() {

    updateVolume();

    const float distance = getDistance();
    const bool isTrigLevel3 = distance < 2;
    const bool isTrigLevel2 = distance > 2.5 && distance < 5;
    const bool isTrigLevel1 = distance > 5.5 && distance < 8;
    const bool isTrigDetect = distance < 9.5;
    const bool isTrigHaultWarn  = distance > 8.5;


    if (isTrigDetect) {

        if (isTrigHaultWarn) {
           currLevel = 0;
        }
        else if (isTrigLevel1) {
           currLevel = 1;
        }
        else if (isTrigLevel2) {
           currLevel = 2;
        }
        else if (isTrigLevel3) {
           currLevel = 3;
        }
    
        if (currLevel == 1) {
           blinkLight(YELLOWPIN);  
        }
        else if (currLevel == 2) {
           blinkLight(ORANGEPIN);
        }
        else if (currLevel == 3) {
           blinkLight(REDPIN);
        }
        else {
           blinkLight();
        }


        // if water level changes, reset sound count and interrupt
        if (prevLevel != currLevel) {
            soundCount = 0;
            isSoundPlaying = false;
        }
        // play the sound three times
        if (currLevel > 0 && soundCount < 3 && !isSoundPlaying) {
           mp3.playTrackNumber(currLevel, volume, false);    
           isSoundPlaying = true;
        }
        if (mp3.playCompleted()) {
           isSoundPlaying = false;
           soundCount++;

           // set the timer for reset 
           if (soundCount == 3) {
              const unsigned long currSoundMillis = millis();
              prevSoundMillis = currSoundMillis;
           }
        }

        // reset the sound count after 10 seconds
        if (soundCount == 3) {    
            const unsigned long currSoundMillis = millis();
            const unsigned int  interval = 7000; 
            if (currSoundMillis - prevSoundMillis >= interval) {
                soundCount = 0;    
            }
        }

        
        updateVolume();
        prevLevel = currLevel;
    }
    
    else {
        offLight();
    }
    
    delay(500);
}
