#pragma once
#include <Arduino.h>
#include <SD.h>                  // <-- Add SD library
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"    // <-- Add HighScoreManager
#include "SoundManager.h"        // <-- Add SoundManager Integration

namespace DinoGame {
const int sdChipSelect = 5;      // <-- Add your SD CS pin (same as Tetris)
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// 8-color palette for the sprite (indexed)
const uint16_t palette[8] = {
  TFT_BLACK,       // 0 - background/sky
  0x7BEF,          // 1 - ground brown
  0x1B60,          // 2 - dino dark green
  0x2E80,          // 3 - dino light green
  0xAD55,          // 4 - cactus green
  0xFFE0,          // 5 - yellow (eyes/sun)
  TFT_WHITE,       // 6 - white (eye highlight)
  0xF800           // 7 - red (game over)
};

const int maxX = 319;
const int maxY = 239;
const int groundY = 200;
const int dinoX = 50;
const int dinoW = 36;
const int dinoH = 40;

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

int dinoY = groundY - dinoH;
int prevDinoY = groundY - dinoH;
int dinoV = 0;
bool isJumping = false;
int groundOffset = 0;
int starOffset = 0;
int cactusX = 320;
int prevCactusX = 320;
int prevCactusH = 32;
int cactusW = 20;
int cactusH = 32;
int score = 0;
int highScore = 0;
int frameCount = 0;
int dinoAnimFrame = 0;
int animTimer = 0;
unsigned long lastFrame = 0;
unsigned long lastDebounce = 0;
const unsigned long frameMs = 40;
const unsigned long debounceMs = 160;
const int gravity = 1;
const int jumpPower = -10;

//// Dino sprite data (18x20) - 8-color indexed
// Colors: 0=bg, 2=dark green, 3=light green, 5=yellow, 6=white
// Frame 0: legs together (standing/neutral)
const uint8_t dinoSprite[20][18] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,2,2,2,2,2,2,2,2,0,0,0,0}, // Top of head
  {0,0,0,0,0,2,3,3,3,3,3,3,3,3,2,0,0,0}, // Head fill
  {0,0,0,0,0,2,3,3,3,6,6,2,3,3,2,0,0,0}, // Eye (White + Dark Green pupil)
  {0,0,0,0,0,2,3,3,3,6,6,3,3,3,2,0,0,0}, 
  {0,0,0,0,0,2,3,3,3,3,3,2,2,2,2,0,0,0}, // Jawline
  {0,0,0,0,0,0,2,2,3,3,3,2,0,0,0,0,0,0}, // Neck
  {0,0,0,0,2,2,2,2,3,3,3,5,2,0,0,0,0,0}, // Upper torso / Back start
  {0,0,0,2,3,3,3,3,3,3,5,5,5,2,0,0,0,0}, // Yellow belly plate starts
  {0,0,2,3,3,3,3,3,3,3,5,5,5,5,2,0,0,0},
  {0,2,3,3,3,3,2,2,2,3,5,5,5,5,2,0,0,0}, // Small arm reaching out
  {2,3,3,3,3,3,3,2,2,3,5,5,5,2,0,0,0,0}, // Tail extending out left
  {2,3,3,3,3,3,3,3,3,3,5,5,2,0,0,0,0,0},
  {0,2,2,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0}, // Lower hips
  {0,0,0,2,2,3,3,2,2,3,3,2,0,0,0,0,0,0}, // Thighs
  {0,0,0,0,2,3,2,0,2,3,2,0,0,0,0,0,0,0}, // Legs together
  {0,0,0,0,2,3,2,0,2,3,2,0,0,0,0,0,0,0},
  {0,0,0,2,2,5,2,0,2,2,5,2,0,0,0,0,0,0}, // Feet with yellow claws
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// Frame 1: legs spread (running)
const uint8_t dinoSpriteRun[20][18] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,2,2,2,2,2,2,2,2,0,0,0,0}, // Top of head
  {0,0,0,0,0,2,3,3,3,3,3,3,3,3,2,0,0,0}, // Head fill
  {0,0,0,0,0,2,3,3,3,6,6,2,3,3,2,0,0,0}, // Eye (White + Dark Green pupil)
  {0,0,0,0,0,2,3,3,3,6,6,3,3,3,2,0,0,0}, 
  {0,0,0,0,0,2,3,3,3,3,3,2,2,2,2,0,0,0}, // Jawline
  {0,0,0,0,0,0,2,2,3,3,3,2,0,0,0,0,0,0}, // Neck
  {0,0,0,0,2,2,2,2,3,3,3,5,2,0,0,0,0,0}, // Upper torso / Back start
  {0,0,0,2,3,3,3,3,3,3,5,5,5,2,0,0,0,0}, // Yellow belly plate starts
  {0,0,2,3,3,3,3,3,3,3,5,5,5,5,2,0,0,0},
  {0,2,3,3,3,3,2,2,2,3,5,5,5,5,2,0,0,0}, // Small arm reaching out  {2,3,3,3,3,3,3,2,2,3,5,5,5,2,0,0,0,0}, // Tail extending out left
  {2,3,3,3,3,3,3,3,3,3,5,5,2,0,0,0,0,0},
  {0,2,2,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0}, // Lower hips
  {0,0,0,2,2,3,3,2,2,3,3,2,0,0,0,0,0,0}, // Thighs
  {0,0,0,0,2,3,2,0,0,0,2,3,2,0,0,0,0,0}, // Left leg forward, right leg back
  {0,0,0,0,2,3,2,0,0,0,2,3,2,0,0,0,0,0},
  {0,0,0,2,2,5,2,0,0,0,2,2,5,2,0,0,0,0}, // Feet spread apart
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
// Cactus sprite data (10x16) - 8-color indexed
// Colors: 0=bg, 4=cactus green
const uint8_t cactusSprite[16][10] = {
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,4,4,0,0,0,0}, // Top of center trunk
  {0,0,0,0,4,4,0,0,0,0},
  {0,4,4,0,4,4,0,0,0,0}, // Left arm top starts
  {0,4,4,0,4,4,0,4,4,0}, // Right arm top starts
  {0,4,4,0,4,4,0,4,4,0},
  {0,4,4,4,4,4,0,4,4,0}, // Left arm connects to trunk
  {0,0,4,4,4,4,0,4,4,0},
  {0,0,0,0,4,4,4,4,4,0}, // Right arm connects to trunk
  {0,0,0,0,4,4,4,4,4,0},
  {0,0,0,0,4,4,0,0,0,0}, // Main lower trunk
  {0,0,0,0,4,4,0,0,0,0},
  {0,0,0,0,4,4,0,0,0,0},
  {0,0,0,0,4,4,0,0,0,0},
  {0,0,0,0,4,4,0,0,0,0},
  {0,0,0,0,4,4,0,0,0,0}  // Base of cactus
};

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

void drawIndexedSprite(int x, int y, int w, int h, const uint8_t sprite[][18]) {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      uint8_t idx = sprite[row][col];
      if (idx != 0) {
        spr.drawPixel(x + col, y + row, palette[idx]);
      }
    }
  }
}

void drawIndexedSpriteSmall(int x, int y, int w, int h, const uint8_t sprite[][10]) {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      uint8_t idx = sprite[row][col];
      if (idx != 0) {
        spr.drawPixel(x + col, y + row, palette[idx]);
      }
    }
  }
}

// Draw a sprite at 2x scale
void drawIndexedSprite2x(int x, int y, int w, int h, const uint8_t sprite[][18]) {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      uint8_t idx = sprite[row][col];
      if (idx != 0) {
        spr.fillRect(x + col * 2, y + row * 2, 2, 2, palette[idx]);
      }
    }
  }
}

// Draw a small sprite at 2x scale
void drawIndexedSpriteSmall2x(int x, int y, int w, int h, const uint8_t sprite[][10]) {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      uint8_t idx = sprite[row][col];
      if (idx != 0) {
        spr.fillRect(x + col * 2, y + row * 2, 2, 2, palette[idx]);
      }
    }
  }
}

void drawMenu() {
  spr.fillSprite(palette[0]);
  spr.setTextColor(palette[5], palette[0]);
  spr.setTextSize(3);
  spr.setCursor(44, 40);
  spr.print("DINO RUN");
  spr.setTextColor(palette[2], palette[0]);
  spr.setTextSize(1);
  spr.setCursor(60, 100);
  spr.print("UP / ENTER to jump");
  spr.setCursor(60, 118);
  spr.print("Dodge the cacti!");
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
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownBoxDrawn = false;
  resumeCountdownActive = false;
  needsDraw = true;
  dinoY = groundY - dinoH;
  prevDinoY = groundY - dinoH;
  dinoV = 0;
  isJumping = false;
  groundOffset = 0;
  cactusX = 320;
  prevCactusX = 320;
  score = 0;
  frameCount = 0;
  dinoAnimFrame = 0;
  animTimer = 0;
  lastFrame = millis();

  // Play introductory game start fanfare
  SoundManager::play(SoundManager::SOUND_GAME_START);
}

void drawQuit() {
  spr.fillRect(54, 76, 212, 88, palette[0]);
  spr.drawRect(52, 74, 216, 92, palette[5]);
  spr.setTextColor(palette[5], palette[0]);
  spr.setTextSize(2);
  spr.setCursor(104, 96);
  spr.print("QUIT?");
  spr.setTextColor(palette[6], palette[0]);
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

void drawStars() {
  // Draw scrolling stars in the sky to show movement
  const uint8_t starColors[8] = {2, 3, 4, 5, 6, 7, 1, 5};  // varied palette indices
  for (int i = 0; i < 320; i += 60) {
    int starX = (i + starOffset) % 320;
    uint16_t col1 = palette[starColors[(i/60) % 8]];
    uint16_t col2 = palette[starColors[((i/60)+3) % 8]];
    uint16_t col3 = palette[starColors[((i/60)+5) % 8]];
    spr.fillRect(starX, 20, 2, 2, col1);
    spr.fillRect(starX + 25, 55, 2, 2, col2);
    spr.fillRect(starX + 10, 85, 2, 2, col3);
    if ((i/60) % 3 == 0) {
      spr.fillRect(starX + 40, 35, 3, 3, palette[6]);  // white bright star
    }
    if ((i/60) % 3 == 1) {
      spr.fillRect(starX + 15, 70, 3, 3, palette[5]);  // yellow bright star
    }
  }
}

void drawGround() {
  spr.drawFastHLine(0, groundY, 320, palette[1]);
  for (int i = 0; i < 320; i += 8) {
    int bumpX = (i + groundOffset) % 320;
    spr.drawPixel(bumpX, groundY + 1, palette[1]);
    spr.drawPixel(bumpX + 2, groundY + 2, palette[1]);
  }
}

void drawFrame() {
  if (needsDraw) {
    spr.fillSprite(palette[0]);
    needsDraw = false;
  } else {
    int clearDinoY = min(prevDinoY, dinoY);
    int clearDinoH = max(prevDinoY + dinoH, dinoY + dinoH) - clearDinoY;
    spr.fillRect(dinoX - 4, clearDinoY - 4, dinoW + 8, clearDinoH + 8, palette[0]);

    int clearCactusX = min(prevCactusX, cactusX);
    int clearCactusW = max(prevCactusX + cactusW, cactusX + cactusW) - clearCactusX;
    spr.fillRect(clearCactusX - 4, groundY - 40, clearCactusW + 8, 48, palette[0]);

    spr.fillRect(0, 12, 320, groundY - 13, palette[0]);
  }

  drawStars();
  drawGround();

  drawIndexedSpriteSmall2x(cactusX, groundY - cactusH, 10, 16, cactusSprite);

  if (dinoAnimFrame == 0) {
    drawIndexedSprite2x(dinoX, dinoY, 18, 20, dinoSprite);
  } else {
    drawIndexedSprite2x(dinoX, dinoY, 18, 20, dinoSpriteRun);
  }

  spr.setTextColor(palette[6], palette[0]);
  spr.setTextSize(2);
  spr.setCursor(8, 8);
  spr.print(score);
}

void drawGameOver() {
  SD.begin(sdChipSelect);
  int rank = HighScoreManager::saveScore("dino", score);

  spr.fillRect(66, 66, 188, 112, palette[0]);
  spr.drawRect(64, 64, 192, 116, palette[7]);
  
  spr.setTextColor(palette[7], palette[0]);
  spr.setTextSize(2);
  spr.setCursor(94, 76);
  spr.print("GAME OVER");
  
  spr.setTextColor(palette[5], palette[0]);
  spr.setTextSize(1);
  spr.setCursor(100, 104);
  spr.print("SCORE: ");
  spr.print(score);

  unsigned long highScores[HighScoreManager::MAX_SCORES];
  int count = HighScoreManager::loadScores("dino", highScores);
  if (count > 0) {
    spr.setCursor(100, 120);
    spr.setTextColor(palette[6], palette[0]);
    spr.print("BEST:  ");
    spr.print(highScores[0]);
  }

  if (rank > 0 && rank <= HighScoreManager::MAX_SCORES) {
    spr.setTextColor(palette[3], palette[0]);
    spr.setCursor(100, 136);
    spr.print("RANK:  #");
    spr.print(rank);
    spr.print("!");
  }

  spr.setTextColor(palette[6], palette[0]);
  spr.setTextSize(1);
  spr.setCursor(102, 156);
  spr.print("START again");
}

void drawPauseOverlay() {
  spr.fillRect(88, 98, 144, 44, palette[0]);
  spr.drawRect(86, 96, 148, 48, palette[6]);
  spr.setTextColor(palette[6], palette[0]);
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
    spr.fillRect(boxX, boxY, boxW, boxH, palette[0]);
    spr.drawRect(boxX, boxY, boxW, boxH, palette[6]);
    spr.setTextColor(palette[5], palette[0]);
    spr.setTextSize(2);
    spr.setCursor(boxX + 24, boxY + 12);
    spr.print("RESUME");
    spr.setTextColor(palette[6], palette[0]);
    spr.setTextSize(1);
    spr.setCursor(boxX + 28, boxY + 74);
    spr.print("A to resume");
    resumeCountdownBoxDrawn = true;
  }

  spr.fillRect(boxX + 42, boxY + 34, 48, 30, palette[0]);
  spr.setTextColor(palette[5], palette[0]);
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
    if (tapped(Enter)) returnToMenu();
    updateButtons();
    delay(10);
    return;
  }

  if (tapped(B) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    quitConfirm = true;
    drawQuit();
    spr.pushSprite(0, 0);
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
      // Play retro sad melody on game over
      SoundManager::play(SoundManager::SOUND_GAME_OVER);
    }
    if (tapped(Start) || tapped(Enter)) resetGame();
    spr.pushSprite(0, 0);
    updateButtons();
    delay(10);
    return;
  }
  
  bool fastDrop = (buttons[Down].pressed || buttons[B].pressed) && isJumping;

  // Jump physics controller hook
  if ((tapped(Up) || tapped(Enter)) && !isJumping && millis() - lastDebounce > 80) {
    lastDebounce = millis();
    dinoV = jumpPower;
    isJumping = true;
    
    // Play a crisp upward audio pitch adjustment sound effect
    SoundManager::play(SoundManager::SOUND_MENU_SELECT);
  }
  
  if (millis() - lastFrame >= frameMs) {
    lastFrame = millis();
    frameCount++;

    prevDinoY = dinoY;
    prevCactusX = cactusX;
    prevCactusH = cactusH;

    if (fastDrop) {
      dinoV += gravity * 3;
    } else {
      dinoV += gravity;
    }
    dinoY += dinoV;
    if (dinoY >= groundY - dinoH) {
      dinoY = groundY - dinoH;
      dinoV = 0;
      isJumping = false;
    }

    if (!isJumping) {
      animTimer++;
      if (animTimer >= 4) {
        animTimer = 0;
        dinoAnimFrame = 1 - dinoAnimFrame;
      }
    } else {
      dinoAnimFrame = 0;
      animTimer = 0;
    }

    groundOffset = (groundOffset - 4 + 320) % 320;
    starOffset = (starOffset - 4 + 320) % 320;

    // Scroll cactus logic
    cactusX -= 4;
    if (cactusX < -cactusW) {
      cactusX = 320 + random(10, 50);  
      cactusH = 24 + random(0, 12);    
      score++;
      
      // Play scoring blip chime whenever a cactus is cleanly avoided
      SoundManager::play(SoundManager::SOUND_SCORE_TALLY);
    }

    int dinoRight = dinoX + dinoW - 6;
    int dinoBottom = dinoY + dinoH - 4;
    int cactusLeft = cactusX + 4;
    int cactusRight = cactusX + cactusW - 4;
    int cactusTop = groundY - cactusH + 4;

    if (dinoRight > cactusLeft && dinoX + 6 < cactusRight && dinoBottom > cactusTop && dinoY + 6 < groundY) {
      gameOver = true;
      if (score > highScore) highScore = score;
    }

    drawFrame();
    spr.pushSprite(0, 0);
  }
  updateButtons();
  delay(8);
}

}  // namespace DinoGame