#pragma once

enum buttonName {
  Up,
  Down,
  Left,
  Right,
  A,
  B,
  Start,
  Enter
};

struct Button {
  int pin;
  bool pressed;
  bool lastPressed;
};

// Declare the array and count as external references (no memory allocated here)
extern Button buttons[];
extern const int buttonCount;