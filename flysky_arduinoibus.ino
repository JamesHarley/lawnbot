/*
  Arduino FS-I6X Demo
  fsi6x-arduino-mega-ibus.ino
  Read iBus output port from FS-IA6B receiver module
  Display values on Serial Monitor
 
  Channel functions by Ricardo Paiva - https://gist.github.com/werneckpaiva/
 
  DroneBot Workshop 2021
  https://dronebotworkshop.com
*/
 
// Include iBusBM Library
#include <IBusBM.h>


// Define motor pins (use PWM-capable pins on your Arduino board)
const int leftMotor1Pin = 3;
const int leftMotor2Pin = 5;
const int rightMotor1Pin = 6;
const int rightMotor2Pin = 9;
const int mowerMotorPin = 10; // Define pin for mower motor

// Define channel numbers
const byte steeringChannel = 0; // Channel 1
const byte forwardBackwardChannel = 1; // Channel 2
const byte throttleChannel = 2; // Channel 3
const byte mowerSpeedChannel = 4; // Channel 5
const byte mowerOnOffChannel = 6; // Channel 7
const byte masterOnOffChannel = 9; // Channel 10
 
// Create iBus Object
IBusBM ibus;
 
// Read the number of a given channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue) {
  uint16_t ch = ibus.readChannel(channelInput);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}
 
// Read the channel and return a boolean value
bool readSwitch(byte channelInput, bool defaultValue) {
  int intDefaultValue = (defaultValue) ? 100 : 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}

// Read the channel and return a boolean value
int read3Switch(byte channelInput, int defaultValue) {
  Serial.print("Ch");

  int ch = readChannel(channelInput, 0, 100, 0);
  Serial.print("----Ch read3:: " );
  Serial.print(ch);
  Serial.print(" -----");
  if(ch == 50){
    return 1;
  }
  if( ch > 50){
    return 2;
  }
}
 
void setup() {
  // Start serial monitor
  Serial.begin(115200);
 
  // Attach iBus object to serial port
  ibus.begin(Serial1);
  // Set motor pins as outputs
  pinMode(leftMotor1Pin, OUTPUT);
  pinMode(leftMotor2Pin, OUTPUT);
  pinMode(rightMotor1Pin, OUTPUT);
  pinMode(rightMotor2Pin, OUTPUT);
  pinMode(mowerMotorPin, OUTPUT); // Set mower motor pin as output

}


void loop() {
  // Read the state of the master on/off switch from channel 10
  bool masterOn = readSwitch(masterOnOffChannel, true);

  // If the master switch is off, turn off all motors and reset states
  if (!masterOn) {
    analogWrite(leftMotor1Pin, 0);
    analogWrite(leftMotor2Pin, 0);
    analogWrite(rightMotor1Pin, 0);
    analogWrite(rightMotor2Pin, 0);
    analogWrite(mowerMotorPin, 0);
    // Reset any other states as needed
    return; // Skip the rest of the loop
  }

  // Read steering, forward/backward, and throttle values from channels
  int steering = readChannel(steeringChannel, -100, 100, 0);
  int forwardBackward = readChannel(forwardBackwardChannel, -100, 100, 0);
  int throttle = readChannel(throttleChannel, -100, 100, 0);

  // Calculate left and right motor speeds
  int leftMotorSpeed = constrain(forwardBackward + steering, -100, 100);
  int rightMotorSpeed = constrain(forwardBackward - steering, -100, 100);

  // Apply throttle
  leftMotorSpeed = map(leftMotorSpeed, -100, 100, -255, 255) * throttle / 100;
  rightMotorSpeed = map(rightMotorSpeed, -100, 100, -255, 255) * throttle / 100;

  // Write PWM values to motor pins
  analogWrite(leftMotor1Pin, leftMotorSpeed >= 0 ? leftMotorSpeed : 0);
  analogWrite(leftMotor2Pin, leftMotorSpeed <= 0 ? -leftMotorSpeed : 0);
  analogWrite(rightMotor1Pin, rightMotorSpeed >= 0 ? rightMotorSpeed : 0);
  analogWrite(rightMotor2Pin, rightMotorSpeed <= 0 ? -rightMotorSpeed : 0);
  
  // Read mower motor on/off state and speed value
  bool mowerOn = readSwitch(mowerOnOffChannel, false);
  int mowerSpeed = readChannel(mowerSpeedChannel, 0, 255, 0);

  // Control mower motor
  if (mowerOn) {
    // If mower is on set the speed based on the value from channel 5
      analogWrite(mowerMotorPin, mowerSpeed);
    } else {
      // If mower is off, set the speed to zero
      analogWrite(mowerMotorPin, 0);
  }

  // Cycle through first 5 channels and determine values
  // Print values to serial monitor
  // Note IBusBM library labels channels starting with "0"
 
  for (byte i = 0; i < 6; i++) {
    int value = readChannel(i, -100, 100, 0);
    Serial.print("Ch");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(value);
    Serial.print(" | ");
  }
  
  // Print channel 6 (switch) boolean value
  Serial.print("Ch6: ");
  Serial.print(readSwitch(6, false));
  Serial.print(" | ");
  Serial.print("Ch7: ");
  Serial.print(readSwitch(7, false));
  Serial.print(" | ");
  Serial.print("Ch8: ");
  Serial.print(read3Switch(8, 0));
  Serial.print(" | ");
  Serial.print("Ch9: ");
  Serial.print(readSwitch(9, false));
  Serial.println();  
  
  delay(10);
}