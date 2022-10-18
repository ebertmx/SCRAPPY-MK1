/*
Control program for SCOOBY to read positional sensor data, average it and send it to
BERT's control system
*/


#include <Arduino.h>
#include <string.h>
#include <math.h>

// Global constants
// interval of time to average positional sensor values for
unsigned long interval = 15;
String positional_character = "p";
char final_character = '\n';

int link0_pin = 25;
int link1_pin = 26;
int link2_pin = 27; 

int readQvalue(String pinN);
bool checkbutton(int pin);


bool sending = false;
int magnet = false;

void setup()
{
  // Set serial to a baud rate of 115200 and configure analog and digital pins
  // Analog pins are for the potentiometers
  // Digital pin is for the control button

  Serial.begin(115200);
  pinMode(link0_pin, INPUT);
  pinMode(link1_pin, INPUT);
  pinMode(link2_pin, INPUT);
  //pinMode(3, OUTPUT);
  pinMode(4, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  //digitalWrite(3,1);
  Serial.println("Program Start");
}

void loop()
{
    delay(250);  
  // Instantiate local variables
  unsigned long previousMillis = millis();
  unsigned long currentMillis = millis();
  int n = 0;
  long int a = 0;
  long int b = 0;
  long int c = 0;

  // Average control values over period of time of interval
  while ((currentMillis - previousMillis) < interval)
  {
    a = analogRead(link0_pin) + a;
    b = analogRead(link1_pin) + b;
    c = analogRead(link2_pin) + c;

    n = n + 1;
    currentMillis = millis();
  }

  // Set control values to average over the time interval
  
  //a = 0;
  //b = 0;
  //c = 0;

  a = a / n;
  b = b / n;
  c = c / n;
  
  a = a * 2900 / 4095;
  b = (b-720) * 1545/(4095-718);
  c = 2190 - ((c-720) * 2190/(4095-718));

  // Reset time
  previousMillis = currentMillis;

  // Set control data value
  String control_data = "P:" + String(a) + ',' + String(b) + ',' + String(c) + ',' + "50" + ',' + "50" + ',' + "50" + final_character;
  
  //Set calibration data
  String calibration_data = "C:" + String(a) + ',' + String(b) + ',' + String(c) + ',' + "50" + ',' + "50" + ',' + "50" + final_character;
    
  // Send control data to control system if SKIPPY's control button is pressed
  
  if (checkbutton(4))
  {
    Serial.println(control_data);
    while (checkbutton(4)){
     continue;
    }
  }

  if (checkbutton(2))
  {
    Serial.println(calibration_data);
    while (checkbutton(2)){
     continue;
    }
  }

  if (checkbutton(1)){
    magnet = !magnet;
    Serial.println("M:%s,0,0,0,0,0", String(magnet))
  }
  
}

bool checkbutton(int pin) {
  unsigned int state = 0xff;
  int i = 0;
  while (i < 16) {
    i = i + 1;
    state = ((state << 1 | digitalRead(pin) | 0x00)) & 0xff;
  }
  if (0x00 == state) {
    return true;
  }
  else {
    return false;
  }

}


