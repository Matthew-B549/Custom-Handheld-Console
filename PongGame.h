#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"
namespace PongGame {
bool launcherExit = false;
bool shouldExitToLauncher() {
  return launcherExit;
}
const int dacPin = 25;

TFT_eSPI tft = TFT_eSPI();

const uint16_t backgroundColor = TFT_GREEN;
const uint16_t menuBackgroundColor = TFT_BLACK;
const int screenCenterX = 160;
const int screenCenterY = 120;
const int maxX = 319;
const int maxY = 239;

bool startImageDrawn = false;
bool byeImageDrawn = false;
bool onStartScreen = true;
bool onSettingsMenu = false;
bool inPongGame = false;
bool gameOver = false;
bool gamePaused = false;
bool courtDrawn = false;
bool menuNeedsRedraw = true;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;

const int paddleWidth = 5;
const int paddleHeight = 40;
const int playerX = 10;
const int computerX = maxX - 10 - paddleWidth;
const int ballSize = 5;

const int playerSpeed = 3;
const int minBallSpeed = 2;

int computerSpeed = 2;
int computerAimError = 45;
int maxBallSpeed = 8;
int winScore = 5;

const int menuItemCount = 3;
int selectedMenuItem = 0;
int difficultySetting = 0;
int winScoreSetting = 5;
int ballSpeedSetting = 0;

const char* difficultyLabels[] = {"Easy", "Med", "Hard"};
const char* ballSpeedLabels[] = {"Slow", "Med", "Fast"};

int playerY = screenCenterY - paddleHeight / 2;
int computerY = screenCenterY - paddleHeight / 2;
int playerOldY = -1;
int computerOldY = -1;
int ballX = screenCenterX - ballSize / 2;
int ballY = screenCenterY - ballSize / 2;
int ballVelX = minBallSpeed;
int ballVelY = 2;

// FIX: Changed sentinel value from -1 to -999 so off-screen negative positions don't trip safety checks
int oldBallX = -999;
int oldBallY = -999;

int playerScore = 0;
int computerScore = 0;
int lastPlayerScore = -1;
int lastComputerScore = -1;

unsigned long lastFrameMs = 0;
const unsigned long frameIntervalMs = 16;
const int hitPaddleSampleCount = 1133;
const unsigned long soundSampleIntervalUs = 125;
const unsigned long serveDelayMs = 500;
bool soundPlaying = false;
int soundSampleIndex = 0;
unsigned long lastSoundSampleUs = 0;

bool servePending = false;
bool pendingServeTowardRight = true;
unsigned long serveDelayStartMs = 0;
unsigned int computerMoveFrame = 0;

const unsigned long menuCooldownMs = 400;
const unsigned long menuReleaseHoldMs = 250;
unsigned long menuLastActionMs = 0;
unsigned long menuReleaseStartMs = 0;
bool menuActionLocked = false;
bool playerWonGame = false;
bool computerWonGame = false;
bool gameOverOverlayDrawn = false;
bool quitConfirm = false;

// Forward Declarations to guarantee compilation
void readButtons();
void updateButtonStates();
bool buttonPressed(buttonName button);
void resetMenuInputState();
void updateMenuInputGate();
bool menuCanAcceptInput();
bool menuButtonPressed(buttonName button);
void applySettingsFromMenu();
void decreaseSelectedSetting();
void increaseSelectedSetting();
void drawSlider(int x, int y, int width, int value, int minValue, int maxValue, bool selected);
void drawSettingsMenu();
void processSettingsMenu();
void startNewMatch();
void resetToTitle();
void togglePause();
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void drawQuitOverlay();
void cancelQuitOverlay();
void clearPauseOverlay();
void redrawActiveGame();
void drawCourt();
void redrawCenterLineInRect(int rx, int ry, int rw, int rh);
void clearBallArea(int x, int y);
void clearBallTrail(int fromX, int fromY, int toX, int toY);
void drawPaddle(int x, int y, uint16_t color);
void processPlayerMovement();
int predictBallYAtX(int targetX);
void processComputerMovement();
bool rectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
void handlePaddleHit(int paddleX, int paddleY, bool isLeftPaddle);
void processBallMovement();
void scheduleServe(bool towardRight);
void updateServeDelay();
void serveBall(bool towardRight);
void drawScore();
void endGame();
void displayStartImage();
void displayByeImage();
void drawGameOverOverlay();
void displayGameOverScreen();
void startCollisionSound();
void updateCollisionSound();

void setupGame() {
  PongGame::tft.init();
  PongGame::tft.setRotation(1);
  PongGame::tft.fillScreen(PongGame::backgroundColor);
  randomSeed(esp_random());
  PongGame::resetMenuInputState();

  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  PongGame::applySettingsFromMenu();
}

void loopGame() {
  readButtons();
  PongGame::updateCollisionSound();

  if (PongGame::onStartScreen) {
    PongGame::displayStartImage();
    PongGame::updateMenuInputGate();
    if (menuButtonPressed(Start)) {
      PongGame::onStartScreen = false;
      PongGame::onSettingsMenu = true;
      PongGame::menuNeedsRedraw = true;
      PongGame::resetMenuInputState();
      PongGame::tft.fillScreen(PongGame::menuBackgroundColor);
    }
    updateButtonStates();
    return;
  }

  if (PongGame::onSettingsMenu) {
    PongGame::processSettingsMenu();
    updateButtonStates();
    return;
  }

  if (PongGame::gameOver) {
    PongGame::displayGameOverScreen();
    PongGame::updateMenuInputGate();
    if (menuButtonPressed(Start)) {
      PongGame::resetToTitle();
    }
    updateButtonStates();
    return;
  }

  if (PongGame::inPongGame) {
    if (PongGame::quitConfirm) {
      if (buttonPressed(B)) {
        PongGame::cancelQuitOverlay();
      }
      if (buttonPressed(Enter)) {
        PongGame::resetToTitle();
      }
      updateButtonStates();
      return;
    }

    if (buttonPressed(B)) {
      PongGame::quitConfirm = true;
      PongGame::drawQuitOverlay();
      updateButtonStates();
      return;
    }

    if (buttonPressed(A)) {
      PongGame::togglePause();
    }

    if (PongGame::resumeCountdownActive) {
      if (PongGame::updateResumeCountdown()) {
        updateButtonStates();
        return;
      }
    }

    if (!PongGame::gamePaused) {
      PongGame::updateServeDelay();
      unsigned long now = millis();
      if (now - PongGame::lastFrameMs >= PongGame::frameIntervalMs) {
        PongGame::lastFrameMs = now;
        PongGame::processPlayerMovement();
        PongGame::processComputerMovement();
        if (!PongGame::servePending) {
          PongGame::processBallMovement();
        }
        PongGame::drawScore();
      }
    }
  }

  updateButtonStates();
}

void readButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

void updateButtonStates() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

bool buttonPressed(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed;
}

void resetMenuInputState() {
  PongGame::menuLastActionMs = 0;
  PongGame::menuReleaseStartMs = 0;
  PongGame::menuActionLocked = false;
}

void updateMenuInputGate() {
  bool anyDown = false;
  for (int i = 1; i < buttonCount; i++) {
    if (buttons[i].pressed) {
      anyDown = true;
      break;
    }
  }

  if (!anyDown) {
    if (PongGame::menuActionLocked) {
      if (PongGame::menuReleaseStartMs == 0) {
        PongGame::menuReleaseStartMs = millis();
      } else if (millis() - PongGame::menuReleaseStartMs >= PongGame::menuReleaseHoldMs) {
        PongGame::menuActionLocked = false;
        PongGame::menuReleaseStartMs = 0;
      }
    } else {
      PongGame::menuReleaseStartMs = 0;
    }
  } else {
    PongGame::menuReleaseStartMs = 0;
  }
}

bool menuCanAcceptInput() {
  if (PongGame::menuActionLocked) {
    return false;
  }
  if (millis() - PongGame::menuLastActionMs < PongGame::menuCooldownMs) {
    return false;
  }
  return true;
}

bool menuButtonPressed(buttonName button) {
  if (!(buttons[button].pressed && !buttons[button].lastPressed)) {
    return false;
  }

  PongGame::updateMenuInputGate();
  if (!PongGame::menuCanAcceptInput()) {
    return false;
  }

  PongGame::menuLastActionMs = millis();
  PongGame::menuActionLocked = true;
  return true;
}

void applySettingsFromMenu() {
  PongGame::winScore = PongGame::winScoreSetting;

  switch (PongGame::difficultySetting) {
    case 0:
      PongGame::computerSpeed = 1;
      PongGame::computerAimError = 50;
      break;
    case 1:
      PongGame::computerSpeed = 2;
      PongGame::computerAimError = 28;
      break;
    default:
      PongGame::computerSpeed = 2;
      PongGame::computerAimError = 12;
      break;
  }

  switch (PongGame::ballSpeedSetting) {
    case 0:
      PongGame::maxBallSpeed = 5;
      break;
    case 1:
      PongGame::maxBallSpeed = 7;
      break;
    default:
      PongGame::maxBallSpeed = 9;
      break;
  }
}

void decreaseSelectedSetting() {
  switch (PongGame::selectedMenuItem) {
    case 0:
      PongGame::difficultySetting = max(0, PongGame::difficultySetting - 1);
      break;
    case 1:
      PongGame::winScoreSetting = max(3, PongGame::winScoreSetting - 1);
      break;
    case 2:
      PongGame::ballSpeedSetting = max(0, PongGame::ballSpeedSetting - 1);
      break;
  }
}

void increaseSelectedSetting() {
  switch (PongGame::selectedMenuItem) {
    case 0:
      PongGame::difficultySetting = min(2, PongGame::difficultySetting + 1);
      break;
    case 1:
      PongGame::winScoreSetting = min(10, PongGame::winScoreSetting + 1);
      break;
    case 2:
      PongGame::ballSpeedSetting = min(2, PongGame::ballSpeedSetting + 1);
      break;
  }
}

void drawSlider(int x, int y, int width, int value, int minValue, int maxValue, bool selected) {
  uint16_t trackColor = selected ? TFT_DARKGREY : TFT_NAVY;
  uint16_t fillColor = selected ? TFT_YELLOW : TFT_WHITE;

  PongGame::tft.drawRect(x, y, width, 14, TFT_WHITE);
  PongGame::tft.fillRect(x + 1, y + 1, width - 2, 12, trackColor);
  int fillWidth = map(value - minValue, 0, maxValue - minValue, 0, width - 4);
  if (fillWidth > 0) {
    PongGame::tft.fillRect(x + 2, y + 3, fillWidth, 8, fillColor);
  }
}

void drawSettingsMenu() {
  PongGame::tft.fillScreen(PongGame::menuBackgroundColor);

  PongGame::tft.setTextColor(TFT_WHITE, PongGame::menuBackgroundColor);
  PongGame::tft.setTextSize(2);
  PongGame::tft.setCursor(72, 16);
  PongGame::tft.print("PONG SETUP");

  const int labelX = 24;
  const int sliderX = 24;
  const int sliderWidth = 180;
  const int valueX = 230;
  int rowY = 58;
  const int rowStep = 52;

  for (int i = 0; i < PongGame::menuItemCount; i++) {
    bool selected = i == PongGame::selectedMenuItem;
    uint16_t labelColor = selected ? TFT_YELLOW : TFT_WHITE;

    PongGame::tft.setTextColor(labelColor, PongGame::menuBackgroundColor);
    PongGame::tft.setTextSize(1);
    PongGame::tft.setCursor(labelX, rowY);
    switch (i) {
      case 0:
        PongGame::tft.print("Difficulty");
        PongGame::drawSlider(sliderX, rowY + 14, sliderWidth, PongGame::difficultySetting, 0, 2, selected);
        PongGame::tft.setCursor(valueX, rowY + 16);
        PongGame::tft.print(PongGame::difficultyLabels[PongGame::difficultySetting]);
        break;
      case 1:
        PongGame::tft.print("Win Score");
        PongGame::drawSlider(sliderX, rowY + 14, sliderWidth, PongGame::winScoreSetting, 3, 10, selected);
        PongGame::tft.setCursor(valueX, rowY + 16);
        PongGame::tft.print(PongGame::winScoreSetting);
        break;
      case 2:
        PongGame::tft.print("Ball Speed");
        PongGame::drawSlider(sliderX, rowY + 14, sliderWidth, PongGame::ballSpeedSetting, 0, 2, selected);
        PongGame::tft.setCursor(valueX, rowY + 16);
        PongGame::tft.print(PongGame::ballSpeedLabels[PongGame::ballSpeedSetting]);
        break;
    }

    rowY += rowStep;
  }

  PongGame::tft.setTextColor(TFT_DARKGREY, PongGame::menuBackgroundColor);
  PongGame::tft.setTextSize(1);
  PongGame::tft.setCursor(24, 210);
  PongGame::tft.print("UP/DOWN select   LEFT/RIGHT adjust");
  PongGame::tft.setCursor(24, 222);
  PongGame::tft.print("Release button between each press");
  PongGame::tft.setCursor(24, 234);
  PongGame::tft.print("START to play");
}

void processSettingsMenu() {
  bool changed = false;
  if (menuButtonPressed(Up)) {
    PongGame::selectedMenuItem = (PongGame::selectedMenuItem + PongGame::menuItemCount - 1) % PongGame::menuItemCount;
    changed = true;
  }
  if (menuButtonPressed(Down)) {
    PongGame::selectedMenuItem = (PongGame::selectedMenuItem + 1) % PongGame::menuItemCount;
    changed = true;
  }
  if (menuButtonPressed(Left)) {
    PongGame::decreaseSelectedSetting();
    changed = true;
  }
  if (menuButtonPressed(Right)) {
    PongGame::increaseSelectedSetting();
    changed = true;
  }
  if (menuButtonPressed(Start) || menuButtonPressed(Enter)) {
    PongGame::applySettingsFromMenu();
    PongGame::onSettingsMenu = false;
    PongGame::inPongGame = true;
    PongGame::startNewMatch();
    return;
  }

  PongGame::updateMenuInputGate();

  if (changed || PongGame::menuNeedsRedraw) {
    PongGame::drawSettingsMenu();
    PongGame::menuNeedsRedraw = false;
  }
}

void startNewMatch() {
  PongGame::tft.fillScreen(PongGame::backgroundColor);
  PongGame::courtDrawn = false;
  PongGame::gameOver = false;
  PongGame::gamePaused = false;
  PongGame::quitConfirm = false;
  PongGame::resumeCountdownActive = false;
  PongGame::byeImageDrawn = false;
  PongGame::playerScore = 0;
  PongGame::computerScore = 0;
  PongGame::lastPlayerScore = -1;
  PongGame::lastComputerScore = -1;
  PongGame::servePending = false;
  PongGame::soundPlaying = false;

  PongGame::playerY = PongGame::screenCenterY - PongGame::paddleHeight / 2;
  PongGame::computerY = PongGame::screenCenterY - PongGame::paddleHeight / 2;
  PongGame::playerOldY = -1;
  PongGame::computerOldY = -1;
  PongGame::oldBallX = -999; // FIX: Reset to -999 sentinel
  PongGame::oldBallY = -999; // FIX: Reset to -999 sentinel
  PongGame::computerMoveFrame = 0;

  PongGame::drawCourt();
  PongGame::drawPaddle(PongGame::playerX, PongGame::playerY, TFT_WHITE);
  PongGame::drawPaddle(PongGame::computerX, PongGame::computerY, TFT_WHITE);
  PongGame::serveBall(true);
}

void resetToTitle() {
  PongGame::launcherExit = true;
  PongGame::onStartScreen = false;
  PongGame::onSettingsMenu = true;
  PongGame::inPongGame = false;
  PongGame::gameOver = false;
  PongGame::gamePaused = false;
  PongGame::quitConfirm = false;
  PongGame::resumeCountdownActive = false;
  PongGame::startImageDrawn = false;
  PongGame::byeImageDrawn = false;
  PongGame::gameOverOverlayDrawn = false;
  PongGame::courtDrawn = false;
  PongGame::menuNeedsRedraw = true;
  PongGame::resetMenuInputState();
  PongGame::tft.fillScreen(PongGame::menuBackgroundColor);
}

void togglePause() {
  PongGame::gamePaused = !PongGame::gamePaused;
  if (PongGame::gamePaused) {
    PongGame::drawPauseOverlay();
  } else {
    PongGame::clearPauseOverlay();
    PongGame::startResumeCountdown();
  }
}

void drawPauseOverlay() {
  PongGame::tft.fillRect(88, 98, 144, 44, TFT_BLACK);
  PongGame::tft.drawRect(86, 96, 148, 48, TFT_WHITE);
  PongGame::tft.setTextColor(TFT_WHITE, TFT_BLACK);
  PongGame::tft.setTextSize(2);
  PongGame::tft.setCursor(104, 112);
  PongGame::tft.print("PAUSED");
}

void drawResumeCountdown() {
  int boxW = 132;
  int boxH = 96;
  int boxX = (PongGame::maxX + 1 - boxW) / 2;
  int boxY = (PongGame::maxY + 1 - boxH) / 2;

  if (!PongGame::resumeCountdownBoxDrawn) {
    PongGame::tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    PongGame::tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    PongGame::tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    PongGame::tft.setTextSize(2);
    PongGame::tft.setCursor(boxX + 24, boxY + 12);
    PongGame::tft.print("RESUME");
    PongGame::tft.setTextColor(TFT_WHITE, TFT_BLACK);
    PongGame::tft.setTextSize(1);
    PongGame::tft.setCursor(boxX + 28, boxY + 74);
    PongGame::tft.print("A to resume");
    PongGame::resumeCountdownBoxDrawn = true;
  }

  PongGame::tft.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
  PongGame::tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  PongGame::tft.setTextSize(4);
  PongGame::tft.setCursor(boxX + 50, boxY + 36);
  PongGame::tft.print(PongGame::resumeCountdownValue);
}

void startResumeCountdown() {
  PongGame::resumeCountdownActive = true;
  PongGame::resumeCountdownBoxDrawn = false;
  PongGame::resumeCountdownValue = 3;
  PongGame::resumeCountdownLastMs = millis();
  PongGame::drawResumeCountdown();
}

bool updateResumeCountdown() {
  unsigned long now = millis();
  if (now - PongGame::resumeCountdownLastMs >= 1000) {
    PongGame::resumeCountdownLastMs += 1000;
    PongGame::resumeCountdownValue--;
    if (PongGame::resumeCountdownValue <= 0) {
      PongGame::resumeCountdownActive = false;
      PongGame::resumeCountdownBoxDrawn = false;
      return false;
    }
  }

  PongGame::drawResumeCountdown();
  return true;
}

void drawQuitOverlay() {
  PongGame::tft.fillRect(54, 78, 212, 84, TFT_BLACK);
  PongGame::tft.drawRect(52, 76, 216, 88, TFT_YELLOW);
  PongGame::tft.drawRect(54, 78, 212, 84, TFT_WHITE);
  PongGame::tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  PongGame::tft.setTextSize(2);
  PongGame::tft.setCursor(102, 96);
  PongGame::tft.print("QUIT?");
  PongGame::tft.setTextColor(TFT_WHITE, TFT_BLACK);
  PongGame::tft.setTextSize(1);
  PongGame::tft.setCursor(88, 130);
  PongGame::tft.print("ENTER quits   B cancels");
}

void cancelQuitOverlay() {
  PongGame::quitConfirm = false;
  PongGame::redrawActiveGame();
  if (PongGame::gamePaused) {
    PongGame::drawPauseOverlay();
  }
}

void clearPauseOverlay() {
  PongGame::tft.fillRect(86, 96, 148, 48, PongGame::backgroundColor);
  PongGame::redrawCenterLineInRect(86, 96, 148, 48);
}

void redrawActiveGame() {
  PongGame::drawPaddle(PongGame::playerX, PongGame::playerY, TFT_WHITE);
  PongGame::drawPaddle(PongGame::computerX, PongGame::computerY, TFT_WHITE);

  if (!PongGame::servePending) {
    PongGame::tft.fillRect(PongGame::ballX, PongGame::ballY, PongGame::ballSize, PongGame::ballSize, TFT_RED);
  }

  PongGame::lastPlayerScore = -1;
  PongGame::lastComputerScore = -1;
  PongGame::drawScore();
}

void drawCourt() {
  if (PongGame::courtDrawn) {
    return;
  }

  for (int y = 0; y <= PongGame::maxY; y += 12) {
    PongGame::tft.fillRect(PongGame::screenCenterX, y, 2, 8, TFT_DARKGREEN);
  }

  PongGame::courtDrawn = true;
}

void redrawCenterLineInRect(int rx, int ry, int rw, int rh) {
  const int lineX = PongGame::screenCenterX;
  const int lineW = 2;

  if (rx + rw <= lineX || rx >= lineX + lineW) {
    return;
  }

  for (int dashY = 0; dashY <= PongGame::maxY; dashY += 12) {
    const int dashH = 8;
    if (ry < dashY + dashH && ry + rh > dashY) {
      PongGame::tft.fillRect(lineX, dashY, lineW, dashH, TFT_DARKGREEN);
    }
  }
}

void clearBallArea(int x, int y) {
  PongGame::tft.fillRect(x, y, PongGame::ballSize, PongGame::ballSize, PongGame::backgroundColor);
  PongGame::redrawCenterLineInRect(x, y, PongGame::ballSize, PongGame::ballSize);
}

void clearBallTrail(int fromX, int fromY, int toX, int toY) {
  // FIX: Look specifically for our -999 sentinel. Valid negative positions near 0 are now allowed.
  if (fromX == -999 || fromY == -999) {
    return;
  }

  int left = min(fromX, toX);
  int top = min(fromY, toY);
  int right = max(fromX + PongGame::ballSize, toX + PongGame::ballSize);
  int bottom = max(fromY + PongGame::ballSize, toY + PongGame::ballSize);
  left = max(0, left - 1);
  top = max(0, top - 1);
  right = min(PongGame::maxX + 1, right + 1);
  bottom = min(PongGame::maxY + 1, bottom + 1);

  int width = right - left;
  int height = bottom - top;
  if (width <= 0 || height <= 0) {
    return;
  }

  PongGame::tft.fillRect(left, top, width, height, PongGame::backgroundColor);
  PongGame::redrawCenterLineInRect(left, top, width, height);
}

void drawPaddle(int x, int y, uint16_t color) {
  PongGame::tft.fillRect(x, y, PongGame::paddleWidth, PongGame::paddleHeight, color);
}

void processPlayerMovement() {
  PongGame::playerOldY = PongGame::playerY;

  if (buttons[Up].pressed && PongGame::playerY > 0) {
    PongGame::playerY -= PongGame::playerSpeed;
  }
  if (buttons[Down].pressed && PongGame::playerY < PongGame::maxY - PongGame::paddleHeight + 1) {
    PongGame::playerY += PongGame::playerSpeed;
  }

  PongGame::playerY = constrain(PongGame::playerY, 0, PongGame::maxY - PongGame::paddleHeight + 1);
  if (PongGame::playerOldY >= 0 && PongGame::playerOldY != PongGame::playerY) {
    PongGame::tft.fillRect(PongGame::playerX, PongGame::playerOldY, PongGame::paddleWidth, PongGame::paddleHeight, PongGame::backgroundColor);
    PongGame::drawPaddle(PongGame::playerX, PongGame::playerY, TFT_WHITE);
  }
}

int predictBallYAtX(int targetX) {
  int simulatedX = PongGame::ballX;
  int simulatedY = PongGame::ballY;
  int velX = PongGame::ballVelX;
  int velY = PongGame::ballVelY;

  if (velX == 0) {
    return simulatedY;
  }

  for (int step = 0; step < 400; step++) {
    simulatedX += velX;
    simulatedY += velY;

    if (simulatedY <= 0) {
      simulatedY = 0;
      velY = abs(velY);
    } else if (simulatedY >= PongGame::maxY - PongGame::ballSize + 1) {
      simulatedY = PongGame::maxY - PongGame::ballSize + 1;
      velY = -abs(velY);
    }

    if ((velX > 0 && simulatedX + PongGame::ballSize >= targetX) ||
        (velX < 0 && simulatedX <= targetX)) {
      return simulatedY;
    }
  }

  return simulatedY;
}

void processComputerMovement() {
  PongGame::computerMoveFrame++;

  int reactionInterval = 1;
  if (PongGame::difficultySetting == 0) {
    reactionInterval = 3;
  } else if (PongGame::difficultySetting == 1) {
    reactionInterval = 2;
  }

  if (PongGame::computerMoveFrame % reactionInterval != 0) {
    return;
  }

  PongGame::computerOldY = PongGame::computerY;
  int targetY;
  if (PongGame::ballVelX > 0) {
    int predictedY = PongGame::predictBallYAtX(PongGame::computerX);
    predictedY += random(-PongGame::computerAimError, PongGame::computerAimError + 1);
    targetY = predictedY + PongGame::ballSize / 2 - PongGame::paddleHeight / 2;
  } else {
    targetY = PongGame::screenCenterY - PongGame::paddleHeight / 2;
  }

  targetY = constrain(targetY, 0, PongGame::maxY - PongGame::paddleHeight + 1);

  int moveDeadZone = (PongGame::difficultySetting == 0) ? 6 : 3;
  if (abs(PongGame::computerY - targetY) > moveDeadZone) {
    if (PongGame::computerY < targetY) {
      PongGame::computerY += PongGame::computerSpeed;
    } else {
      PongGame::computerY -= PongGame::computerSpeed;
    }
  }

  PongGame::computerY = constrain(PongGame::computerY, 0, PongGame::maxY - PongGame::paddleHeight + 1);
  if (PongGame::computerOldY >= 0 && PongGame::computerOldY != PongGame::computerY) {
    PongGame::tft.fillRect(PongGame::computerX, PongGame::computerOldY, PongGame::paddleWidth, PongGame::paddleHeight, PongGame::backgroundColor);
    PongGame::drawPaddle(PongGame::computerX, PongGame::computerY, TFT_WHITE);
  }
}

bool rectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void handlePaddleHit(int paddleX, int paddleY, bool isLeftPaddle) {
  int paddleCenter = paddleY + PongGame::paddleHeight / 2;
  int ballCenter = PongGame::ballY + PongGame::ballSize / 2;
  int hitOffset = ballCenter - paddleCenter;

  PongGame::ballVelY += hitOffset / 4;
  if (PongGame::ballVelY == 0) {
    PongGame::ballVelY = hitOffset >= 0 ? 1 : -1;
  }
  PongGame::ballVelY = constrain(PongGame::ballVelY, -PongGame::maxBallSpeed, PongGame::maxBallSpeed);

  int nextSpeed = abs(PongGame::ballVelX);
  if (random(0, 3) == 0) {
    nextSpeed = min(abs(PongGame::ballVelX) + 1, PongGame::maxBallSpeed);
  }
  PongGame::ballVelX = isLeftPaddle ? nextSpeed : -nextSpeed;

  if (isLeftPaddle) {
    PongGame::ballX = paddleX + PongGame::paddleWidth;
  } else {
    PongGame::ballX = paddleX - PongGame::ballSize;
  }

  PongGame::startCollisionSound();
}

void processBallMovement() {
  PongGame::oldBallX = PongGame::ballX;
  PongGame::oldBallY = PongGame::ballY;

  PongGame::ballX += PongGame::ballVelX;
  PongGame::ballY += PongGame::ballVelY;

  if (PongGame::ballY <= 0) {
    PongGame::ballY = 0;
    PongGame::ballVelY = abs(PongGame::ballVelY);
  } else if (PongGame::ballY >= PongGame::maxY - PongGame::ballSize + 1) {
    PongGame::ballY = PongGame::maxY - PongGame::ballSize + 1;
    PongGame::ballVelY = -abs(PongGame::ballVelY);
  }

  if (PongGame::ballVelX < 0 &&
      PongGame::rectsOverlap(PongGame::ballX, PongGame::ballY, PongGame::ballSize, PongGame::ballSize, PongGame::playerX, PongGame::playerY, PongGame::paddleWidth, PongGame::paddleHeight)) {
    PongGame::handlePaddleHit(PongGame::playerX, PongGame::playerY, true);
  } else if (PongGame::ballVelX > 0 &&
             PongGame::rectsOverlap(PongGame::ballX, PongGame::ballY, PongGame::ballSize, PongGame::ballSize, PongGame::computerX, PongGame::computerY, PongGame::paddleWidth, PongGame::paddleHeight)) {
    PongGame::handlePaddleHit(PongGame::computerX, PongGame::computerY, false);
  }

  if (PongGame::ballX + PongGame::ballSize < 0) {
    PongGame::computerScore++;
    PongGame::clearBallTrail(PongGame::oldBallX, PongGame::oldBallY, PongGame::ballX, PongGame::ballY);
    if (PongGame::computerScore >= PongGame::winScore) {
      PongGame::endGame();
      return;
    }
    PongGame::scheduleServe(true);
  } else if (PongGame::ballX > PongGame::maxX) {
    PongGame::playerScore++;
    PongGame::clearBallTrail(PongGame::oldBallX, PongGame::oldBallY, PongGame::ballX, PongGame::ballY);
    if (PongGame::playerScore >= PongGame::winScore) {
      PongGame::endGame();
      return;
    }
    PongGame::scheduleServe(false);
  } else {
    PongGame::clearBallTrail(PongGame::oldBallX, PongGame::oldBallY, PongGame::ballX, PongGame::ballY);
    PongGame::tft.fillRect(PongGame::ballX, PongGame::ballY, PongGame::ballSize, PongGame::ballSize, TFT_RED);
  }
}

void scheduleServe(bool towardRight) {
  PongGame::servePending = true;
  PongGame::pendingServeTowardRight = towardRight;
  PongGame::serveDelayStartMs = millis();
}

void updateServeDelay() {
  if (!PongGame::servePending || PongGame::gamePaused) {
    return;
  }

  if (millis() - PongGame::serveDelayStartMs >= PongGame::serveDelayMs) {
    PongGame::servePending = false;
    PongGame::serveBall(PongGame::pendingServeTowardRight);
  }
}

void serveBall(bool towardRight) {
  // FIX: Look for our -999 sentinel
  if (PongGame::oldBallX != -999 && PongGame::oldBallY != -999) {
    PongGame::clearBallTrail(PongGame::oldBallX, PongGame::oldBallY, PongGame::ballX, PongGame::ballY);
  }

  PongGame::ballX = PongGame::screenCenterX - PongGame::ballSize / 2;
  PongGame::ballY = PongGame::screenCenterY - PongGame::ballSize / 2;
  PongGame::ballVelX = towardRight ? PongGame::minBallSpeed : -PongGame::minBallSpeed;
  PongGame::ballVelY = random(0, 2) == 0 ? 2 : -2;
  PongGame::oldBallX = -999; // FIX: Reset to -999 sentinel
  PongGame::oldBallY = -999; // FIX: Reset to -999 sentinel
}

void drawScore() {
  if (PongGame::playerScore == PongGame::lastPlayerScore && PongGame::computerScore == PongGame::lastComputerScore) {
    return;
  }

  PongGame::tft.fillRect(PongGame::screenCenterX - 40, 4, 80, 16, PongGame::backgroundColor);
  PongGame::tft.setTextColor(TFT_WHITE, PongGame::backgroundColor);
  PongGame::tft.setTextSize(2);
  PongGame::tft.setCursor(PongGame::screenCenterX - 36, 4);
  PongGame::tft.print(PongGame::playerScore);
  PongGame::tft.print(" - ");
  PongGame::tft.print(PongGame::computerScore);

  PongGame::lastPlayerScore = PongGame::playerScore;
  PongGame::lastComputerScore = PongGame::computerScore;
}

void endGame() {
  PongGame::playerWonGame = PongGame::playerScore >= PongGame::winScore;
  PongGame::computerWonGame = PongGame::computerScore >= PongGame::winScore;
  PongGame::inPongGame = false;
  PongGame::gameOver = true;
  PongGame::gamePaused = false;
  PongGame::resumeCountdownActive = false;
  PongGame::byeImageDrawn = false;
  PongGame::gameOverOverlayDrawn = false;
}

void displayStartImage() {
  if (PongGame::startImageDrawn) {
    return;
  }

  for (int y = 0; y < 120; y++) {
    for (int x = 0; x < 160; x++) {
      PongGame::tft.fillRect(2 * x, 2 * y, 2, 2, startImage[y][x]);
    }
  }
  PongGame::startImageDrawn = true;
}

void displayByeImage() {
  if (PongGame::byeImageDrawn) {
    return;
  }

  for (int y = 0; y < 120; y++) {
    for (int x = 0; x < 160; x++) {
      PongGame::tft.fillRect(2 * x, 2 * y, 2, 2, byeImage[y][x]);
    }
  }
  PongGame::byeImageDrawn = true;
}

void drawGameOverOverlay() {
  if (PongGame::gameOverOverlayDrawn) {
    return;
  }

  PongGame::tft.fillRect(0, 168, 320, 72, TFT_BLACK);
  PongGame::tft.drawRect(0, 168, 320, 72, TFT_WHITE);

  PongGame::tft.setTextColor(TFT_WHITE, TFT_BLACK);
  PongGame::tft.setTextSize(1);
  PongGame::tft.setCursor(108, 176);
  PongGame::tft.print("FINAL SCORE");

  PongGame::tft.setTextSize(2);
  PongGame::tft.setCursor(118, 192);
  PongGame::tft.print(PongGame::playerScore);
  PongGame::tft.print(" - ");
  PongGame::tft.print(PongGame::computerScore);

  PongGame::tft.setTextSize(1);
  if (PongGame::playerWonGame) {
    PongGame::tft.setCursor(118, 212);
    PongGame::tft.print("YOU WIN!");
  } else if (PongGame::computerWonGame) {
    PongGame::tft.setCursor(110, 212);
    PongGame::tft.print("YOU LOSE");
  } else {
    PongGame::tft.setCursor(128, 212);
    PongGame::tft.print("TIE GAME");
  }

  PongGame::tft.setCursor(62, 220);
  PongGame::tft.print("Press START for menu");
  PongGame::tft.setCursor(48, 232);
  PongGame::tft.print("Then START again to play");

  PongGame::gameOverOverlayDrawn = true;
}

void displayGameOverScreen() {
  PongGame::displayByeImage();
  PongGame::drawGameOverOverlay();
}

void startCollisionSound() {
  PongGame::soundPlaying = true;
  PongGame::soundSampleIndex = 0;
  PongGame::lastSoundSampleUs = micros();
}

void updateCollisionSound() {
  if (!PongGame::soundPlaying) {
    return;
  }

  unsigned long now = micros();
  while (PongGame::soundPlaying && now - PongGame::lastSoundSampleUs >= PongGame::soundSampleIntervalUs) {
    if (PongGame::soundSampleIndex >= PongGame::hitPaddleSampleCount) {
      dacWrite(PongGame::dacPin, 0);
      dacDisable(PongGame::dacPin);
      PongGame::soundPlaying = false;
      return;
    }

    dacWrite(PongGame::dacPin, pgm_read_byte(&hitPaddle_data[PongGame::soundSampleIndex]));
    PongGame::soundSampleIndex++;
    PongGame::lastSoundSampleUs += PongGame::soundSampleIntervalUs;
  }
} // FIX: Removed the extra trailing brace here that broke global compilation scope

} // namespace PongGame
