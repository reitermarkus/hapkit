#include "motor.hpp"
#include "position.hpp"

int M1 = 12; int ENA = 3;
int M2 = 13; int ENB = 11;

int pwmPin = ENA;
int dirPin = M1;

int sensorPin = A2;

/// These were found be reading the sensor repeatedly
// and calculating the min/max values.
const int MIN_VALUE = 48;
const int MAX_VALUE = 971;
const float POSITION_10_DEGREE_DIFFERENCE = 726.6;

float positionToDegrees(int position) {
  return position / POSITION_10_DEGREE_DIFFERENCE * 10.0;
}

Position position(sensorPin, MIN_VALUE, MAX_VALUE);

Motor motor(pwmPin, dirPin);

void setup() {
  Serial.begin(57600);

  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  pinMode(sensorPin, INPUT);

  position.init();
}

int i = 0;

int maxi = 1777;

const double HANDLE_RADIUS = 0.065659; // m
const double PULLEY_RADIUS = 0.004191; // m
const double S_RADIUS = 0.073152; // m


float deg2rad(float deg) {
  return deg * PI / 180.0;
}

double lastHandlePosition = 0.0;
double handleVelocity = 0.0;
double lastHandleVelocity = 0.0;
double lastLastHandleVelocity = 0.0;

double renderSpring(const double handlePosition) {
  const double springConstant = 110; // N/m
  return -springConstant * handlePosition;
}

double renderWall(const double handlePosition, const double wallPosition) {
  const double kWall = 100; // N/m

  if ((wallPosition < 0 && handlePosition < wallPosition) || (wallPosition > 0 && handlePosition > wallPosition)) {
    return kWall;
  } else {
    return 0;
  }
}

double renderTexture(const double handlePosition, const double handleVelocity) {
  double w = 0.001; // width of damping area
  double bDamper = 1;

  for(int i = 0; i < 9; i = i + 2){
    if((abs(handlePosition) > i * w) && (abs(handlePosition) < (i + 1) * w)){
      return -bDamper * handleVelocity;
    } else if((abs(handlePosition) > (i + 1) * w) && (abs(handlePosition) < (i + 2) * w)){
      return 0;
    }
  }
}

double renderCoulombFriction(const double handlePosition, const double handleVelocity) {
  const double dampingFactor = 1;

  if (abs(handleVelocity) <= 0.25) {
    return -handleVelocity;
  } else {
    return -dampingFactor * (signbit(handleVelocity) ? -1 : 1);
  }
}

double renderViscousFriction(const double handlePosition, const double handleVelocity) {
  const double dampingFactor = 1;

  return -dampingFactor * handleVelocity;
}

void loop() {
  i++;

  position.update();

  const int updatedPosition = position.updatedPosition;
  const float handleDeg = positionToDegrees(updatedPosition);
  const float handleRad = deg2rad(handleDeg);
  const double handlePosition = HANDLE_RADIUS * handleRad;

  handleVelocity = -(0.95 * 0.95) * lastLastHandleVelocity +
                   2 * 0.95 * lastHandleVelocity +
                   (1 - 0.95) * (1 - 0.95) * (handlePosition - lastHandlePosition) / 0.0001;
  lastHandlePosition = handlePosition;
  lastLastHandleVelocity = lastHandleVelocity;
  lastHandleVelocity = handleVelocity;

  // const double force = renderSpring(handlePosition);

  // const double wallPosition = -0.005; // m
  // const double force = renderWall(handlePosition, wallPosition);

  // const double force = renderCoulombFriction(handlePosition, handleVelocity);
  // const double force = renderViscousFriction(handlePosition, handleVelocity);

  const double force = renderTexture(handlePosition, handleVelocity);

  const double pulleyTorque = PULLEY_RADIUS / S_RADIUS * HANDLE_RADIUS * force;

  const double direction = signbit(pulleyTorque) ? -1.0 : 1.0;
  const double duty = sqrt(abs(pulleyTorque) / 0.03) * direction;

  const short speed = min(max(duty, -1.0), 1.0) * 255.0;

  if (i % 10000 == 0) {
    Serial.print("Handle Position: ");
    Serial.println(updatedPosition);

    Serial.print("Handle Degrees: ");
    Serial.print(handleDeg);
    Serial.print(" ° (");
    Serial.print(handleRad);
    Serial.print(" rad, ");
    Serial.print(handlePosition * 100.0);
    Serial.println(" cm)");

    Serial.print("Velocity: ");
    Serial.print(handleVelocity);
    Serial.println(" m/s");

    Serial.print("Duty: ");
    Serial.println(duty);

    Serial.print("Speed: ");
    Serial.println(speed);
  }

  motor.setSpeed(speed);
}
