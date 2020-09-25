#include "constants.hpp"
#include "control.hpp"
#include "motor.hpp"
#include "position.hpp"
#include "render.hpp"
#include "serial.hpp"
#include "state.hpp"

int pwmPin = ENA;
int dirPin = M1;

int sensorPin = A2;

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

  lastTime = micros();
}

int i = 0;

inline double deg2rad(double deg) {
  return deg * PI / 180.0;
}

inline double rad2deg(double rad) {
  return rad / PI * 180.0;
}

inline double deg2pos(double deg) {
  return rad2pos(deg2rad(deg));
}

inline double rad2pos(double rad) {
  return rad * HANDLE_RADIUS;
}

inline double pos2rad(double pos) {
  return pos / HANDLE_RADIUS;
}

inline double pos2deg(double pos) {
  return rad2deg(pos2rad(pos));
}

double lastHandlePosition = 0.0;
double handleVelocity = 0.0;
double lastHandleVelocity = 0.0;
double lastLastHandleVelocity = 0.0;

double targetPosition = deg2pos(0);

State currentState = STOPPED;
bool ignoreStateChangesUntilNewStateReached = false;

void changeState(enum State newState, bool forceChange = false) {
  if (ignoreStateChangesUntilNewStateReached && newState != currentState) {
    return;
  }

  ignoreStateChangesUntilNewStateReached = forceChange;

  if (currentState == newState) {
    return;
  }

  if (newState == DOWN_WHILE_PRESSED || newState == DOWN_UNTIL_END) {
    Serial.println("command: down");
  } else if (newState == UP_WHILE_PRESSED || newState == UP_UNTIL_END) {
    Serial.println("command: up");
  } else if (newState == STOPPED) {
    Serial.println("command: stop");
  }

  if (newState == DOWN_UNTIL_END) {
    targetPosition = deg2pos(DOWN_UNTIL_END_POSITION);
  } else if (newState == UP_UNTIL_END) {
    targetPosition = deg2pos(UP_UNTIL_END_POSITION);
  } else if (newState == STOPPED || (currentState == UP_UNTIL_END && newState == UP_WHILE_PRESSED) || (currentState == DOWN_UNTIL_END && newState == DOWN_WHILE_PRESSED)) {
    targetPosition = deg2pos(0.0);
  }

  currentState = newState;
}

void loop() {
  i++;

  position.update();

  const int updatedPosition = position.updatedPosition;
  const float handleDeg = positionToDegrees(updatedPosition);
  const float handleRad = deg2rad(handleDeg);
  const double handlePosition = rad2pos(handleRad);

  handleVelocity = -(0.95 * 0.95) * lastLastHandleVelocity +
                   2 * 0.95 * lastHandleVelocity +
                   (1 - 0.95) * (1 - 0.95) * (handlePosition - lastHandlePosition) / 0.0001;
  lastHandlePosition = handlePosition;
  lastLastHandleVelocity = lastHandleVelocity;
  lastHandleVelocity = handleVelocity;

  // const double force = renderSpring(handlePosition);

  // const double wallPosition = -0.005; // m
  // const double force = renderWall(handlePosition, wallPosition);
  // const double force = renderHardSurface(handlePosition, wallPosition);

  // const double force = renderCoulombFriction(handlePosition, handleVelocity);
  // const double force = renderViscousFriction(handlePosition, handleVelocity);

  // const double force = renderTexture(handlePosition, handleVelocity);

  const double error = handlePosition - targetPosition;
  double force = pdControl(error);

  if (currentState == DOWN_WHILE_PRESSED) {
    force += renderWall(handlePosition, deg2pos(DOWN_WHILE_PRESSED_POSITION));
  } else if (currentState == UP_WHILE_PRESSED) {
    force += renderWall(handlePosition, deg2pos(UP_WHILE_PRESSED_POSITION));
  } else if (currentState == DOWN_UNTIL_END) {
    force += renderWall(handlePosition, deg2pos(DOWN_UNTIL_END_POSITION + 5.0));
  } else if (currentState == UP_UNTIL_END) {
    force += renderWall(handlePosition, deg2pos(UP_UNTIL_END_POSITION - 5.0));
  }

  // const double force = pidControl(error);

  const double pulleyTorque = PULLEY_RADIUS / S_RADIUS * HANDLE_RADIUS * force;

  const double direction = signbit(pulleyTorque) ? -1.0 : 1.0;
  const double duty = sqrt(abs(pulleyTorque) / 0.03) * direction;

  const short speed = min(max(duty, -1.0), 1.0) * 255.0;

  auto command = receiveCommand();

  if (command != "") {
    if (command == "command: up") {
      changeState(UP_UNTIL_END, true);
    } else if (command == "command: stop") {
      changeState(STOPPED, true);
    } else if (command == "command: down") {
      changeState(DOWN_UNTIL_END, true);
    }
  }

  const bool down = direction < 0;
  const bool up = !down;

  if (handleDeg > (DOWN_UNTIL_END_POSITION - 5.0)) {
    changeState(DOWN_UNTIL_END);
  } else if (handleDeg < UP_UNTIL_END_POSITION + 5.0) {
    changeState(UP_UNTIL_END);
  } else if (handleDeg > DOWN_WHILE_PRESSED_POSITION) {
    changeState(DOWN_WHILE_PRESSED);
  } else if (handleDeg < UP_WHILE_PRESSED_POSITION) {
    changeState(UP_WHILE_PRESSED);
  } else if (abs(handleDeg) < abs(DOWN_WHILE_PRESSED_POSITION) - 3.0) {
    changeState(STOPPED);
  }

  motor.setSpeed(speed);
}
