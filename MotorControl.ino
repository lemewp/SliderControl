
// Used for AccelStepper's movement...
void ForwardStep() {  
  motor1.onestep(FORWARD, step_mode);
}

void BackwardStep() {  
  motor1.onestep(BACKWARD, step_mode);
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

boolean IsMoving() {
  //if (is_enabled && stepper.speed() != 0)
    return (is_enabled && stepper.speed() != 0);
  //else
    //return false;
}

void Reverse() {
  SetDirection(dir * -1);
  if (state == FINDLIMITS) {
    GoToLimit();
  }
  
  #ifdef DEBUG_ON
  Serial.print("Reversing: ");
  Serial.println(stepper.speed());
  #endif
}

void SetDirection(int new_dir) {
  if (new_dir != dir) {
    dir = new_dir;
    
    if (use_accel) {
      stepper.setMaxSpeed(movement_speed);
      stepper.setAcceleration(1950.0);
      // Reverse our target position
      if (IsMoving())
        stepper.moveTo(20000 * dir);
    } else
      stepper.setSpeed(movement_speed*dir);
    
    #ifdef DEBUG_ON
    Serial.print("Switching direction\n");
    #endif
  }
}

void SetMovementSpeed(float new_speed) {
  if (new_speed != movement_speed) {
    movement_speed = new_speed;
    
    if (use_accel) {
      stepper.setMaxSpeed(movement_speed);
      stepper.setAcceleration(450.0);
    } else
      stepper.setSpeed(movement_speed*dir);
    
    #ifdef DEBUG_ON
    //Serial.print(movement_speed*dir);
    #endif
  }
}

/*
    Movements
*/

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
  Serial.println("move one step");
  #endif
  stepper.setSpeed(movement_speed*dir);
  //stepper.setAcceleration(3.5);
  stepper.move(single_step*dir);
}

void StartMoving() {
  #ifdef DEBUG_ON
  Serial.println("Start Moving");
  #endif
  // Enable the stepper
  //EnableOutput();
  
  SetMovementSpeed(movement_speed);
  stepper.moveTo(20000*dir);
}

void Stop() {
  #ifdef DEBUG_ON
  Serial.println("Stop");
  #endif
  
  stepper.moveTo(stepper.currentPosition());
  //stepper.stop();
  //SetMovementSpeed(0.0);
  
  // Disable the stepper to save energy
  motor1.release();
  //Release();
}
