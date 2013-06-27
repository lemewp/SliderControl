// ConstantSpeed.pde
// -*- mode: C++ -*-
//
// Shows how to run AccelStepper in the simplest,
// fixed speed mode with no accelerations
// Requires the AFMotor library (https://github.com/adafruit/Adafruit-Motor-Shield-library)
// And AccelStepper with AFMotor support (https://github.com/adafruit/AccelStepper)
// Public domain!
#include <Timer.h>
#include <AccelStepper.h>
#include <AFMotor.h>
#define LIMIT_F_PIN A1
#define LIMIT_B_PIN A0
#define DEBUG_ON 1

// States:
#define NORMAL 1
#define TIMELAPSE 2
#define CONTINUOUS 3
#define FINDLIMITS 4

AF_Stepper motor1(200, 1);



boolean is_enabled = true;
int single_step = 6;
int movement_speed = 100;
int dir = 1;
unsigned long delay_time = 15000;

Timer timer;

int bounds = 500;
int speed_mod = 2;
int run_count = 0;
int state = TIMELAPSE;
int step_mode = INTERLEAVE;

// you can change these to DOUBLE or INTERLEAVE or MICROSTEP!
void forwardstep() {  
  motor1.onestep(FORWARD, step_mode);
}
void backwardstep() {  
  motor1.onestep(BACKWARD, step_mode);
}

AccelStepper stepper(forwardstep, backwardstep); // use functions to step

void setup()
{  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Slider, away!");
  
  pinMode(LIMIT_F_PIN, INPUT_PULLUP);
  pinMode(LIMIT_B_PIN, INPUT_PULLUP);

  stepper.setSpeed(0);
  
  int moveEvent = timer.every(delay_time, MoveForPhoto);
}

void loop()
{  
  // Check for serial input
  char c;
  if (Serial.available()) {
    c = Serial.read();
    if (c == 'm'){
      MoveForPhoto();
    } 
    else if (c == 'f'){
#ifdef DEBUG_ON
      Serial.println("forward");
#endif
      //stepper.moveTo(500);
      stepper.setSpeed(abs(stepper.speed()));
      dir  = 1;
    } 
    else if (c == 'b') {
#ifdef DEBUG_ON
      Serial.println("backward");
#endif
      //stepper.moveTo(-500);
      dir  = -1;
      stepper.setSpeed(abs(stepper.speed())*-1);
    } 
    else if (c == 's') {
      Stop();
    } 
    else if ((c >= '0') && (c <= '9')) {
      int speed = (c-'0')*50;
      single_step = int(c-'0')*speed_mod;
#ifdef DEBUG_ON
      Serial.print("setting speed: ");
      Serial.println(speed);
#endif

      stepper.setSpeed(speed);
    } 
    else if (c == 'r') {
      Reverse();
    } 
    else if (c == 'e') {
      FindLimits();
    }
    else if (c == 'R') {
      Release(); 
    }

  }

  CheckLimitHit();
  timer.update();
  
  if (state == CONTINUOUS) {
    if (stepper.speed() != 0)
      stepper.runSpeed();
  } else {
    
    
    if (stepper.distanceToGo() == 0) {
      //MoveForPhoto();
      //run_count++;
      if (is_enabled)
        Release();
    } else {
      if (!is_enabled)
        EnableOutput();
      stepper.run();
    }
  }
}

void Release() {
  is_enabled = false;
  motor1.release();
  //stepper.disableOutputs();
}

void EnableOutput() {
  is_enabled = true;
  stepper.enableOutputs();
}

void Reverse() {
  stepper.setSpeed(stepper.speed()*-1);

#ifdef DEBUG_ON
  Serial.print("Reversing: ");
  Serial.println(stepper.speed());
#endif
  dir = dir * -1;
  if (state == FINDLIMITS) {
    GoToLimit();
  }
}

void Stop() {
#ifdef DEBUG_ON
  Serial.println("stop");
#endif
  //stepper.stop();
  stepper.moveTo(stepper.currentPosition());
  stepper.setSpeed(0);
  //motor.run(RELEASE); 
}

int CheckLimitHit() {
  //int limitVal = digitalRead(LIMIT_F_PIN);
  if (digitalRead(LIMIT_F_PIN) == 0) {
    //Stop();
    // Forward limit hit
#ifdef DEBUG_ON
    //Serial.print("Forward limit hit");
#endif
    dir = -1;
    //stepper.setSpeed(abs(stepper.speed())*-1);
    //stepper.move(single_step*dir);
    MoveForPhoto();
    //Serial.print("Limit after: " + run_count);
  } 
  else if (digitalRead(LIMIT_B_PIN) == 0) {
    //Stop();
#ifdef DEBUG_ON
    //Serial.print("Back limit hit");
#endif
    dir = 1;
    //stepper.setSpeed(abs(stepper.speed()));
    stepper.setCurrentPosition(0);
    MoveForPhoto();
    //stepper.move(1); 
    Serial.print(run_count);
  }
  
  if (state == FINDLIMITS) {
    state = NORMAL;
  }
  
}

void FindLimits() {
#ifdef DEBUG_ON
  Serial.print("Find limit");
#endif
  state = FINDLIMITS;
  stepper.setSpeed(movement_speed*dir);
  //stepper.setAcceleration(10.5);
  stepper.moveTo(9000*dir);
}

void GoToLimit() {
   // Goes to the limit of the current direction
  stepper.moveTo(9000*dir);
}

void MoveForPhoto() {
#ifdef DEBUG_ON
  //Serial.println("move one step");
#endif
  stepper.setSpeed(movement_speed*dir);
  //stepper.setAcceleration(3.5);
  stepper.move(single_step*dir);
}

