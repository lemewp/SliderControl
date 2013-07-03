// SliderControl
// -*- mode: C++ -*-
//
// Basic control of a stepper based slider.
// Uses the AccelStepper and AFMotor libraries for communication.
// Can be controlled by serial or via sensors.
// Includes support for limit switches to prevent overstepping.

#include <Timer.h>
#include <AccelStepper.h>
#include <AFMotor.h>
#include <stdarg.h>

// Print debug strings via serial
#define DEBUG_ON 1

// Limit switch pins (forward and backwards):
#define LIMIT_F_PIN A0
#define LIMIT_B_PIN A1
#define SPEED_POT_PIN A2
#define DIRECTION_SWITCH_PIN A3
#define START_BTN_PIN A4

// States:
#define NORMAL 1
#define TIMELAPSE 2
#define CONTINUOUS 3
#define FINDLIMITS 4

// Hold time for start button to switch modes
#define MODE_SWITCH_DELAY 2000
#define DEBOUNCE_DELAY 150
#define SPEED_MOD 2            // Step size adjustment for time lapse

// The stepper motor object
AF_Stepper motor1(200, 1);


// Timelapse timer
Timer timer;

boolean is_enabled = true;           // Movement is enabled
boolean use_accel = true;            // Use smooth acceleration
int single_step = 6;
float movement_speed = 100.0;
int last_speed_read = 0;
int dir = 1;                         // Direction (1 or -1)
unsigned long delay_time = 15000;    // Delay between steps when in timelapse mode

int high_bounds = 500;
int run_count = 0;
int state = CONTINUOUS;
int step_mode = INTERLEAVE;          // Stepper mode: SINGLE, DOUBLE, INTERLEAVE, or MICROSTEP

unsigned long last_print_time = millis();
unsigned long last_btn_time = 0;
int last_btn_state = HIGH;


void p(char *fmt, ... ){
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}

AccelStepper stepper(ForwardStep, BackwardStep); // use functions to step

void setup()
{  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Slider, away!");
  
  pinMode(LIMIT_F_PIN, INPUT_PULLUP);
  pinMode(LIMIT_B_PIN, INPUT_PULLUP);
  pinMode(START_BTN_PIN, INPUT_PULLUP);
  pinMode(DIRECTION_SWITCH_PIN, INPUT_PULLUP);
  
  stepper.setSpeed(0);
  
  if (state == TIMELAPSE)
    int moveEvent = timer.every(delay_time, MoveForPhoto);
}

void loop()
{  
  do {
    CheckSerial();
    CheckInputs();
    CheckLimitHit();
    
    if (state == CONTINUOUS) {
      if (stepper.speed() != 0) {
        if (use_accel) {
        //  while (digitalRead(START_BTN_PIN) == LOW)
            stepper.run();
        }
        else //if (stepper.speed() != 0)
          stepper.runSpeed();
      }
    } else {
      if (state == TIMELAPSE)
        // If we're in timelapse mode, check if a movement is due
        timer.update();
        
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
  } while (stepper.distanceToGo() != 0);
}

void CheckSerial() {
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
      single_step = int(c-'0')*SPEED_MOD;
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
    else if (c == 't') {
      // Cycle step types
      // Micro step is very slow...
      // Interleave seems like a good balance
      CycleModes();
    }
    else if (c == 'R') {
      Release(); 
    }
  }
}

void CheckInputs() {
   int input_speed = analogRead(SPEED_POT_PIN);
   
   if (last_speed_read + 1 < input_speed  || last_speed_read - 1 > input_speed ) {
     
     SetMovementSpeed(ConvertAnalogReadingToSpeed(input_speed));
     last_speed_read = input_speed;
     
     #ifdef DEBUG_ON
     if ((millis() - last_print_time) > 2000 && input_speed > 1) {
       
       Serial.print("Input speed: ");
       Serial.print(input_speed);
       Serial.print("\n");
       
       //if (digitalRead(DIRECTION_SWITCH_PIN) == LOW)
         //Serial.print("Switch open");       
       last_print_time = millis();
     }
     #endif
   }
   
   // Start button is pressed
   if (digitalRead(START_BTN_PIN) == LOW) {
     if (last_btn_state == HIGH && millis() - last_btn_time > DEBOUNCE_DELAY) {
       // If this is the first time we're noticing the button press and it's been more than the debounce delay...
       last_btn_time = millis();
       last_btn_state = LOW;
       
       if (IsMoving())
         Stop();
       else
         StartMoving();
     } else if (last_btn_state == LOW && millis() - last_btn_time > MODE_SWITCH_DELAY) {
       // Button has been held down for a while, let's perform a secondary function: switch stepper modes
       last_btn_time = millis();
       CycleModes(); 
     }
   } else {
     // Button HIGH 
     last_btn_state = HIGH;
   }
   
   // Direction switch
   if (digitalRead(DIRECTION_SWITCH_PIN) == LOW) {
       SetDirection(1);
   }
   else
       SetDirection(-1);
}

void CheckLimitHit() {
  //int limitVal = digitalRead(LIMIT_F_PIN);
  if (digitalRead(LIMIT_F_PIN) == 0) {
    //Stop();
    // Forward limit hit
#ifdef DEBUG_ON
    Serial.print("Forward limit hit");
#endif
    dir = -1;
    high_bounds = stepper.currentPosition();
    stepper.setSpeed(abs(stepper.speed())*-1);
    stepper.move(-1);
  } 
  else if (digitalRead(LIMIT_B_PIN) == 0) {
#ifdef DEBUG_ON
    Serial.print("Back limit hit");
#endif
    dir = 1;
    stepper.setSpeed(abs(stepper.speed()));
    stepper.setCurrentPosition(0);
    stepper.move(1); 
    Serial.print(run_count);
  }
  
  if (state == FINDLIMITS) {
    state = NORMAL;
  }
}

void CycleModes() {
  step_mode = step_mode % 4 + 1;
  p("Step mode: %i\n", step_mode);
}

float ConvertAnalogReadingToSpeed(int reading) {
  // Takes a raw pot value and returns a usable speed
  return float(reading * 0.80);
}


