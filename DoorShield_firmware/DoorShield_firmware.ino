#include <RFM69.h>  //  https://github.com/LowPowerLab/RFM69
#include <SPI.h>
#include <Arduino.h>
#include <Wire.h> 
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "config.h"

// define node parameters
//char node[] = "25";
//#define NODEID        25 // same sa above - must be unique for each node on same network (range up to 254, 255 is used for broadcast)
#define GATEWAYID     1
#define NETWORKID     101
#define FREQUENCY     RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
//#define ENCRYPTKEY    "Tt-Mh=SQ#dn#JY3_" //has to be same 16 characters/bytes on all nodes, not more not less!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED           9 // led pin
//#define PIN           6 // NeoPixel driver pin


// define objects
RFM69 radio;

int state, door;
char dataPacket[100], _dataPacket[100];

void _isr()
{
  cli();
  state = digitalRead(3);
}


void setup()
{
  pinMode(10, OUTPUT); // Radio SS pin set as output

  Serial.begin(115200);
  Serial.println("Setup");

  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  
  pinMode(9, OUTPUT);  // pin 9 controls LED
}



void sleep()
{
  attachInterrupt(digitalPinToInterrupt(3), _isr, CHANGE);
  
  Serial.flush(); // empty the send buffer, before continue with; going to sleep
  
  radio.sleep();
  
  byte _ADCSRA = ADCSRA;  // save ADC state
  ADCSRA &= ~(1 << ADEN);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();       

  sleep_enable();  
  sleep_bod_disable();
  sei();       
  sleep_cpu();   
    
  sleep_disable();   
  sei();  

  ADCSRA = _ADCSRA; // restore ADC state (enable ADC)
  delay(1);
}





void loop() 
{
  sleep();
  detachInterrupt();
  readSensors();

  Serial.println(_dataPacket);
  Serial.println(dataPacket);
  delay(10);
 
  // send datapacket
  radio.sendWithRetry(GATEWAYID, _dataPacket, strlen(dataPacket), 5, 100);
  radio.sendWithRetry(GATEWAYID, dataPacket, strlen(dataPacket), 5, 100);  // send data, retry 5 times with delay of 100ms between each retry
  dataPacket[0] = (char)0; // clearing first byte of char array clears the array
  _dataPacket[0] = (char)0;
  
  digitalWrite(LED, HIGH);
  delay(5);
  digitalWrite(LED, LOW);
}


void readSensors()
{
  if(state == 1)
    door = 0;
  if(state == 0)
    door = 1;

  // define character arrays for all variables
  char _i[3];
  char _d[2];
  char _d_2[2];
  
  // convert all flaoting point and integer variables into character arrays
  dtostrf(NODEID, 1, 0, _i);
  dtostrf(state, 1, 0, _d);  // this function converts float into char array. 3 is minimum width, 2 is decimal precision
  dtostrf(door, 1, 0, _d_2);  // this function converts float into char array. 3 is minimum width, 2 is decimal precision
  delay(10);
  
  dataPacket[0] = 0;  // first value of dataPacket should be a 0
  _dataPacket[0] = 0;
  
  // create datapacket by combining all character arrays into a large character array
  strcat(dataPacket, "i:222");
  //strcat(dataPacket, _i);
  strcat(dataPacket, ",d:");
  strcat(dataPacket, _d);
  
  strcat(_dataPacket, "i:222");
  //strcat(_dataPacket, _i);
  strcat(_dataPacket, ",d:");
  strcat(_dataPacket, _d_2);
  delay(10);
}

/*
void fadeLED()
{
  int brightness = 0;
  int fadeAmount = 5;
  for(int i=0; i<510; i=i+5)  // 255 is max analog value, 255 * 2 = 510
  {
    analogWrite(9, brightness);  // pin 9 is LED
  
    // change the brightness for next time through the loop:
    brightness = brightness + fadeAmount;  // increment brightness level by 5 each time (0 is lowest, 255 is highest)
  
    // reverse the direction of the fading at the ends of the fade:
    if (brightness <= 0 || brightness >= 255)
    {
      fadeAmount = -fadeAmount;
    }
    // wait for 20-30 milliseconds to see the dimming effect
    delay(10);
  }
  digitalWrite(9, LOW); // switch LED off at the end of fade
}
*/


// bruh
