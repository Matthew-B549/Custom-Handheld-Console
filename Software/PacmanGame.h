#pragma once
#include <Arduino.h>
#include <SD.h>                  // Added SD library support
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"    // Added HighScoreManager support
#include "SoundManager.h"        // Added SoundManager Integration

namespace PacmanGame {

// Using inline allows a single-file header implementation without duplicate symbol linker errors
inline bool launcherExit = false;

inline bool shouldExitToLauncher() {
  return launcherExit;
}

inline const int sdChipSelect = 5;      // Standard SD card CS pin config
inline const int dacPin = 25;

inline TFT_eSPI tft = TFT_eSPI();
inline const uint16_t backgroundColor = TFT_BLACK;
inline const int maxX = 319;
inline const int maxY = 239;

// Arcade-accurate grid: 28 cols x 31 rows
inline const int mazeCols = 28;
inline const int mazeRows = 31;
inline const int cellSize = 7;
inline const int mazeX = 50;
inline const int mazeY = 12;

// Ghost personalities
enum GhostName { BLINKY, PINKY, INKY, CLYDE };
enum GhostMode { SCATTER, CHASE, FRIGHTENED, EATEN, IN_HOUSE, LEAVING_HOUSE };

// ===================== MAZE LAYOUT =====================
inline const char levelMap[mazeRows][mazeCols + 1] = {
  "############################",
  "#............##............#",
  "#.####.#####.##.#####.####.#",
  "#o####.#####.##.#####.####o#",
  "#.####.#####.##.#####.####.#",
  "#..........................#",
  "#.####.##.########.##.####.#",
  "#.####.##.########.##.####.#",
  "#......##....##....##......#",
  "######.#####.##.#####.######",
  "######.#####.##.#####.######",
  "#......##..........##......#",
  "#.####.##.###  ###.##.####.#",
  "#.####.##.---GG---.##.####.#", 
  " o..##.##.|      |.##.##..o ",
  "######.##.|      |.##.######", 
  "     #.##.--------.##.#     ",
  "     #.##.        .##.#     ",
  "     #.##.        .##.#     ",
  "######.##.        .##.######",
  "      .  .        .  .      ",
  "######.##.        .##.######",
  "     #.##.        .##.#     ",
  "     #.##.        .##.#     ",
  "     #.##.        .##.#     ",
  "######.##.########.##.######",
  "#............##............#",
  "#.####.#####.##.#####.####.#",
  "#.####.#####.##.#####.####.#",
  "#o..##.......  .......##..o#",
  "############################"
};

inline char maze[mazeRows][mazeCols];

// ===================== STRUCTS =====================
struct Actor {
  int col;
  int row;
  int dirX;
  int dirY;
  uint16_t color;
};

inline Actor pacman;
inline int nextDirX = -1;
inline int nextDirY = 0;

inline const int ghostCount = 4;
inline Actor ghosts[ghostCount];
inline GhostMode ghostModes[ghostCount];
inline GhostMode previousModes[ghostCount];
inline unsigned long ghostModeTimer[ghostCount];
inline unsigned long ghostReleaseTime[ghostCount];
inline int ghostFrightenedFlashCount = 0;
inline unsigned long frightenedFlashTimer = 0;
inline bool frightenedFlashState = false;

struct ScorePopup {
  int col;
  int row;
  int points;
  unsigned long drawnAt;
  bool active;
};
inline ScorePopup scorePopups[ghostCount];
inline const unsigned long scorePopupDuration = 1500;

inline const int scatterTargets[4][2] = {
  { mazeCols - 3, 0 },
  { 2, 0 },
  { mazeCols - 1, mazeRows - 1 },
  { 0, mazeRows - 1 }
};

inline const int ghostHouseExitCol = 13;
inline const int ghostHouseExitRow = 11;
inline const int ghostHouseCenterCol = 13;
inline const int ghostHouseCenterRow = 14;

// ===================== GAME STATE =====================
inline int score = 0;
inline int lives = 3;
inline int pelletsLeft = 0;
inline int pelletsEaten = 0;
inline int ghostCombo = 0;
inline bool gameOver = false;
inline bool gameWon = false;
inline bool endScreenDrawn = false;
inline bool onMenu = true;
inline bool quitConfirm = false;
inline bool gamePaused = false;
inline bool pauseScreenDrawn = false;
inline bool resumeCountdownActive = false;
inline int resumeCountdownValue = 0;
inline unsigned long resumeCountdownLastMs = 0;
inline bool resumeCountdownBoxDrawn = false;
inline int lastDrawnCountdownValue = -1;

inline bool mouthOpen = true;
inline int mouthFrame = 0;
inline unsigned long mouthTimer = 0;

inline unsigned long powerUntilMs = 0;
inline unsigned long lastDebounce = 0;
inline const unsigned long debounceMs = 160;

inline unsigned long nextPacmanMoveMs = 0;
inline unsigned long nextGhostMoveMs = 0;
inline const unsigned long pacmanIntervalMs = 180;
inline const unsigned long ghostIntervalMs = 200;

inline const unsigned long scatterDuration = 7000;
inline const unsigned long chaseDuration = 20000;
inline const unsigned long frightenedDuration = 6000;
inline const unsigned long frightenedFlashDuration = 4000;

inline int ghostPhase = 0;
inline unsigned long ghostPhaseStart = 0;

// ===================== SOUNDS =====================
inline bool soundPlaying = false;
inline int soundSampleIndex = 0;
inline unsigned long lastSoundSampleUs = 0;
inline const unsigned long soundSampleIntervalUs = 125;
inline int currentSoundSampleCount = 0;
inline const uint8_t* currentSoundData = nullptr;

inline const int chompSampleCount = 400;
inline uint8_t chompSound[chompSampleCount];

inline void generateChompSound() {
  for (int i = 0; i < chompSampleCount; i++) {
    chompSound[i] = (i % 20 < 10) ? 40 : 0;
  }
}

inline void startSound(const uint8_t* data, int count) {
  currentSoundData = data;
  currentSoundSampleCount = count;
  soundPlaying = true;
  soundSampleIndex = 0;
  lastSoundSampleUs = micros();
}

inline void updateSound() {
  if (!soundPlaying) return;
  unsigned long now = micros();
  while (soundPlaying && now - lastSoundSampleUs >= soundSampleIntervalUs) {
    if (soundSampleIndex >= currentSoundSampleCount) {
      dacWrite(dacPin, 0);
      soundPlaying = false;
    } else {
      dacWrite(dacPin, currentSoundData[soundSampleIndex++]);
      lastSoundSampleUs += soundSampleIntervalUs;
    }
  }
}

// ===================== BUTTONS =====================
inline void scanButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

inline bool tapped(buttonName b) {
  return buttons[b].pressed && !buttons[b].lastPressed;
}

inline void updateButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

// ===================== MAZE NAVIGATION =====================
inline bool isWalkable(int col, int row, bool isGhost) {
  if (row == 14 && (col < 0 || col >= mazeCols)) return true;
  if (col < 0 || col >= mazeCols || row < 0 || row >= mazeRows) return false;

  char cell = maze[row][col];
  if (cell == '#') return false;
  if (cell == '-' || cell == 'G' || cell == '|') {
    if (!isGhost) return false;
  }
  return true;
}

// ===================== DRAWING =====================
inline void drawCell(int col, int row) {
  if (col < 0 || col >= mazeCols || row < 0 || row >= mazeRows) return;
  
  int x = mazeX + col * cellSize;
  int y = mazeY + row * cellSize;
  char cell = maze[row][col];

  if (cell == '#') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x + 1, y + 1, cellSize - 2, cellSize - 2, TFT_BLUE);
  } else if (cell == '-') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x, y + cellSize / 2 - 1, cellSize, 2, TFT_WHITE);
  } else if (cell == '|') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x + cellSize / 2 - 1, y, 2, cellSize, TFT_WHITE);
  } else if (cell == 'G') {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    tft.fillRect(x, y + cellSize / 2 - 1, cellSize, 2, 0xF81F);
  } else {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    if (cell == '.') {
      tft.fillRect(x + cellSize / 2 - 1, y + cellSize / 2 - 1, 2, 2, 0xFDCB);
    } else if (cell == 'o') {
      tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 3, 0xFDCB);
    }
  }
}

inline void drawMaze() {
  tft.fillScreen(backgroundColor);
  for (int r = 0; r < mazeRows; r++) {
    for (int c = 0; c < mazeCols; c++) {
      drawCell(c, r);
    }
  }
}

inline void drawPacman() {
  if (pacman.col < 0 || pacman.col >= mazeCols) {
    int x, y;
    if (pacman.col < 0) {
      x = mazeX - cellSize / 2;
    } else {
      x = mazeX + mazeCols * cellSize + cellSize / 2;
    }
    y = mazeY + pacman.row * cellSize + cellSize / 2;
    tft.fillCircle(x, y, 3, TFT_YELLOW);
    return;
  }
  int x = mazeX + pacman.col * cellSize + cellSize / 2;
  int y = mazeY + pacman.row * cellSize + cellSize / 2;

  tft.fillRect(x - 4, y - 4, 9, 9, backgroundColor);

  char cell = maze[pacman.row][pacman.col];
  if (cell == '.') {
    tft.fillRect(x - 1, y - 1, 2, 2, 0xFDCB);
  } else if (cell == 'o') {
    tft.fillCircle(x, y, 3, 0xFDCB);
  }

  tft.fillCircle(x, y, 3, TFT_YELLOW);

  if (mouthOpen) {
    int mouthSize = 2;
    int px1, py1, px2, py2;

    if (pacman.dirX == 1) {
      px1 = x; py1 = y - mouthSize;
      px2 = x; py2 = y + mouthSize;
    } else if (pacman.dirX == -1) {
      px1 = x; py1 = y - mouthSize;
      px2 = x; py2 = y + mouthSize;
    } else if (pacman.dirY == -1) {
      px1 = x - mouthSize; py1 = y;
      px2 = x + mouthSize; py2 = y;
    } else {
      px1 = x - mouthSize; py1 = y;
      px2 = x + mouthSize; py2 = y;
    }
    tft.fillTriangle(x, y, px1, py1, px2, py2, backgroundColor);
  }
}

inline void drawGhost(int index) {
  Actor& g = ghosts[index];
  if (g.col < 0 || g.col >= mazeCols) return;

  int x = mazeX + g.col * cellSize + cellSize / 2;
  int y = mazeY + g.row * cellSize + cellSize / 2;
  unsigned long now = millis();

  GhostMode mode = ghostModes[index];
  drawCell(g.col, g.row);

  if (mode == EATEN) {
    tft.fillRect(x - 2, y - 1, 2, 2, TFT_WHITE);
    tft.fillRect(x + 1, y - 1, 2, 2, TFT_WHITE);
    int eyeDirX = (ghostHouseCenterCol > g.col) ? 1 : (ghostHouseCenterCol < g.col) ? -1 : 0;
    int eyeDirY = (ghostHouseCenterRow > g.row) ? 1 : (ghostHouseCenterRow < g.row) ? -1 : 0;
    tft.fillRect(x - 2 + (eyeDirX > 0 ? 1 : 0), y - 1 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLACK);
    tft.fillRect(x + 1 + (eyeDirX > 0 ? 1 : 0), y - 1 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLACK);
    return;
  }

  uint16_t bodyColor;
  if (mode == FRIGHTENED) {
    if (now >= frightenedFlashTimer) {
      bodyColor = frightenedFlashState ? TFT_WHITE : TFT_BLUE;
    } else {
      bodyColor = TFT_BLUE;
    }
  } else {
    bodyColor = g.color;
  }

  tft.fillCircle(x, y, 2, bodyColor);
  tft.fillRect(x - 2, y + 2, 5, 2, bodyColor);
  tft.drawPixel(x - 1, y + 3, bodyColor);
  tft.drawPixel(x + 1, y + 3, bodyColor);

  tft.fillRect(x - 2, y - 2, 2, 2, TFT_WHITE);
  tft.fillRect(x + 1, y - 2, 2, 2, TFT_WHITE);
  
  if (mode != FRIGHTENED) {
    int eyeDirX = g.dirX;
    int eyeDirY = g.dirY;
    tft.fillRect(x - 2 + (eyeDirX > 0 ? 1 : 0), y - 2 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLUE);
    tft.fillRect(x + 1 + (eyeDirX > 0 ? 1 : 0), y - 2 + (eyeDirY > 0 ? 1 : 0), 1, 1, TFT_BLUE);
  } else {
    tft.fillRect(x - 1, y - 1, 1, 1, 0xFCA0);
    tft.fillRect(x + 2, y - 1, 1, 1, 0xFCA0);
  }
}

inline void drawHud() {
  tft.fillRect(0, 0, mazeX, maxY + 1, backgroundColor);
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(4, 20);
  tft.print("SCORE");
  tft.setCursor(4, 36);
  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.print(score);

  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setCursor(4, 70);
  tft.print("LIVES");
  tft.setCursor(4, 86);
  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.print(lives < 0 ? 0 : lives);
}

inline void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(80, 40);
  tft.print("PACMAN");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(70, 100);
  tft.print("ARROWS: Steer Pacman");
  tft.setCursor(70, 120);
  tft.print("A Button: Pause Game");
  tft.setCursor(70, 140);
  tft.print("B Button: Exit to Launcher");

  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(80, 190);
  tft.print("START TO PLAY");
}

inline void drawPauseOverlay() {
  tft.fillRect(100, 95, 120, 50, TFT_BLACK);
  tft.drawRect(98, 93, 124, 54, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(125, 110);
  tft.print("PAUSED");
  pauseScreenDrawn = true;
}

inline void drawResumeCountdown() {
  int boxW = 120;
  int boxH = 80;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  if (!resumeCountdownBoxDrawn) {
    tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(boxX + 24, boxY + 12);
    tft.print("RESUME");
    resumeCountdownBoxDrawn = true;
    lastDrawnCountdownValue = -1;
  }

  if (resumeCountdownValue != lastDrawnCountdownValue) {
    tft.fillRect(boxX + 45, boxY + 35, 30, 30, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(boxX + 52, boxY + 38);
    tft.print(resumeCountdownValue);
    lastDrawnCountdownValue = resumeCountdownValue;
  }
}

inline void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownBoxDrawn = false;
  resumeCountdownValue = 3;
  lastDrawnCountdownValue = -1;
  resumeCountdownLastMs = millis();
  drawResumeCountdown();
}

inline bool updateResumeCountdown() {
  unsigned long now = millis();
  if (now - resumeCountdownLastMs >= 1000) {
    resumeCountdownLastMs += 1000;
    resumeCountdownValue--;
    if (resumeCountdownValue <= 0) {
      resumeCountdownActive = false;
      resumeCountdownBoxDrawn = false;
      lastDrawnCountdownValue = -1;
      return false;
    }
    drawResumeCountdown();
    SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Resume counting down beep
  }
  return true;
}

inline void drawQuit() {
  tft.fillRect(50, 80, 220, 80, TFT_BLACK);
  tft.drawRect(48, 78, 224, 84, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(130, 95);
  tft.print("QUIT?");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(75, 130);
  tft.print("ENTER to Confirm  B to Return");
}

inline void drawEndScreen() {
  int rank = -1;
  unsigned long highScores[HighScoreManager::MAX_SCORES];
  int count = 0;

  SD.begin(sdChipSelect);
  if (gameWon || gameOver) {
    rank = HighScoreManager::saveScore("pacman", score);
  }
  count = HighScoreManager::loadScores("pacman", highScores);

  tft.fillScreen(backgroundColor);
  drawHud();

  if (gameWon) {
    tft.fillRect(54, 54, 212, 132, backgroundColor);
    tft.drawRect(52, 52, 216, 136, TFT_GREEN);
    tft.setTextColor(TFT_GREEN, backgroundColor);
    tft.setTextSize(2);
    tft.setCursor(110, 64);
    tft.print("VICTORY!");
    SoundManager::play(SoundManager::SOUND_GAME_START); // Victory tune
  } else {
    tft.fillRect(54, 54, 212, 132, backgroundColor);
    tft.drawRect(52, 52, 216, 136, TFT_RED);
    tft.setTextColor(TFT_RED, backgroundColor);
    tft.setTextSize(2);
    tft.setCursor(114, 64);
    tft.print("GAME OVER");
    SoundManager::play(SoundManager::SOUND_GAME_OVER);  // Defeat tone
  }

  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(88, 96);
  tft.print("FINAL SCORE: ");
  tft.print(score);

  if (count > 0) {
    tft.setTextColor(TFT_WHITE, backgroundColor);
    tft.setCursor(88, 114);
    tft.print("ALL-TIME BEST: ");
    tft.print(highScores[0]);
  }

  if (rank > 0 && rank <= HighScoreManager::MAX_SCORES) {
    tft.setTextColor(TFT_GREEN, backgroundColor);
    tft.setCursor(88, 132);
    tft.print("RANK ACHIEVED: #");
    tft.print(rank);
  }
  
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(102, 162);
  tft.print("START to replay");
}

// ===================== GHOST AI & PATHS =====================
inline int manhattanDist(int c1, int r1, int c2, int r2) {
  return abs(c1 - c2) + abs(r1 - r2);
}

inline void chooseGhostDirection(int index) {
  Actor& g = ghosts[index];
  GhostMode mode = ghostModes[index];

  if (mode == EATEN) {
    int options[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int bestDir = -1;
    int bestDist = 9999;

    for (int i = 0; i < 4; i++) {
      int dc = options[i][0];
      int dr = options[i][1];
      if (dc == -g.dirX && dr == -g.dirY) continue;

      int nc = g.col + dc;
      int nr = g.row + dr;
      if (!isWalkable(nc, nr, true)) continue;

      int d = manhattanDist(nc, nr, ghostHouseCenterCol, ghostHouseCenterRow);
      if (d < bestDist) {
        bestDist = d;
        bestDir = i;
      }
    }
    if (bestDir != -1) {
      g.dirX = options[bestDir][0];
      g.dirY = options[bestDir][1];
    }
    return;
  }

  if (mode == FRIGHTENED) {
    int options[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int validDirs[4];
    int validCount = 0;

    for (int i = 0; i < 4; i++) {
      int dc = options[i][0];
      int dr = options[i][1];
      if (dc == -g.dirX && dr == -g.dirY) continue;
      int nc = g.col + dc;
      int nr = g.row + dr;
      if (isWalkable(nc, nr, true)) {
        validDirs[validCount++] = i;
      }
    }

    if (validCount > 0) {
      int chosen = validDirs[random(0, validCount)];
      g.dirX = options[chosen][0];
      g.dirY = options[chosen][1];
    } else {
      g.dirX = -g.dirX;
      g.dirY = -g.dirY;
    }
    return;
  }

  int targetCol, targetRow;
  if (mode == SCATTER) {
    targetCol = scatterTargets[index][0];
    targetRow = scatterTargets[index][1];
  } else {
    switch (index) {
      case BLINKY:
        targetCol = pacman.col;
        targetRow = pacman.row;
        break;
      case PINKY:
        targetCol = pacman.col + pacman.dirX * 4;
        targetRow = pacman.row + pacman.dirY * 4;
        break;
      case INKY: {
        int pivotCol = pacman.col + pacman.dirX * 2;
        int pivotRow = pacman.row + pacman.dirY * 2;
        int blinkyCol = ghosts[BLINKY].col;
        int blinkyRow = ghosts[BLINKY].row;
        targetCol = pivotCol + (pivotCol - blinkyCol);
        targetRow = pivotRow + (pivotRow - blinkyRow);
        break;
      }
      case CLYDE: {
        int dist = manhattanDist(g.col, g.row, pacman.col, pacman.row);
        if (dist > 8) {
          targetCol = pacman.col;
          targetRow = pacman.row;
        } else {
          targetCol = scatterTargets[CLYDE][0];
          targetRow = scatterTargets[CLYDE][1];
        }
        break;
      }
    }
  }

  int options[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
  int bestDir = -1;
  int bestDist = 9999;

  for (int i = 0; i < 4; i++) {
    int dc = options[i][0];
    int dr = options[i][1];
    if (dc == -g.dirX && dr == -g.dirY) continue;
    int nc = g.col + dc;
    int nr = g.row + dr;

    if (nr == 14 && (nc == -1 || nc == mazeCols)) {
      bestDir = i;
      break;
    }
    if (!isWalkable(nc, nr, true)) continue;

    int d = manhattanDist(nc, nr, targetCol, targetRow);
    if (d < bestDist) {
      bestDist = d;
      bestDir = i;
    }
  }

  if (bestDir != -1) {
    g.dirX = options[bestDir][0];
    g.dirY = options[bestDir][1];
  } else {
    g.dirX = -g.dirX;
    g.dirY = -g.dirY;
  }
}

inline void setGhostMode(GhostMode mode) {
  unsigned long now = millis();
  for (int i = 0; i < ghostCount; i++) {
    if (ghostModes[i] == IN_HOUSE || ghostModes[i] == LEAVING_HOUSE || ghostModes[i] == EATEN) continue;
    if (mode == FRIGHTENED && ghostModes[i] != FRIGHTENED) {
      previousModes[i] = ghostModes[i];
      ghostModes[i] = FRIGHTENED;
      ghosts[i].dirX = -ghosts[i].dirX;
      ghosts[i].dirY = -ghosts[i].dirY;
    } else if (mode != FRIGHTENED && ghostModes[i] == FRIGHTENED) {
      ghostModes[i] = previousModes[i];
    } else if (mode != FRIGHTENED) {
      ghostModes[i] = mode;
    }
  }
  ghostModeTimer[0] = now;
}

inline void updateGhostModes() {
  unsigned long now = millis();
  unsigned long elapsed = now - ghostModeTimer[0];

  if (ghostModes[0] == FRIGHTENED) {
    if (elapsed >= frightenedDuration) {
      setGhostMode(previousModes[0]);
      ghostModeTimer[0] = now;
    } else if (elapsed >= frightenedFlashDuration) {
      if (now >= frightenedFlashTimer) {
        frightenedFlashTimer = now + 200;
        frightenedFlashState = !frightenedFlashState;
      }
    }
    return;
  }

  unsigned long phaseElapsed = now - ghostPhaseStart;
  const unsigned long phaseDurations[] = { 7000, 20000, 7000, 20000, 5000, 20000, 5000 };
  const int phaseCount = 7;

  if (ghostPhase < phaseCount && phaseElapsed >= phaseDurations[ghostPhase]) {
    ghostPhase++;
    ghostPhaseStart = now;
    GhostMode newMode = (ghostPhase % 2 == 0) ? SCATTER : CHASE;
    setGhostMode(newMode);
  }
}

// ===================== INIT SYSTEMS =====================
inline void initMaze() {
  pelletsLeft = 0;
  for (int r = 0; r < mazeRows; r++) {
    for (int c = 0; c < mazeCols; c++) {
      maze[r][c] = levelMap[r][c];
      if (maze[r][c] == '.' || maze[r][c] == 'o') {
        pelletsLeft++;
      }
    }
  }
  pelletsEaten = 0;
}

inline void resetActors() {
  pacman = { 14, 23, -1, 0, TFT_YELLOW };
  nextDirX = -1;
  nextDirY = 0;

  ghosts[BLINKY] = { 13, 11, 1, 0, TFT_RED };
  ghosts[PINKY]  = { 13, 14, 0, -1, 0xFC10 };
  ghosts[INKY]   = { 11, 14, 0, -1, TFT_CYAN };
  ghosts[CLYDE]  = { 15, 14, 0, -1, 0xFDB0 };

  ghostModes[BLINKY] = SCATTER;
  ghostModes[PINKY]  = IN_HOUSE;
  ghostModes[INKY]   = IN_HOUSE;
  ghostModes[CLYDE]  = IN_HOUSE;

  unsigned long now = millis();
  ghostReleaseTime[BLINKY] = now;
  ghostReleaseTime[PINKY]  = now + 3000;
  ghostReleaseTime[INKY]   = now + 7000;
  ghostReleaseTime[CLYDE]  = now + 12000;

  for (int i = 0; i < ghostCount; i++) {
    scorePopups[i].active = false;
  }
}

inline void resetGame() {
  score = 0;
  lives = 3;
  gameOver = false;
  gameWon = false;
  endScreenDrawn = false;
  quitConfirm = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownActive = false;
  ghostPhase = 0;
  ghostPhaseStart = millis();
  initMaze();
  resetActors();
  drawMaze();
  drawHud();
  SoundManager::play(SoundManager::SOUND_MENU_SELECT);
}

inline void clearExpiredScorePopups() {
  unsigned long now = millis();
  for (int i = 0; i < ghostCount; i++) {
    if (scorePopups[i].active && now - scorePopups[i].drawnAt >= scorePopupDuration) {
      scorePopups[i].active = false;
      drawCell(scorePopups[i].col, scorePopups[i].row);
    }
  }
}

inline void setupGame() {
  launcherExit = false;
  tft.init();
  tft.setRotation(3);
  generateChompSound();
  for (int i = 0; i < buttonCount; i++) pinMode(buttons[i].pin, INPUT_PULLUP);
  onMenu = true;
}

// ===================== RUNNING GAME LOOP =====================
inline void loopGame() {
  scanButtons();
  updateSound();

  // Handle quit confirmation from any state
  if (quitConfirm) {
    if (tapped(B)) {
      quitConfirm = false;
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
      if (onMenu) {
        drawMenu();
      } else {
        drawMaze();
        drawHud();
        for (int i = 0; i < ghostCount; i++) drawGhost(i);
        drawPacman();
      }
    }
    if (tapped(Enter)) {
      SoundManager::play(SoundManager::SOUND_MENU_SELECT);
      launcherExit = true;
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
    SoundManager::play(SoundManager::SOUND_MENU_MOVE);
    updateButtons();
    return;
  }

  if (onMenu) {
    static bool menuDrawn = false;
    if (!menuDrawn) {
      drawMenu();
      menuDrawn = true;
    }
    if ((tapped(Start) || tapped(Enter)) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      onMenu = false;
      menuDrawn = false;
      resetGame();
    }
    updateButtons();
    delay(10);
    return;
  }

  if (gameOver || gameWon) {
    if (!endScreenDrawn) {
      drawEndScreen();
      endScreenDrawn = true;
    }
    if (tapped(Start) || tapped(Enter)) {
      onMenu = true;
      endScreenDrawn = false;
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
      drawMaze();
      drawHud();
      startResumeCountdown();
    } else {
      drawPauseOverlay();
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
    }
  }

  if (resumeCountdownActive) {
    if (updateResumeCountdown()) {
      updateButtons();
      delay(10);
      return;
    }
  }

  if (gamePaused) {
    if (!pauseScreenDrawn) drawPauseOverlay();
    updateButtons();
    delay(10);
    return;
  }

  // Read steering updates
  if (buttons[Up].pressed)    { nextDirX = 0;  nextDirY = -1; }
  if (buttons[Down].pressed)  { nextDirX = 0;  nextDirY = 1;  }
  if (buttons[Left].pressed)  { nextDirX = -1; nextDirY = 0;  }
  if (buttons[Right].pressed) { nextDirX = 1;  nextDirY = 0;  }

  unsigned long now = millis();

  // Pacman tick animation
  if (now - mouthTimer > 100) {
    mouthTimer = now;
    mouthFrame = (mouthFrame + 1) % 3;
    mouthOpen = (mouthFrame < 2);
    drawPacman(); 
  }

  clearExpiredScorePopups();

  // Move Pacman Engine
  if (now >= nextPacmanMoveMs) {
    nextPacmanMoveMs = now + pacmanIntervalMs;

    int newCol = pacman.col + nextDirX;
    int newRow = pacman.row + nextDirY;

    if (isWalkable(newCol, newRow, false)) {
      pacman.dirX = nextDirX;
      pacman.dirY = nextDirY;
    }

    int targetCol = pacman.col + pacman.dirX;
    int targetRow = pacman.row + pacman.dirY;

    // Side tunnel tracking wrap-around
    if (targetRow == 14 && targetCol < 0) targetCol = mazeCols - 1;
    else if (targetRow == 14 && targetCol >= mazeCols) targetCol = 0;

    if (isWalkable(targetCol, targetRow, false)) {
      tft.fillRect(mazeX + pacman.col * cellSize, mazeY + pacman.row * cellSize, cellSize, cellSize, backgroundColor);
      pacman.col = targetCol;
      pacman.row = targetRow;

      // Handle items
      char currentCell = maze[pacman.row][pacman.col];
      if (currentCell == '.' || currentCell == 'o') {
        startSound(chompSound, chompSampleCount);
        if (currentCell == '.') {
          score += 10;
        } else {
          score += 50;
          ghostCombo = 0;
          setGhostMode(FRIGHTENED);
          SoundManager::play(SoundManager::SOUND_POWERUP); // Energized mode triggered!
        }
        maze[pacman.row][pacman.col] = ' ';
        pelletsLeft--;
        drawHud();
        if (pelletsLeft <= 0) {
          gameWon = true;
        }
      }
      drawPacman();
    }
  }

  // Move Ghosts Engine
  if (now >= nextGhostMoveMs) {
    nextGhostMoveMs = now + ghostIntervalMs;
    updateGhostModes();

    for (int i = 0; i < ghostCount; i++) {
      Actor& g = ghosts[i];
      GhostMode& mode = ghostModes[i];

      if (mode == IN_HOUSE) {
        if (now >= ghostReleaseTime[i]) {
          mode = LEAVING_HOUSE;
        } else {
          // bounce up and down inside house
          drawCell(g.col, g.row);
          g.row += g.dirY;
          if (g.row > 15 || g.row < 13) {
            g.dirY = -g.dirY;
          }
          drawGhost(i);
          continue;
        }
      }

      if (mode == LEAVING_HOUSE) {
        drawCell(g.col, g.row);
        if (g.col < ghostHouseExitCol) g.col++;
        else if (g.col > ghostHouseExitCol) g.col--;
        else if (g.row > ghostHouseExitRow) g.row--;
        
        if (g.col == ghostHouseExitCol && g.row == ghostHouseExitRow) {
          mode = (ghostPhase % 2 == 0) ? SCATTER : CHASE;
          g.dirX = -1; g.dirY = 0;
        }
        drawGhost(i);
        continue;
      }

      if (mode == EATEN) {
        if (g.col == ghostHouseCenterCol && g.row == ghostHouseCenterRow) {
          mode = LEAVING_HOUSE;
          ghostReleaseTime[i] = now + 2000;
          continue;
        }
      }

      chooseGhostDirection(i);
      
      int nextC = g.col + g.dirX;
      int nextR = g.row + g.dirY;

      // Wrap around tunnel navigation for ghosts
      if (nextR == 14 && nextC < 0) nextC = mazeCols - 1;
      else if (nextR == 14 && nextC >= mazeCols) nextC = 0;

      drawCell(g.col, g.row);
      g.col = nextC;
      g.row = nextR;
      drawGhost(i);
    }
  }

  // Collision handling matrix
  for (int i = 0; i < ghostCount; i++) {
    if (pacman.col == ghosts[i].col && pacman.row == ghosts[i].row) {
      if (ghostModes[i] == FRIGHTENED) {
        ghostModes[i] = EATEN;
        int pts = 200 * (1 << ghostCombo);
        score += pts;
        ghostCombo = (ghostCombo < 3) ? ghostCombo + 1 : 3;

        scorePopups[i] = { ghosts[i].col, ghosts[i].row, pts, millis(), true };
        tft.fillRect(mazeX + ghosts[i].col * cellSize, mazeY + ghosts[i].row * cellSize, cellSize, cellSize, backgroundColor);
        tft.setTextColor(TFT_CYAN, backgroundColor);
        tft.setTextSize(1);
        tft.setCursor(mazeX + ghosts[i].col * cellSize - 4, mazeY + ghosts[i].row * cellSize - 2);
        tft.print(pts);

        drawHud();
        SoundManager::play(SoundManager::SOUND_COIN); // Ghost eaten chime
      } else if (ghostModes[i] != EATEN && ghostModes[i] != IN_HOUSE && ghostModes[i] != LEAVING_HOUSE) {
        lives--;
        drawHud();
        SoundManager::play(SoundManager::SOUND_PLAYER_DEATH); // Played died alarm
        if (lives < 0) {
          gameOver = true;
        } else {
          resetActors();
          drawMaze();
          for (int j = 0; j < ghostCount; j++) drawGhost(j);
          drawPacman();
          delay(1000);
          nextPacmanMoveMs = millis() + 500;
          nextGhostMoveMs = millis() + 500;
        }
        break;
      }
    }
  }

  updateButtons();
  delay(10);
}

} // namespace PacmanGame