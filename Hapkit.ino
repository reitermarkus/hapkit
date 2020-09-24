#include "motor.hpp"
#include "position.hpp"
#include "state.hpp"

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

unsigned long lastTime;

void setup() {
  Serial.begin(57600);

  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  pinMode(sensorPin, INPUT);

  position.init();

  lastTime = micros();
}

int i = 0;

const double HANDLE_RADIUS = 0.065659; // m
const double PULLEY_RADIUS = 0.004191; // m
const double S_RADIUS = 0.073152; // m

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

double renderSpring(const double handlePosition) {
  const double springConstant = 110; // N/m
  return -springConstant * handlePosition;
}

double renderWall(const double handlePosition, const double wallPosition) {
  const double kWall = 50; // N/m

  // Detect if we hit the wall, i.e. if we are inside of the wall.
  if ((wallPosition < 0 && handlePosition < wallPosition) || (wallPosition > 0 && handlePosition > wallPosition)) {
    return -kWall * handlePosition;
  } else {
    return 0;
  }
}

bool wasInsideWall = false;
unsigned long wallEntryTime = 0;

double renderHardSurface(const double handlePosition, const double wallPosition) {
  const double wallForce = renderWall(handlePosition, wallPosition);
  const bool insideWall = wallForce != 0;

  double vibrationForce = 0;
  const double maxVibrationLength = 50;

  if (insideWall) {
    const unsigned long currentTime = millis();

    if (!wasInsideWall) {
      wallEntryTime = currentTime;
    }
    wasInsideWall = true;

    // Serial.print("wallEntryTime: ");
    // Serial.println(wallEntryTime);
    // Serial.print("time diff: ");
    // Serial.println(currentTime - wallEntryTime);

    const unsigned long vibrationTime = (maxVibrationLength - min(currentTime - wallEntryTime, maxVibrationLength));

    // Serial.print("vibrationTime: ");
    // Serial.println(vibrationTime);

    const double vibrationTimeRatio = (vibrationTime / maxVibrationLength);
    // Serial.print("vibrationTimeRatio: ");
    // Serial.println(vibrationTimeRatio);

    const double cosInput = currentTime / 1000.0 * 90.0;

    vibrationForce = cos(cosInput) * vibrationTimeRatio;

    // Serial.print("vibrationForce: ");
    // Serial.println(vibrationForce);
  } else {
    wasInsideWall = false;
  }

  return wallForce + vibrationForce;
}

double renderTexture(const double handlePosition, const double handleVelocity) {
  double w = 0.001; // width of damping area
  double dampingFactor = 1;

  for (int i = 0; i < 9; i = i + 2) {
    if ((abs(handlePosition) > i * w) && (abs(handlePosition) < (i + 1) * w)){
      return -dampingFactor * handleVelocity;
    } else if ((abs(handlePosition) > (i + 1) * w) && (abs(handlePosition) < (i + 2) * w)){
      return 0;
    }
  }

  return 0;
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
  const double dampingFactor = 4;

  return -dampingFactor * handleVelocity;
}


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
  const double kD =  2.5;

  return control(error, kF, kP, kI, kD);
}

double pidControl(const double error) {
  const double kF =  0.1;
  const double kP = 30.0;
  const double kI =  2.0;
  const double kD =  1.1;

  return control(error, kF, kP, kI, kD);
}

String command;

String receiveCommand() {
  if (!Serial.available()) {
    return "";
  }

  char c = Serial.read();

  if (c == '\r' || c == '\n') {
    String currentCommand = command;
    command = "";
    return currentCommand;
  } else {
    command += c;
    return "";
  }
}

double targetPosition = deg2pos(0);

State currentState = STOPPED;

double DOWN_UNTIL_END_POSITION = 30.0;
double UP_UNTIL_END_POSITION = -DOWN_UNTIL_END_POSITION;
double DOWN_WHILE_PRESSED_POSITION = 5.0;
double UP_WHILE_PRESSED_POSITION = -DOWN_WHILE_PRESSED_POSITION;

bool ignoreStateChangesUntilNewStateReached = false;

void changeState(enum State newState, bool force = false) {
  if (ignoreStateChangesUntilNewStateReached && newState != currentState) {
    return;
  }

  ignoreStateChangesUntilNewStateReached = false;

  if (currentState == newState) {
    return;
  }

  Serial.print("Changing state from ");
  Serial.print(currentState);
  Serial.print(" to ");
  Serial.println(newState);

  if (newState == DOWN_WHILE_PRESSED || newState == DOWN_UNTIL_END) {
    Serial.println("send-command: down");
  } else if (newState == UP_WHILE_PRESSED || newState == UP_UNTIL_END) {
    Serial.println("send-command: up");
  } else if (newState == STOPPED) {
    Serial.println("send-command: stop");
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
    Serial.print("Command: '");
    Serial.print(command);
    Serial.println("'");

    if (command == "up") {
      changeState(UP_UNTIL_END, true);
      ignoreStateChangesUntilNewStateReached = true;
    } else if (command == "stop") {
      changeState(STOPPED, true);
      ignoreStateChangesUntilNewStateReached = true;
    } else if (command == "down") {
      changeState(DOWN_UNTIL_END, true);
      ignoreStateChangesUntilNewStateReached = true;
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

  if (i % 1000 == 0) {
    // Serial.print("POS: ");
    // Serial.println(handleDeg);
    //
    // Serial.print("TARGET DEG: ");
    // Serial.println(pos2deg(targetPosition));
    //
    // Serial.print("state: ");
    // Serial.println(currentState);

    // Serial.print("Total Error: ");
    // Serial.println(totalError, 10);

    // Serial.print("Handle Position: ");
    // Serial.println(updatedPosition);
    //
    // Serial.print("Handle (deg): ");
    // Serial.println(handleDeg);
    // Serial.print("Handle (rad): ");
    // Serial.println(handleRad);
    // Serial.print("Handle (cm): ");
    // Serial.println(handlePosition * 100.0);
    //
    // Serial.print("Velocity (m/s): ");
    // Serial.println(handleVelocity);
    //
    // Serial.print("Duty: ");
    // Serial.println(duty);
    //
    // Serial.print("Speed: ");
    // Serial.println(speed);
  }


  if (i % 50 == 0) {


    // Serial.print(handleDeg);
    // Serial.print(",");
    // Serial.print(handleVelocity);
    // Serial.print(",");
    // Serial.print(force);
    // Serial.print(",");
    // Serial.print(millis());
    // Serial.println();
  }

  motor.setSpeed(speed);
}
