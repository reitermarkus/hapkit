#pragma once

unsigned long lastTime;
double totalError = 0;
double lastError = 0;

double control(const double error, const double kF, const double kP, const double kI, const double kD) {

  const unsigned long currentTime = micros(); // µs
  const double currentError = error;

  const double timeDiff = (currentTime - lastTime) / 1000000.0; // s
  const double errorDiff = currentError - lastError; // m
  const double errorSpeed = errorDiff / timeDiff; // m/s

  totalError += error * timeDiff;

  const double output = (-kP * error) + (-kI * totalError) + (-kD * errorSpeed) + (-kF * (signbit(error) ? -1 : 1));

  lastTime = currentTime;
  lastError = currentError;

  return output;
}

double pControl(const double error) {
  const double kF =  0.1;
  const double kP = 30.0;
  const double kI =  0.0;
  const double kD =  0.0;

  return control(error, kF, kP, kI, kD);
}

double pdControl(const double error) {
  const double kF =  0.1;
  const double kP = 50.0;
  const double kI =  0.0;
  const double kD =  3.0;

  return control(error, kF, kP, kI, kD);
}

double pidControl(const double error) {
  const double kF =  0.1;
  const double kP = 30.0;
  const double kI =  2.0;
  const double kD =  1.1;

  return control(error, kF, kP, kI, kD);
}
