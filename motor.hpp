enum Direction {
  LEFT,
  RIGHT,
};

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

class Motor {
  private:
    int pwmPin;
    int dirPin;

  public:
    Motor(int pwmPin, int dirPin) {
      this->pwmPin = pwmPin;
      this->dirPin = dirPin;

      setPwmFrequency(pwmPin, 1);

      this->stop();
      this->setDirection(LEFT);
    }

    void stop() {
      this->setSpeed(0);
    }

    void setSpeed(short speed) {
      analogWrite(this->pwmPin, abs(speed));

      if (speed > 0) {
        this->setDirection(LEFT);
      } else if (speed < 0) {
        this->setDirection(RIGHT);
      }
    }

    void setDirection(Direction dir) {
      if (dir == LEFT) {
        digitalWrite(this->dirPin, LOW);
      } else {
        digitalWrite(this->dirPin, HIGH);
      }
    }
};
