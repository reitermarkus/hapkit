class Position {
  private:
    const int FLIP_THRESHOLD = 700;

  public:
    int sensorPin;
    int lastPosition;
    int currentRawPosition;
    bool justFlipped = false;
    int flips = 0;
    int tempDistance = 0;
    int updatedPosition;
    int initialPosition = 0;

    Position(int sensorPin) {
      this->sensorPin = sensorPin;
      this->lastPosition = read();
      this->currentRawPosition = lastPosition;
      this->updatedPosition = currentRawPosition;
      this->initialPosition = updatedPosition;
      this->tempDistance = initialPosition;
    }

    int read() {
      return analogRead(sensorPin);
    }

    void update() {
      int newPosition = read();
      int lastDifference = newPosition - lastPosition;
      int lastDistance = abs(lastDifference);
      int currentDifference = newPosition - currentRawPosition;
      int currentDistance = abs(currentDifference);

      // Update tracking variables.
      lastPosition = currentRawPosition;
      currentRawPosition = newPosition;

      if (lastDistance > FLIP_THRESHOLD && !justFlipped) {
        if (lastDifference > 0) {
          // Clockwise rotation.
          flips--;
        } else {
          // Counter-clockwise rotation.
          flips++;
        }

        if (currentDistance > FLIP_THRESHOLD) {
          tempDistance = currentDistance;
          updatedPosition = newPosition + flips * tempDistance;
        } else {
          tempDistance = lastDistance;
          updatedPosition = newPosition + flips * tempDistance;
        }

        justFlipped = true;
      } else {
        updatedPosition = newPosition + flips * tempDistance;
        justFlipped = false;
      }
    }
};
