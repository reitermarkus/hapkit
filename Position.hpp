class Position {
  private:
    const int FLIP_THRESHOLD = 700;

  public:
    int sensorPin;
    int currentPosition;
    bool justFlipped = false;
    int flips = 0;
    int updatedPosition;
    int initialPosition;
    int minValue = 0;
    int maxValue = 1000;

    Position(int sensorPin, int minValue, int maxValue) {
      this->sensorPin = sensorPin;
      this->minValue = minValue;
      this->maxValue = maxValue;
    }

    int read() {
      int value = analogRead(sensorPin);

      // Recalibrate on-the-fly.
      if (value < this->minValue) {
        this->minValue = value;
      } else if (value > this->maxValue) {
        this->maxValue = value;
      }

      return value;
    }

    void init() {
      this->currentPosition = read();
      this->updatedPosition = currentPosition;
      this->initialPosition = updatedPosition;
    }

    void update() {
      int maxDistance = this->maxValue - this->minValue;
      int threshold = maxDistance * 0.3;

      int newPosition = read();
      int currentDifference = newPosition - currentPosition;
      int currentDistance = abs(currentDifference);

      // Update tracking variables.
      currentPosition = newPosition;

      if (currentDistance > threshold && !justFlipped) {
        if (currentDifference > 0) {
          // Clockwise rotation.
          flips--;
        } else {
          // Counter-clockwise rotation.
          flips++;
        }

        justFlipped = true;
      } else {
        justFlipped = false;
      }

      updatedPosition = newPosition + flips * (maxValue - minValue) - initialPosition;
    }
};
