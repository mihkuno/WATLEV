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

int volume = 20;

int prevLevel = 0;
int currLevel = 0;
unsigned long previousMillis = 0; 
const long interval = 300;

void updateVolume() {
  const int potval = analogRead(POTPIN);
  volume = map(potval, 0, 1020, 0, 30);
  mp3.player.volume(volume);
}

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


void setup(void)
{

  pinMode(POTPIN,    INPUT);
  pinMode(TRIGPIN,   OUTPUT);
  pinMode(ECHOPIN,   INPUT);
  pinMode(REDPIN,    OUTPUT);
  pinMode(ORANGEPIN, OUTPUT);
  pinMode(YELLOWPIN, OUTPUT);
  pinMode(WHITEPIN,  OUTPUT);
  
  Serial.begin(9600);
  mp3.initialize();
  mp3.player.EQ(DFPLAYER_EQ_BASS);

  updateVolume();
}

void loop(void)
{
  updateVolume();

  const float distance = getDistance();
   
  if (distance > 8.5) {
    currLevel = 0;
  }
  else if (distance <= 8 && distance >= 7.5) {
     currLevel = 1;
  }

  else if (distance <= 5 && distance >= 4.5) {
     currLevel = 2;
  }

  else if (distance <= 2 && distance >= 1.5) {
     currLevel = 3;
  }

  Serial.print(distance);
  Serial.print('\t');
  Serial.println(currLevel);

  
  
  if (currLevel > 0 && prevLevel != currLevel) {


    
    // temporary bug fix

    if (currLevel == 1) {
      mp3.playTrackNumber(1, volume, false);        
    }
    else if (currLevel == 2) {
      mp3.playTrackNumber(3, volume, false);        
    }
    else if (currLevel == 3) {
      mp3.playTrackNumber(2, volume, false);        
    }
   
    int ledState = LOW;
    const int ledPin = currLevel + 2;
    
    while(!mp3.playCompleted()) {
          updateVolume();
          
          unsigned long currentMillis = millis();

          if (currentMillis - previousMillis >= interval) {
              previousMillis = currentMillis;
          
              if (ledState == LOW) {
                ledState = HIGH;
              } 
              
              else {
                ledState = LOW;
              }

              digitalWrite(WHITEPIN, ledState);
              digitalWrite(ledPin, ledState);
          }

          updateVolume();
    }

    digitalWrite(WHITEPIN, LOW);
    digitalWrite(ledPin, LOW);

    
    prevLevel = currLevel; 
  }

  delay(300);

}
