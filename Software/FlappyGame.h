#pragma once
#include <Arduino.h>
#include <SD.h>                  // <-- Add SD library
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"    // <-- Add HighScoreManager
#include "SoundManager.h"        // <-- Add SoundManager Integration

namespace FlappyGame {
const int sdChipSelect = 5;      // <-- Add your SD CS pin (same as Tetris)
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
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
bool endScreenShown = false;     // <-- Add tracker to prevent rapid SD writing
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

bool shouldExitToLauncher() {
  return launcherExit;
}

void scanButtons() {
  for (int i = 0; i < buttonCount; i++) buttons[i].pressed = !digitalRead(buttons[i].pin);
}

bool tapped(buttonName b) {
  return buttons[b].pressed && !buttons[b].lastPressed;
}

void updateButtons() {
  for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
}

void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();

void drawMenu() {
  spr.fillSprite(skyColor);
  spr.setTextColor(TFT_YELLOW, skyColor);
  spr.setTextSize(3);
  spr.setCursor(44, 40);
  spr.print("FLAPPY BIRD");
  spr.setTextColor(TFT_WHITE, skyColor);
  spr.setTextSize(1);
  spr.setCursor(60, 100);
  spr.print("A / UP / ENTER to flap");
  spr.setCursor(60, 118);
  spr.print("Avoid the green pipes!");
  spr.setCursor(60, 136);
  spr.print("B asks to quit");
  spr.setTextSize(2);
  spr.setCursor(60, 190);
  spr.print("START TO PLAY");
}

void resetGame() {
  gameOver = false;
  endScreenShown = false;
  quitConfirm = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownBoxDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  birdY = 112;
  birdV = 0;
  pipeX = 320;
  gapY = 70 + random(0, 60);
  score = 0;
  prevBirdY = 112;
  prevPipeX = 320;
  prevGapY = gapY;
  lastFrame = millis();
  
  // Play game start fanfare
  SoundManager::play(SoundManager::SOUND_GAME_START);
}

void drawQuit() {
  spr.fillRect(54, 76, 212, 88, TFT_BLACK);
  spr.drawRect(52, 74, 216, 92, TFT_YELLOW);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(104, 96);
  spr.print("QUIT?");
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(88, 132);
  spr.print("ENTER quits   B cancels");
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
    spr.fillSprite(skyColor);
    needsDraw = false;
  } else {
    int clearBirdY = min(prevBirdY, birdY);
    int clearBirdH = max(prevBirdY + 12, birdY + 12) - clearBirdY;
    spr.fillRect(birdX - 2, clearBirdY - 2, 20, clearBirdH + 4, skyColor);

    int clearPipeX = min(prevPipeX, pipeX);
    int clearPipeW = max(prevPipeX + 32, pipeX + 32) - clearPipeX;
    spr.fillRect(clearPipeX - 4, 0, clearPipeW + 8, prevGapY, skyColor);
    spr.fillRect(clearPipeX - 4, prevGapY + 64, clearPipeW + 8, 240 - (prevGapY + 64), skyColor);
  }

  // Draw Top Pipe
  spr.fillRect(pipeX, 0, 32, gapY, TFT_GREEN);
  spr.drawRect(pipeX, 0, 32, gapY, TFT_BLACK);
  spr.fillRect(pipeX - 2, gapY - 12, 36, 12, TFT_GREEN);
  spr.drawRect(pipeX - 2, gapY - 12, 36, 12, TFT_BLACK);

  // Draw Bottom Pipe
  int botY = gapY + 64;
  spr.fillRect(pipeX, botY, 32, 240 - botY, TFT_GREEN);
  spr.drawRect(pipeX, botY, 32, 240 - botY, TFT_BLACK);
  spr.fillRect(pipeX - 2, botY, 36, 12, TFT_GREEN);
  spr.drawRect(pipeX - 2, botY, 36, 12, TFT_BLACK);

  // Draw Bird (Yellow body, red beak, white eye)
  spr.fillCircle(birdX + 8, birdY + 6, 6, TFT_YELLOW);
  spr.drawCircle(birdX + 8, birdY + 6, 6, TFT_BLACK);
  spr.fillRect(birdX + 11, birdY + 4, 4, 3, TFT_RED); // Beak
  spr.fillCircle(birdX + 10, birdY + 4, 1, TFT_WHITE); // Eye

  spr.setTextColor(TFT_WHITE, skyColor);
  spr.setTextSize(2);
  spr.setCursor(8, 8);
  spr.print(score);
}

void drawGameOver() {
  SD.begin(sdChipSelect);
  int rank = HighScoreManager::saveScore("flappy", score);

  spr.fillRect(66, 66, 188, 112, TFT_BLACK);
  spr.drawRect(64, 64, 192, 116, TFT_RED);
  
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(94, 76);
  spr.print("GAME OVER");
  
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(100, 104);
  spr.print("SCORE: ");
  spr.print(score);

  unsigned long highScores[HighScoreManager::MAX_SCORES];
  int count = HighScoreManager::loadScores("flappy", highScores);
  if (count > 0) {
    spr.setCursor(100, 120);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.print("BEST:  ");
    spr.print(highScores[0]);
  }

  if (rank > 0 && rank <= HighScoreManager::MAX_SCORES) {
    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.setCursor(100, 136);
    spr.print("RANK:  #");
    spr.print(rank);
    spr.print("!");
  }

  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(102, 156);
  spr.print("START again");
}

void drawPauseOverlay() {
  spr.fillRect(88, 98, 144, 44, TFT_BLACK);
  spr.drawRect(86, 96, 148, 48, TFT_WHITE);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(104, 112);
  spr.print("PAUSED");
  pauseScreenDrawn = true;
}

void drawResumeCountdown() {
  int boxW = 132;
  int boxH = 96;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  if (!resumeCountdownBoxDrawn) {
    spr.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    spr.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    spr.setTextColor(TFT_YELLOW, TFT_BLACK);
    spr.setTextSize(2);
    spr.setCursor(boxX + 24, boxY + 12);
    spr.print("RESUME");
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextSize(1);
    spr.setCursor(boxX + 28, boxY + 74);
    spr.print("A to resume");
    resumeCountdownBoxDrawn = true;
  }

  spr.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(4);
  spr.setCursor(boxX + 50, boxY + 36);
  spr.print(resumeCountdownValue);
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
    drawResumeCountdown();
    SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Countdown beep
  }
  return true;
}

void setupGame() {
  launcherExit = false;
  tft.init();
  tft.setRotation(3);
  spr.setColorDepth(8);
  spr.createSprite(320, 240);
  randomSeed(esp_random());
  for (int i = 0; i < buttonCount; i++) pinMode(buttons[i].pin, INPUT_PULLUP);
  onMenu = true;
  needsDraw = true;
}

void loopGame() {
  scanButtons();

  if (quitConfirm) {
    if (tapped(B)) {
      quitConfirm = false;
      needsDraw = true;
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
    }
    if (tapped(Enter)) {
      SoundManager::play(SoundManager::SOUND_MENU_SELECT);
      returnToMenu();
    }
    updateButtons();
    delay(10);
    return;
  }

  if (tapped(B) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    quitConfirm = true;
    drawQuit();
    spr.pushSprite(0, 0);
    SoundManager::play(SoundManager::SOUND_MENU_MOVE);
    updateButtons();
    return;
  }

  if (onMenu) {
    if (needsDraw) {
      drawMenu();
      spr.pushSprite(0, 0);
      needsDraw = false;
    }
    if ((tapped(Start) || tapped(Enter)) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      onMenu = false;
      resetGame();
    }
    updateButtons();
    delay(10);
    return;
  }

  if (tapped(A) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      needsDraw = true;
      startResumeCountdown();
    } else {
      drawPauseOverlay();
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
    }
  }

  if (resumeCountdownActive) {
    if (updateResumeCountdown()) {
      spr.pushSprite(0, 0);
      updateButtons();
      delay(10);
      return;
    }
  }

  if (gamePaused) {
    if (!pauseScreenDrawn) drawPauseOverlay();
    spr.pushSprite(0, 0);
    updateButtons();
    delay(10);
    return;
  }

  if (gameOver) {
    if (!endScreenShown) {
      drawGameOver();       
      endScreenShown = true;
      SoundManager::play(SoundManager::SOUND_GAME_OVER);
    }
    if (tapped(Start) || tapped(Enter)) resetGame();
    spr.pushSprite(0, 0);
    updateButtons();
    delay(10);
    return;
  }

  if ((tapped(A) || tapped(Up) || tapped(Enter)) && millis() - lastDebounce > 80) {
    lastDebounce = millis();
    birdV = -6;
    SoundManager::play(SoundManager::SOUND_MENU_MOVE); // Flap audio feedback
  }

  if (millis() - lastFrame >= frameMs) {
    lastFrame = millis();
    birdV++;
    
    prevBirdY = birdY;
    prevPipeX = pipeX;
    prevGapY = gapY;

    birdY += birdV;
    pipeX -= 4;

    if (pipeX < -32) {
      pipeX = 320;
      gapY = 70 + random(0, 60);
    }

    // Scoring boundary cross listener
    if (pipeX == birdX) {
      score++;
      SoundManager::play(SoundManager::SOUND_COIN); // Point chime
    }

    // Ceilings and Floor crash conditions
    if (birdY > 228 || birdY < 0) {
      gameOver = true;
      SoundManager::play(SoundManager::SOUND_PLAYER_DEATH); // Splat/Crash effect
    }

    // Box obstacle crash mechanics matching 2D arrays
    if (birdX + 14 > pipeX && birdX < pipeX + 32) {
      if (birdY < gapY || birdY + 12 > gapY + 64) {
        gameOver = true;
        SoundManager::play(SoundManager::SOUND_PLAYER_HURT); // Impact crash tone
      }
    }

    drawFrame();
    spr.pushSprite(0, 0);
  }

  updateButtons();
  delay(10);
}

} // namespace FlappyGame