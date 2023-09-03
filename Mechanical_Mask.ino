#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

SoftwareSerial lnrSerial(2, 3);
#define DHTPIN A2
#define DHTTYPE DHT22 

// Filter element replacement indication lights Pin Configuration
const int greenLed = 0;
const int redLed = 1;

// Movement Detection Pin Configuration
const int pirSensor = A1;

// Air Regulating and Cooling Fan Pin Configuration
const int fan = A4;

// Motor Pin Configuration
const int motorStby = A3;
const int leftMotorIn1 = 4;
const int leftMotorIn2 = 5;
const int rightMotorIn1 = 6;
const int rightMotorIn2 = 7;

// Limit Switch Pin Configuration -- OS - OUTER SWITCH, IS - INNER SWITCH --
const int leftMotorOS = 8;
const int leftMotorIS = 9;
const int rightMotorOS = 10;
const int rightMotorIS = 11;

// Control Button Pin Configuration -- Left Button - Close, Right Button - Open --
const int openButton = 12;
const int closeButton = 13;
const int autoButton = A0;

// Control Button LEDs
const int buttonLed = A5;


DHT dht = DHT(DHTPIN, DHTTYPE);

// variable for checking movement for PIR sensor
boolean movementDetected = false;

// variables for TF-LUNA proximity sensor
int dist;
int strength;
int check;
int uart[9];
const int HEADER=0x59;

// variables for button press and debouncing effect for auto/manual button
int autoLastState = 0;
int autoCurrentState = 0;
boolean autoMode = false;

int time_start = 0;

// function for debounce of auto button and checking auto button status
void check_auto_button_status(){
  // debounce the button
  autoCurrentState = analogRead(autoButton);
  if ((autoLastState < 50 && autoCurrentState > 200) 
  || (autoLastState > 200 && autoCurrentState < 50)){
    delay(5);
    autoCurrentState = analogRead(autoButton);
  }
  // check button state
  if (autoLastState < 50 && autoCurrentState > 200){
    // flip auto button state to whatever it is to the other
    autoMode = !autoMode;
  }
  autoLastState  = autoCurrentState;
}

// function to activate linear actuator motors for opening mask
void open_mask(){
  digitalWrite(motorStby, HIGH);
  delay(10);
  digitalWrite(leftMotorIn1, LOW);
  digitalWrite(leftMotorIn2, HIGH);
  digitalWrite(rightMotorIn1, HIGH);
  digitalWrite(rightMotorIn2, LOW);
  digitalWrite(fan, LOW);
}

// function to activate linear actuator motors for closing mask
void close_mask(){
  digitalWrite(motorStby, HIGH);
  delay(10);
  digitalWrite(leftMotorIn1, HIGH);
  digitalWrite(leftMotorIn2, LOW);
  digitalWrite(rightMotorIn1, LOW);
  digitalWrite(rightMotorIn2, HIGH);
  digitalWrite(fan, HIGH);
}

// function to check the limit switches if the are pressed
void check_limits(){
  // saving states of the limit switches as variables
  int leftMotorOSS = digitalRead(leftMotorOS);
  int rightMotorOSS = digitalRead(rightMotorOS);
  int leftMotorISS = digitalRead(leftMotorIS);
  int rightMotorISS = digitalRead(rightMotorIS);

  // if ststements to run if any of the limit switches are pressed
  if (leftMotorOSS == HIGH){
    digitalWrite(leftMotorIn1, LOW);
    digitalWrite(leftMotorIn2, LOW);
    delay(30);
    digitalWrite(leftMotorIn1, LOW);
    digitalWrite(leftMotorIn2, HIGH);
    delay(350);
    digitalWrite(leftMotorIn1, LOW);
    digitalWrite(leftMotorIn2, LOW);
  }
  if (rightMotorOSS == HIGH){
    digitalWrite(rightMotorIn1, LOW);
    digitalWrite(rightMotorIn2, LOW);
    delay(30);
    digitalWrite(rightMotorIn1, HIGH);
    digitalWrite(rightMotorIn2, LOW);
    delay(350);
    digitalWrite(rightMotorIn1, LOW);
    digitalWrite(rightMotorIn2, LOW);
  }
  if (leftMotorISS == HIGH){
    digitalWrite(leftMotorIn1, LOW);
    digitalWrite(leftMotorIn2, LOW);
    delay(30);
    digitalWrite(leftMotorIn1, HIGH);
    digitalWrite(leftMotorIn2, LOW);
    delay(300);
    digitalWrite(leftMotorIn1, LOW);
    digitalWrite(leftMotorIn2, LOW);
  }
  if (rightMotorISS == HIGH){
    digitalWrite(rightMotorIn1, LOW);
    digitalWrite(rightMotorIn2, LOW);
    delay(30);
    digitalWrite(rightMotorIn1, LOW);
    digitalWrite(rightMotorIn2, HIGH);
    delay(300);
    digitalWrite(rightMotorIn1, LOW);
    digitalWrite(rightMotorIn2, LOW);
  }
 
}

//int check_proximity(){
//  if (lnrSerial.available()){
//    if (lnrSerial.read() == HEADER){
//      uart[0] = HEADER;
//      if (lnrSerial.read() == HEADER){
//        uart[1] = HEADER;
//        for (int i=2; i<9; i++){
//          uart[i] = lnrSerial.read();
//        }
//        check = uart[0]+uart[1]+uart[2]+uart[3]+uart[4]+uart[5]+uart[6]+uart[7];
//        if (uart[8] == (check&0xff)){
//          dist = uart[2] + uart[3] * 256;
//          strength = uart[4] + uart[5]  * 256;
//          delay(50);
//          return dist;
//          
//        }
//      }
//    }
//  }
//}

// function to check humidity and turn on/off the respective indicating LEDs
void check_humidity(){
  float h = dht.readHumidity();
  if (h >= 80){
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  }
  if (h < 80){
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);
  }
}

// function to check movement with PIR sensor
boolean check_movement(){
  int pirVal = analogRead(pirSensor);
  if (pirVal > 200){
    movementDetected = true;
  }
  if (pirVal < 50){
    movementDetected = false;
  }
}

// setup function defining the pin modes of the pins and initializing Serial 
// connection with TF-LUNA and initializing the humidity sensor
void setup(){
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(motorStby, OUTPUT);
  pinMode(leftMotorIn1, OUTPUT);
  pinMode(leftMotorIn2, OUTPUT);
  pinMode(rightMotorIn1, OUTPUT);
  pinMode(rightMotorIn2, OUTPUT);
  pinMode(buttonLed, OUTPUT);
  pinMode(pirSensor, INPUT);
  pinMode(leftMotorOS, INPUT);
  pinMode(leftMotorIS, INPUT);
  pinMode(rightMotorOS, INPUT);
  pinMode(rightMotorIS, INPUT);
  pinMode(openButton, INPUT);
  pinMode(closeButton, INPUT);
  pinMode(autoButton, INPUT);
  lnrSerial.begin(115200);
  dht.begin();
}

// loop fucntion which runs repeatedly
void loop(){
  // check the auto buttin state
  check_auto_button_status();
  // start timer, on boot up time_start will be 0
  // therefore time_elapsed = time_end
  int time_end = millis();
  int time_elapsed = time_end - time_start;
  
  if (lnrSerial.available()){
    if (lnrSerial.read() == HEADER){
      uart[0] = HEADER;
      if (lnrSerial.read() == HEADER){
        uart[1] = HEADER;
        for (int i=2; i<9; i++){
          uart[i] = lnrSerial.read();
        }
        check = uart[0]+uart[1]+uart[2]+uart[3]+uart[4]+uart[5]+uart[6]+uart[7];
        if (uart[8] == (check&0xff)){
          dist = uart[2] + uart[3] * 256;
          strength = uart[4] + uart[5]  * 256;
          // if statement to check if in automode and time_elapsed = 10 seconds or
          // if it is the first run of the program which will give time_start = 0
          if ((autoMode) && ((time_start == 0) || (time_elapsed > 10000))){
            digitalWrite(buttonLed, LOW);
            check_movement();
            // reset the time_start value
            time_start = millis();  
            // if distance is greater than 50 cm from an object the mask opens
            if (dist > 50){
              open_mask();
            }
            // if distance <= 50 cm and movementDetected variable is true mask closes
            if (dist <= 50 && movementDetected){
              close_mask();
            }
          }
          delay(50);
        }
      }
    }
  }
  // if it is in manual mode 
  if (!autoMode){
    digitalWrite(buttonLed, HIGH);
    time_start = 0;
    // check buttonstates of the open and close buttons
    int openButtonStatus = digitalRead(openButton);
    int closeButtonStatus = digitalRead(closeButton);
    // execute functions to open or close mask depending on button status
    if (openButtonStatus == HIGH){
      open_mask();
    }
    if (closeButtonStatus == HIGH){
      close_mask();
    }   
  }
  // update humidity values and turn on/off indicating LEDs
  check_humidity();
  // check limit switches
  check_limits();
  // if the two inputs for both motors are all LOW, then put the motor driver on standby for power save
  if (leftMotorIn1 == LOW && leftMotorIn2 == LOW && rightMotorIn1 == LOW && rightMotorIn2 == LOW){
    digitalWrite(motorStby, LOW);
  }
}
