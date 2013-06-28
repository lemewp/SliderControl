
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

void SetDirection(int new_dir) {
  if (new_dir != dir) {
    dir = new_dir;
    stepper.setSpeed(stepper.speed()*-1);
    
    #ifdef DEBUG_ON
    Serial.print("Switching direction\n");
    #endif
  }
}

void SetMovementSpeed(float new_speed) {
  if (new_speed != movement_speed) {
    movement_speed = new_speed;
    
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
  //Serial.println("move one step");
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
  EnableOutput();
  stepper.moveTo(19000*dir);
  SetMovementSpeed(movement_speed);
}

void Stop() {
  #ifdef DEBUG_ON
  Serial.println("Stop");
  #endif
  
  stepper.moveTo(stepper.currentPosition());
  //stepper.stop();
  SetMovementSpeed(0.0);
  
  // Disable the stepper to save energy
  Release();
}
