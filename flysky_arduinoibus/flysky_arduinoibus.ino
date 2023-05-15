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

//TODO: Speed curve
//TODO: rewrite direction flip using function for left vs right

// Define motor pins (use PWM-capable pins on your Arduino board)
//left front
const int leftFrontMotor = 3;
const int leftFrontDirPin = 16;
const int lefFrontDirection = HIGH;
//left rear
const int leftRearMotor = 5;
const int leftRearDirPin = 17;
const int leftRearDirection = HIGH;
//right front
const int rightFrontMotor = 6; 
const int rightFrontDirPin = 20;
const int rightFrontDirection = LOW;
//right rear
const int rightRearMotor = 9;
const int rightRearDirPin = 21;
const int rightRearDirection = LOW;
//mower motor
const int mowerMotorPin = 10; // Define pin for mower motor

// Define channel numbers
const byte steeringChannel = 0; // Channel 1
const byte forwardBackwardChannel = 1; // Channel 2
const byte throttleChannel = 2; // Channel 3
const byte mowerSpeedChannel = 4; // Channel 5
const byte mowerOnOffChannel = 6; // Channel 7
const byte masterOnOffChannel = 9; // Channel 10

// define a new pin for the motor direction on each motor (except the mower motor)


// create a map for the motor pins, the channel, the direction, and the pwm value
// use the constants defined above for the motor pins and channel numbers
// use the constants HIGH and LOW for the direction
// use the pwm value from the channel for the pwm value
//LF, LR, RF, RR
int driveMotorMap[4][5] = {
  {leftFrontMotor, steeringChannel, lefFrontDirection, 0,  leftFrontDirPin},
  {leftRearMotor, steeringChannel, leftRearDirection, 0 , leftRearDirPin},
  {rightFrontMotor, steeringChannel, rightFrontDirection, 0 , rightFrontDirPin},
  {rightRearMotor, steeringChannel, rightRearDirection, 0 , rightRearDirPin}
};
//Store previous driveMotorMap in a temporary variable to indicate the direction so we can handle the ramp down
//LF, LR, RF, RR
int tempDriveMotorMap[4][5] = {
  {leftFrontMotor, steeringChannel, lefFrontDirection, 0,  leftFrontDirPin},
  {leftRearMotor, steeringChannel, leftRearDirection, 0 , leftRearDirPin},
  {rightFrontMotor, steeringChannel, rightFrontDirection, 0 , rightFrontDirPin},
  {rightRearMotor, steeringChannel, rightRearDirection, 0 , rightRearDirPin}
};

//Quick Fix
void directionCorrection(){
  for(int i = 0; i < 4; i++){
    if(i == 2 || i == 3){
      //flip HIGH and LOW
      if(driveMotorMap[i][2] == HIGH){
        driveMotorMap[i][2] = LOW;
      } else {
        driveMotorMap[i][2] = HIGH;
      }
    }
  }
}

void motorDrive(){
  //read the steering channel value
  int steering = readChannel(steeringChannel, -100, 100, 0);
  int forwardBackward = readChannel(forwardBackwardChannel, -100, 100, 0);
  int throttle = readChannel(throttleChannel, -100, 100, 0);

  int direction = HIGH;
  //Straight forward or backward
  //LF, LR, RF, RR
  if(steering > -30 && steering < 30){
    if(forwardBackward > 0){
      direction = HIGH;
      //set map
    }
    else{
      direction = LOW;
    }
    //all motors the same direction
    for(int i = 0; i < 4; i++){
      driveMotorMap[i][2] = direction;
    }
  }
  if(steering > 30 && forwardBackward > 0){
    //turning right and forward
    //left motors forward
    //right motors backward

    for(int i = 0; i < 2; i++){
      driveMotorMap[i][2] = LOW;
    }
    for(int i = 2; i < 4; i++){
      driveMotorMap[i][2] = HIGH;
    }
  }
  if(steering < -30 && forwardBackward > 0){
    //turning left and forward
    //left motors backward
    //right motors forward
    for(int i = 0; i < 2; i++){
      driveMotorMap[i][2] = HIGH;
    }
    for(int i = 2; i < 4; i++){
      driveMotorMap[i][2] = LOW;
    }
  }
  if(steering > 30 && forwardBackward < 0){
    //turning right and backward
    //left motors forward
    //right motors backward
    for(int i = 0; i < 2; i++){
      driveMotorMap[i][2] = HIGH;
    }
    for(int i = 2; i < 4; i++){
      driveMotorMap[i][2] = LOW;
    }
  }
  if(steering < -30 && forwardBackward < 0){
    //turning left and backward
    //left motors backward
    //right motors forward
    for(int i = 0; i < 2; i++){
      driveMotorMap[i][2] = LOW;
    }
    for(int i = 2; i < 4; i++){
      driveMotorMap[i][2] = HIGH;
    }
  }
  //fix motor direction caused by meridian flip
  directionCorrection();
  int speed = forwardBackward >= 0 ? forwardBackward : forwardBackward * -1;
  //if forwardBackward is greater than 100 then set it to 100
  if(speed > 1){
    speed = 1;

  } else {
    speed = 0;
  }
  // convert throttle to a positive number from -100 to 100 equals 1 to 100
  throttle = map(throttle, -100, 100, 0, 100);
  //write the pwm values to the motors
  for(int i = 0; i < 4; i++){
    //map the pwm value to the motor
    driveMotorMap[i][3] = speed * ( throttle / 100.0) * 255.0;
    //compare the current pwm value to the previous pwm value and if it is different then we need to ramp up or down
    // if(tempDriveMotorMap[i][3] != driveMotorMap[i][3]){
    //   //if the current pwm value is greater than the previous pwm value then we need to ramp up
    //   if(tempDriveMotorMap[i][3] < driveMotorMap[i][3]){
    //     //we need to ramp up the pwm values for the motors that are changing direction
    //     analogWrite(driveMotorMap[i][0], driveMotorMap[i][3] >= 0 ? driveMotorMap[i][3] : 0);
       

    //   }
    //   //if the current pwm value is less than the previous pwm value then we need to ramp down
    //   if(tempDriveMotorMap[i][3] > driveMotorMap[i][3]){
    //     //we need to ramp down the pwm values for the motors that are changing direction
      
    //     analogWrite(driveMotorMap[i][0], driveMotorMap[i][3] >= 0 ? driveMotorMap[i][3] : 0);
    //   }
    // }
    //if the current pwm value is the same as the previous pwm value then we don't need to ramp up or down
    analogWrite(driveMotorMap[i][0], driveMotorMap[i][3] >= 0 ? driveMotorMap[i][3] : 0);
    //write the direction to the dir pin for each motor
    //print to serial
    
    delay(100);
    Serial.print("Motor: ");
    Serial.print(i);
    Serial.print(" Direction: ");
    Serial.print(driveMotorMap[i][2]);
    Serial.print(" PWM: ");
    Serial.println(driveMotorMap[i][3]);

    digitalWrite(driveMotorMap[i][4], driveMotorMap[i][2]);
  }

  return;
}



 
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
  int ch = readChannel(channelInput, 0, 100, 0);
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
  pinMode(leftFrontMotor, OUTPUT);
  pinMode(leftRearMotor, OUTPUT);
  pinMode(rightFrontMotor, OUTPUT);
  pinMode(rightRearMotor, OUTPUT);
  pinMode(mowerMotorPin, OUTPUT); // Set mower motor pin as output

  pinMode(leftFrontDirPin, OUTPUT);
  pinMode(leftRearDirPin, OUTPUT);
  pinMode(rightFrontDirPin, OUTPUT);
  pinMode(rightRearDirPin, OUTPUT);


}




void loop() {
  // Read the state of the master on/off switch from channel 10
  bool masterOn = readSwitch(masterOnOffChannel, true);

  // If the master switch is off, turn off all motors and reset states
  if (!masterOn) {
    analogWrite(leftFrontMotor, 0);
    analogWrite(leftRearMotor, 0);
    analogWrite(rightFrontMotor, 0);
    analogWrite(rightRearMotor, 0);
    analogWrite(mowerMotorPin, 0);
    // Reset any other states as needed
    return; // Skip the rest of the loop
  }
  //store driveMotorMap in a temporary variable to indicate the direction so we can handle the ramp down
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 4; j++){
      tempDriveMotorMap[i][j] = driveMotorMap[i][j];
    }
  }
  
  motorDrive();
  
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
  Serial.print("Ch7: ");
  Serial.print(readSwitch(6, false));
  Serial.print(" | ");
  Serial.print("Ch8: ");
  Serial.print(readSwitch(7, false));
  Serial.print(" | ");
  Serial.print("Ch9: ");
  Serial.print(read3Switch(8, 0));
  Serial.print(" | ");
  Serial.print("Ch10: ");
  Serial.print(readSwitch(9, false));
  Serial.println();  
  
  delay(10);
}