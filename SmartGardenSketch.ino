#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <dht.h>
#include <Wire.h>

#include "ds3231.h"
struct ts t; 

#define DHT11_PIN 5

const byte DS3231 = 0x68;

dht DHT;

const int analogInPin = A0; 

SoftwareSerial ble(7, 8); // RX, TX for ble

unsigned long int avgValue; 
int buf[10],temp;
//float b;

int tint = 0;

uint32_t myTimer1;
uint32_t myTimer2;

int monitorSeconds (int a){
  return a*1000;
}

float phValue(){     
  float calibration = 21.38; //подбор
  for(int i=0;i<10;i++) { 
    buf[i]=analogRead(analogInPin);
    delay(30);
  }   
  for(int i=0;i<9;i++) {
    for(int j=i+1;j<10;j++) {  
      if(buf[i]>buf[j]) {
        temp=buf[i];
        buf[i]=buf[j];   
        buf[j]=temp;
      }
    }
  } 
  avgValue=0;
  for(int i=2;i<8;i++)
  avgValue+=buf[i];
  float pHVol=(float)avgValue*5.0/1024/6;
  float phValue = -5.70 * pHVol + calibration;
  return phValue;
}

void setup() {
  Serial.begin(9600);
  ble.begin(9600);
  Wire.begin();
  
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);
}

void loop() {    

  if (millis() - myTimer1 >= 2500) {  //Таймер 2 (25млс)
    myTimer1 = millis();              
    
    DS3231_get(&t);
    Serial.print(t.hour);
    Serial.print(":");
    Serial.println(t.min);
    
    byte bleDataRead = ble.read();//read data

    switch (bleDataRead) {
      case 1:
        digitalWrite(2, HIGH);
        break;
      case 2:
        digitalWrite(2, LOW);
        break;
      case 3:
        digitalWrite(3, HIGH);
        break;
      case 4:
        digitalWrite(3, LOW); 
        break;
      case 5:
        digitalWrite(4, HIGH);
        break;
      case 6:
        digitalWrite(4, LOW);
        break;
      case 7:
        tint = 0;
        break;
      case 8:
        tint = 1;
        break;
      case 9:
        tint = 2;
        break;
      case 0:
        digitalWrite(2, LOW);
        digitalWrite(3, LOW); 
        digitalWrite(4, LOW); 
        tint = 0;
        break;     
    }

    if(tint == 1){
      if(t.hour == 6){
        digitalWrite(2, HIGH);
      }  
      if(t.hour == 23 && t.min == 59) {
        digitalWrite(2, LOW);
      }
    }
    
    if(tint == 2){
      if(t.hour==12){
        digitalWrite(2, HIGH);
      }  
      if(t.hour == 23 && t.min == 59) {
        digitalWrite(2, LOW);
      }
    }
    
    Serial.println(bleDataRead);
        
  }

  if (millis() - myTimer2 >= 2500) {  // Таймер 2 (25млс)
    myTimer2 = millis();               

    int chk = DHT.read11(DHT11_PIN);
    
    int PH = (phValue()*100);
    int PHa = PH/100;
    int PHb = PH%100;    

    byte b[] = {
        digitalRead(2),
        digitalRead(3),
        digitalRead(4),
        tint,
        PHa,
        PHb,
        DHT.temperature,
        DHT.humidity
    };

    ble.write(b,8); 

    Serial.println(String(digitalRead(2)) + 
        String(digitalRead(3)) + 
        String(digitalRead(4)) + 
        String(tint) + " " + 
        String(phValue()) + " " + 
        String(DHT.temperature) + " " + 
        String(DHT.humidity));
    
  }
    
}
