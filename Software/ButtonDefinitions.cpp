#include "ButtonDefinitions.h"

// Define the array and allocate memory ONCE
Button buttons[] = {
  { 17, false, false },
  { 5,  false, false },
  { 16, false, false },
  { 18, false, false },
  { 22, false, false },
  { 21, false, false },
  { 23, false, false },
  { 19, false, false }
};

const int buttonCount = sizeof(buttons) / sizeof(buttons[0]);