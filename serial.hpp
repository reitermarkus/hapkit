#pragma once

static String command;

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
