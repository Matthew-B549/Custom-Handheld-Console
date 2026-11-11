#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace FlappyGame {
TFT_eSPI tft = TFT_eSPI();
const uint16_t skyColor = 0x867D;
const int maxX = 319;
const int maxY = 239;
bool launcherExit = false;
bool onMenu = true;
bool gameOver = false;
bool quitConfirm = false;
bool needsDraw = true;
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
int birdX = 52;
int birdY = 112;
int birdV = 0;
int pipeX = 320;
int gapY = 96;
int score = 0;
int prevBirdX = 52;
int prevBirdY = 112;
int prevPipeX = 320;
int prevGapY = 96;
unsigned long lastFrame = 0;
unsigned long lastDebounce = 0;
const unsigned long frameMs = 50;
const unsigned long debounceMs = 160;

bool shouldExitToLauncher() { return launcherExit; }

void scanButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) buttons[i].pressed = !digitalRead(buttons[i].pin);
}

bool tapped(buttonName b) { return buttons[b].pressed && !buttons[b].lastPressed; }

void updateButtons() {
  for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
}

void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();

void fillClipped(int x, int y, int w, int h, uint16_t color) {
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > 320) w = 320 - x;
  if (y + h > 240) h = 240 - y;
  if (w > 0 && h > 0) tft.fillRect(x, y, w, h, color);
}

void drawMenu() {
  tft.fillScreen(skyColor);
  tft.setTextColor(TFT_YELLOW, skyColor);
  tft.setTextSize(3);
  tft.setCursor(52, 36);
  tft.print("FLAPPY BIRD");
  tft.setTextColor(TFT_BLACK, skyColor);
  tft.setTextSize(1);
  tft.setCursor(70, 98);
  tft.print("A / UP / ENTER flaps");
  tft.setCursor(70, 116);
  tft.print("Fly through the pipes");
  tft.setCursor(70, 134);
  tft.print("B asks to quit");
  tft.setTextSize(2);
  tft.setCursor(70, 190);
  tft.print("START TO PLAY");
}

void resetGame() {
  gameOver = false;
  quitConfirm = false;
  gamePaused = false;
pauseScreenDrawn = false;
resumeCountdownBoxDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  birdY = 112;
  birdV = 0;
  pipeX = 320;
  gapY = random(58, 160);
  score = 0;
  lastFrame = millis();
  prevBirdX = birdX;
  prevBirdY = birdY;
  prevPipeX = pipeX;
  prevGapY = gapY;
}

void drawQuit() {
  tft.fillRect(54, 76, 212, 88, TFT_BLACK);
  tft.drawRect(52, 74, 216, 92, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(104, 96);
  tft.print("QUIT?");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(88, 132);
  tft.print("ENTER quits   B cancels");
}

void returnToMenu() {
  launcherExit = true;
  gamePaused = false;
pauseScreenDrawn = false;
resumeCountdownBoxDrawn = false;
  resumeCountdownActive = false;
}

void drawFrame() {
  if (needsDraw) {
    tft.fillScreen(skyColor);
    needsDraw = false;
  } else {
    fillClipped(0, 0, 320, 20, skyColor);
    fillClipped(prevBirdX - 10, prevBirdY - 10, 28, 20, skyColor);
    fillClipped(prevPipeX, 0, 32, prevGapY - 38, skyColor);
    fillClipped(prevPipeX, prevGapY + 38, 32, 216 - prevGapY - 38, skyColor);
  }
  tft.fillRect(0, 216, 320, 24, TFT_GREEN);
  tft.fillRect(pipeX, 0, 32, gapY - 38, TFT_GREEN);
  tft.fillRect(pipeX, gapY + 38, 32, 216 - gapY - 38, TFT_GREEN);
  tft.fillCircle(birdX, birdY, 8, TFT_YELLOW);
  tft.fillTriangle(birdX + 7, birdY, birdX + 15, birdY - 4, birdX + 15, birdY + 4, TFT_ORANGE);
  tft.setTextColor(TFT_BLACK, skyColor);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.print(score);
  prevBirdX = birdX;
  prevBirdY = birdY;
  prevPipeX = pipeX;
  prevGapY = gapY;
}

void drawGameOver() {
  tft.fillRect(70, 78, 180, 82, TFT_BLACK);
  tft.drawRect(68, 76, 184, 86, TFT_RED);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(100, 96);
  tft.print("GAME OVER");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(100, 128);
  tft.print("START again");
}

void drawPauseOverlay() {
  tft.fillRect(88, 98, 144, 44, TFT_BLACK);
  tft.drawRect(86, 96, 148, 48, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(104, 112);
  tft.print("PAUSED");
  pauseScreenDrawn = true;
}

void drawResumeCountdown() {
  int boxW = 132;
  int boxH = 96;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  if (!resumeCountdownBoxDrawn) {
    tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(boxX + 24, boxY + 12);
    tft.print("RESUME");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(boxX + 28, boxY + 74);
    tft.print("A to resume");
    resumeCountdownBoxDrawn = true;
  }

  tft.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(boxX + 50, boxY + 36);
  tft.print(resumeCountdownValue);
}

void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownBoxDrawn = false;
  resumeCountdownValue = 3;
  resumeCountdownLastMs = millis();
  drawResumeCountdown();
}

bool updateResumeCountdown() {
  unsigned long now = millis();
  if (now - resumeCountdownLastMs >= 1000) {
    resumeCountdownLastMs += 1000;
    resumeCountdownValue--;
    if (resumeCountdownValue <= 0) {
      resumeCountdownActive = false;
      resumeCountdownBoxDrawn = false;
      return false;
    }
  }

  drawResumeCountdown();
  return true;
}

void setupGame() {
  launcherExit = false;
  FlappyGame::tft.init();
  FlappyGame::tft.setRotation(1);
  randomSeed(esp_random());
  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) pinMode(buttons[i].pin, INPUT_PULLUP);
  onMenu = true;
  needsDraw = true;
}

void loopGame() {
  FlappyGame::scanButtons();
  if (FlappyGame::onMenu) {
    if (FlappyGame::needsDraw) { FlappyGame::drawMenu(); FlappyGame::needsDraw = false; }
    if ((FlappyGame::tapped(Start) || FlappyGame::tapped(Enter)) && millis() - FlappyGame::lastDebounce > FlappyGame::debounceMs) {
      FlappyGame::lastDebounce = millis();
      FlappyGame::onMenu = false;
      FlappyGame::resetGame();
    }
    FlappyGame::updateButtons();
    delay(10);
    return;
  }
  if (FlappyGame::quitConfirm) {
    if (FlappyGame::tapped(B)) { FlappyGame::quitConfirm = false; FlappyGame::needsDraw = true; }
    if (FlappyGame::tapped(Enter)) FlappyGame::returnToMenu();
    FlappyGame::updateButtons();
    delay(10);
    return;
  }
  if (FlappyGame::tapped(Start) && millis() - FlappyGame::lastDebounce > FlappyGame::debounceMs) {
    FlappyGame::lastDebounce = millis();
    FlappyGame::gamePaused = !FlappyGame::gamePaused;
    if (!FlappyGame::gamePaused) {
      FlappyGame::pauseScreenDrawn = false;
      FlappyGame::startResumeCountdown();
    } else {
      FlappyGame::drawPauseOverlay();
    }
  }
  if (FlappyGame::resumeCountdownActive) {
    if (FlappyGame::updateResumeCountdown()) {
      FlappyGame::updateButtons();
      delay(10);
      return;
    }
  }
  if (FlappyGame::gamePaused) {
    if (!FlappyGame::pauseScreenDrawn) FlappyGame::drawPauseOverlay();
    FlappyGame::updateButtons();
    delay(10);
    return;
  }
  if (FlappyGame::tapped(B) && millis() - FlappyGame::lastDebounce > FlappyGame::debounceMs) {
    FlappyGame::lastDebounce = millis();
    FlappyGame::quitConfirm = true;
    FlappyGame::drawQuit();
    FlappyGame::updateButtons();
    return;
  }
  if (FlappyGame::gameOver) {
    if (FlappyGame::tapped(Start) || FlappyGame::tapped(Enter)) FlappyGame::resetGame();
    FlappyGame::updateButtons();
    delay(10);
    return;
  }
  if ((FlappyGame::tapped(A) || FlappyGame::tapped(Up) || FlappyGame::tapped(Enter)) && millis() - FlappyGame::lastDebounce > 80) {
    FlappyGame::lastDebounce = millis();
    FlappyGame::birdV = -6;
  }
  if (millis() - FlappyGame::lastFrame >= FlappyGame::frameMs) {
    FlappyGame::lastFrame = millis();
    FlappyGame::birdV++;
    FlappyGame::birdY += FlappyGame::birdV;
    FlappyGame::pipeX -= 4;
    if (FlappyGame::pipeX < -32) {
      FlappyGame::pipeX = 320;
      FlappyGame::gapY = random(58, 160);
      FlappyGame::score++;
    }
    if (FlappyGame::birdY < 8 || FlappyGame::birdY > 208) FlappyGame::gameOver = true;
    if (FlappyGame::birdX + 8 > FlappyGame::pipeX && FlappyGame::birdX - 8 < FlappyGame::pipeX + 32 && (FlappyGame::birdY - 8 < FlappyGame::gapY - 38 || FlappyGame::birdY + 8 > FlappyGame::gapY + 38)) FlappyGame::gameOver = true;
    FlappyGame::drawFrame();
    if (FlappyGame::gameOver) FlappyGame::drawGameOver();
  }
  FlappyGame::updateButtons();
  delay(8);
}

} // namespace FlappyGame
