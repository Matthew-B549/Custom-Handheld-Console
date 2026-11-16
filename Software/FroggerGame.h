#pragma once
#include <Arduino.h>
#include <SD.h>                  // <-- Add SD library
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"    // <-- Add HighScoreManager
#include "SoundManager.h"        // <-- Integrated SoundManager

namespace FroggerGame {
const int sdChipSelect = 5;      // <-- Add your SD CS pin (same as Tetris)
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
const int maxX = 319;
const int maxY = 239;
bool launcherExit = false, onMenu = true, gameOver = false, quitConfirm = false, won = false, needsDraw = true;
bool endScreenShown = false;     // <-- Add tracker to prevent rapid SD writing
int frogX = 152, frogY = 216, score = 0, lives = 3;
int carX[8], carY[8], carSpeed[8];
int prevFrogX = 152, prevFrogY = 216, prevCarX[8], prevCarY[8];
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
unsigned long lastFrame = 0, lastDebounce = 0;
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

void resetCars() {
  for (int i = 0; i < 8; i++) {
    carY[i] = 54 + i * 18;
    carX[i] = random(0, 280);
    carSpeed[i] = random(2, 6);
    if (i % 2 == 0) carSpeed[i] = -carSpeed[i];
    prevCarX[i] = carX[i];
    prevCarY[i] = carY[i];
  }
}
void drawMenu() {
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_GREEN, TFT_BLACK);
  spr.setTextSize(3);
  spr.setCursor(80, 40);
  spr.print("FROGGER");
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(60, 100);
  spr.print("D-PAD / JOYSTICK to move");
  spr.setCursor(60, 118);
  spr.print("Cross the road safely!");
  spr.setCursor(60, 136);
  spr.print("B asks to quit");
  spr.setTextSize(2);
  spr.setCursor(60, 190);
  spr.print("START TO PLAY");
}
void resetGame() {
  gameOver = false;
  endScreenShown = false;        // <-- Reset tracker on new game
  quitConfirm = false;
  won = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownBoxDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  frogX = 152;
  frogY = 216;
  score = 0;
  lives = 3;
  resetCars();
  lastFrame = millis();

  SoundManager::play(SoundManager::SOUND_GAME_START); // Play start melody
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
    spr.fillSprite(TFT_BLACK);
    needsDraw = false;
  } else {
    spr.fillRect(prevFrogX - 2, prevFrogY - 2, 16, 16, TFT_BLACK);
    for (int i = 0; i < 8; i++) {
      spr.fillRect(prevCarX[i] - 2, prevCarY[i] - 2, 20, 14, TFT_BLACK);
    }
  }
  spr.fillRect(0, 20, 320, 16, TFT_DARKGREEN);
  spr.fillRect(0, 198, 320, 38, TFT_DARKGREEN);
  for (int i = 0; i < 8; i++) {
    uint16_t color = (i % 2 == 0) ? TFT_RED : TFT_YELLOW;
    spr.fillRect(carX[i], carY[i], 16, 10, color);
    prevCarX[i] = carX[i];
    prevCarY[i] = carY[i];
  }
  spr.fillRect(frogX, frogY, 12, 12, TFT_GREEN);
  prevFrogX = frogX;
  prevFrogY = frogY;
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(8, 6);
  spr.print("SCORE: ");
  spr.print(score);
  spr.setCursor(200, 6);
  spr.print("LIVES: ");
  spr.print(lives);
}

void drawGameOver() {
  // Initialize SD card and save the score to SD card
  SD.begin(sdChipSelect);
  int rank = HighScoreManager::saveScore("frogger", score);

  // Expanded box height slightly (from 82 to 112) to fit high scores safely
  spr.fillRect(66, 66, 188, 112, TFT_BLACK);
  spr.drawRect(64, 64, 192, 116, TFT_RED);
  
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.setTextSize(2);
  spr.setCursor(94, 76);
  if (won) {
    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.print("YOU WIN!");
  } else {
    spr.print("GAME OVER");
  }
  
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextSize(1);
  spr.setCursor(100, 104);
  spr.print("SCORE: ");
  spr.print(score);

  // Load and display the all-time high score
  unsigned long highScores[HighScoreManager::MAX_SCORES];
  int count = HighScoreManager::loadScores("frogger", highScores);
  if (count > 0) {
    spr.setCursor(100, 120);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.print("BEST:  ");
    spr.print(highScores[0]);
  }

  // If the player placed on the leaderboard, display their rank
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
    SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Countdown tick
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

  // Handle quit confirmation from any state
  if (quitConfirm) {
    if (tapped(B)) {
      quitConfirm = false;
      needsDraw = true;
      SoundManager::play(SoundManager::SOUND_MENU_BACK); // Cancel quit sound
    }
    if (tapped(Enter)) {
      SoundManager::play(SoundManager::SOUND_MENU_SELECT); // Confirm quit sound
      returnToMenu();
    }
    updateButtons();
    delay(10);
    return;
  }

  // Check for B button to initiate quit from any state
  if (tapped(B) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    quitConfirm = true;
    drawQuit();
    spr.pushSprite(0, 0);
    SoundManager::play(SoundManager::SOUND_MENU_MOVE); // Prompt open effect
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
      SoundManager::play(SoundManager::SOUND_MENU_BACK); // Pause audio feedback
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
  if (gameOver || won) {
    if (!endScreenShown) {
      drawGameOver();       // This now saves and builds the frame safely once
      endScreenShown = true;
      SoundManager::play(SoundManager::SOUND_GAME_OVER); // Game over / Victory fanfare
    }
    if (tapped(Start) || tapped(Enter)) resetGame();
    spr.pushSprite(0, 0);
    updateButtons();
    delay(10);
    return;
  }
  if (millis() - lastDebounce > debounceMs) {
    bool moved = false;
    if (tapped(Up)) {
      frogY -= 18;
      lastDebounce = millis();
      score += 10;
      moved = true;
    }
    if (tapped(Down)) {
      frogY += 18;
      lastDebounce = millis();
      moved = true;
    }
    if (tapped(Left)) {
      frogX -= 18;
      lastDebounce = millis();
      moved = true;
    }
    if (tapped(Right)) {
      frogX += 18;
      lastDebounce = millis();
      moved = true;
    }
    if (moved) {
      SoundManager::play(SoundManager::SOUND_MENU_MOVE); // Hop audio effect
    }
  }
  frogX = constrain(frogX, 8, 312);
  frogY = constrain(frogY, 20, 224);
  if (frogY < 36 && !won) {
    won = true;
    score += 100;
    SoundManager::play(SoundManager::SOUND_POWERUP); // Success chime
  }
  if (millis() - lastFrame > 32) {
    lastFrame = millis();
    for (int i = 0; i < 8; i++) {
      carX[i] += carSpeed[i];
      if (carSpeed[i] > 0 && carX[i] > 320) carX[i] = -16;
      if (carSpeed[i] < 0 && carX[i] < -16) carX[i] = 320;
      if (frogX + 12 > carX[i] && frogX < carX[i] + 16 && frogY + 12 > carY[i] && frogY < carY[i] + 10) {
        lives--;
        frogX = 152;
        frogY = 216;
        SoundManager::play(SoundManager::SOUND_PLAYER_HURT); // Crash/Splat noise
        if (lives <= 0) gameOver = true;
      }
    }
    drawFrame();
    spr.pushSprite(0, 0);
  }
  updateButtons();
  delay(8);
}

}  // namespace FroggerGame