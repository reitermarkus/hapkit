#pragma once

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

    const unsigned long vibrationTime = (maxVibrationLength - min(currentTime - wallEntryTime, maxVibrationLength));

    const double vibrationTimeRatio = (vibrationTime / maxVibrationLength);

    const double cosInput = currentTime / 1000.0 * 90.0;

    vibrationForce = cos(cosInput) * vibrationTimeRatio;
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
