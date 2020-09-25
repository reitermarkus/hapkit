#pragma once

// Control & Sensor Pins
int M1 = 12; int ENA = 3;
int M2 = 13; int ENB = 11;

// Hapkit data.
const double HANDLE_RADIUS = 0.065659; // m
const double PULLEY_RADIUS = 0.004191; // m
const double S_RADIUS = 0.073152; // m

// These were found be reading the sensor repeatedly
// and calculating the min/max values.
const int MIN_VALUE = 48;
const int MAX_VALUE = 971;
const float POSITION_10_DEGREE_DIFFERENCE = 726.6;

// Constants for the wall/trigger positions of the
// garage door/projection screen controller.
double DOWN_UNTIL_END_POSITION = 30.0;
double UP_UNTIL_END_POSITION = -DOWN_UNTIL_END_POSITION;
double DOWN_WHILE_PRESSED_POSITION = 5.0;
double UP_WHILE_PRESSED_POSITION = -DOWN_WHILE_PRESSED_POSITION;
