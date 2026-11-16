#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>                  // <-- Included SD library for standard hardware compatibility
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"
#include "SoundManager.h"        // <-- Integrated SoundManager

namespace DrMarioGame {
const int sdChipSelect = 5;
bool launcherExit = false;
bool shouldExitToLauncher() {
  return launcherExit;
}

// TFT LCD Setup
TFT_eSPI tft = TFT_eSPI();
const uint16_t backgroundColor = TFT_BLACK;
const int maxX = 239;
const int maxY = 319;

// Dr. Mario Board Geometry Configuration
const int boardCols = 8;
const int boardRows = 16;
const int cellSize = 16; 
const int boardX = 12;
const int boardY = 32;
const int sidePanelX = 156;

// Optimized Memory Cell Grid (Replaced dangerous infinitely growing IDs with Link States)
enum CellLink { LINK_NONE = 0, LINK_RIGHT = 1, LINK_LEFT = 2, LINK_DOWN = 3, LINK_UP = 4 };

struct Cell {
  uint8_t t;    // 0: Empty, 1: Virus, 2: Pill Element
  uint8_t c;    // Color Index (1: Red, 2: Yellow, 3: Blue)
  uint8_t link; // Tracks connectivity to the other half of the pill
};

Cell board[boardRows][boardCols];
const uint16_t pieceColors[] = { TFT_BLACK, TFT_RED, TFT_YELLOW, TFT_BLUE };

// State Engine Variables
bool onStartScreen = true;
bool inGame = false;
bool gameOver = false;
bool gamePaused = false;
bool boardNeedsRedraw = true;
bool pauseScreenDrawn = false;
bool endScreenShown = false;
bool quitConfirm = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
bool isCascading = false;

// Time configurations
unsigned long globalDebounceTime = 0;
unsigned long lastFallTime = 0;
const unsigned long debounceDelayMs = 180;
unsigned long fallDelayMs = 800;

long score = 0;
int virusCount = 0;
int level = 1;
int comboChain = 0; // Tracks consecutive cascade clears for multiplier scoring

// Coordinate Offsets Mapping Matrix For The 4 Rotational Orientations
const int8_t OFF_X[] = { 1, 0, -1, 0 };
const int8_t OFF_Y[] = { 0, -1, 0, 1 };

struct Pill {
  int8_t x, y;
  uint8_t c1, c2;
  uint8_t d; // Orientation Index (0-3)
};

struct NextPill {
  uint8_t c1, c2;
};

Pill currentPill;
NextPill nextPill;

// Function Declarations
void scanButtons();
bool buttonTapped(buttonName button);
bool checkCollision(int8_t nx, int8_t ny, uint8_t nd);
void handleRotation(int8_t dirSign);
void drawStartScreen();
void drawBoardFrame();
void drawBoard();
void drawCell(int col, int row, uint8_t type, uint8_t colorIndex, uint8_t link);
void drawSidePanel();
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void drawQuitOverlay();
void cancelQuitOverlay();
void displayGameOver();
void returnToMenu();
void resetGame();
void advanceToNextLevel();
void spawnPill();
void stepGravity();
void processMatches();
void runCascadeEngine();
void calculateDropSpeed();

void setupGame() {
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(backgroundColor);
  randomSeed(esp_random());

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void loopGame() {
  scanButtons();

  if (quitConfirm) {
    if (buttonTapped(B) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
      cancelQuitOverlay();
    }
    if (buttonTapped(Enter) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      SoundManager::play(SoundManager::SOUND_MENU_SELECT);
      returnToMenu();
    }
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (buttonTapped(B) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    quitConfirm = true;
    SoundManager::play(SoundManager::SOUND_MENU_MOVE);
    drawQuitOverlay();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (onStartScreen) {
    if (boardNeedsRedraw) {
      drawStartScreen();
      boardNeedsRedraw = false;
    }

    if ((buttonTapped(Start) || buttonTapped(Enter)) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      onStartScreen = false;
      inGame = true;
      level = 1;
      score = 0;
      resetGame();
    }

    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (gameOver) {
    if (!endScreenShown) {
      displayGameOver();
      endScreenShown = true;
      SoundManager::play(SoundManager::SOUND_GAME_OVER);
    }

    if ((buttonTapped(Start) || buttonTapped(Enter)) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      level = 1;
      score = 0;
      resetGame();
    }

    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (buttonTapped(A) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      startResumeCountdown();
    } else {
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
    }
  }

  if (resumeCountdownActive) {
    if (updateResumeCountdown()) {
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
      delay(10);
      return;
    }
  }

  if (gamePaused) {
    if (!pauseScreenDrawn) drawPauseOverlay();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (boardNeedsRedraw) {
    tft.fillScreen(backgroundColor);
    drawBoardFrame();
    drawBoard();
    drawSidePanel();
    boardNeedsRedraw = false;
  }

  if (isCascading) {
    runCascadeEngine();
  } else {
    if (buttonTapped(Left) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      if (!checkCollision(currentPill.x - 1, currentPill.y, currentPill.d)) {
        currentPill.x--;
        drawBoard();
        SoundManager::play(SoundManager::SOUND_MENU_MOVE);
      }
    }
    if (buttonTapped(Right) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      if (!checkCollision(currentPill.x + 1, currentPill.y, currentPill.d)) {
        currentPill.x++;
        drawBoard();
        SoundManager::play(SoundManager::SOUND_MENU_MOVE);
      }
    }
    if (buttonTapped(Up) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      handleRotation(1); 
      drawBoard();
    }
    if (buttonTapped(Start) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      handleRotation(-1); 
      drawBoard();
    }
    if (buttons[Down].pressed) {
      stepGravity();
      drawBoard();
      delay(35); 
    }

    if (millis() - lastFallTime >= fallDelayMs) {
      stepGravity();
      drawBoard();
      lastFallTime = millis();
    }
  }

  for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
  delay(10);
}

void scanButtons() {
  for (int i = 0; i < buttonCount; i++) buttons[i].pressed = !digitalRead(buttons[i].pin);
}

bool buttonTapped(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed;
}

void calculateDropSpeed() {
  fallDelayMs = 800 - (level * 45);
  if (fallDelayMs < 120) fallDelayMs = 120;
}

bool checkCollision(int8_t nx, int8_t ny, uint8_t nd) {
  int8_t x2 = nx + OFF_X[nd];
  int8_t y2 = ny + OFF_Y[nd];

  if (nx < 0 || nx >= boardCols || ny < 0 || ny >= boardRows) return true;
  if (x2 < 0 || x2 >= boardCols || y2 < 0 || y2 >= boardRows) return true;

  if (board[ny][nx].t != 0) return true;
  if (board[y2][x2].t != 0) return true;

  return false;
}

void handleRotation(int8_t dirSign) {
  uint8_t nextDir = (currentPill.d + dirSign + 4) % 4;
  bool rotated = false;
  if (!checkCollision(currentPill.x, currentPill.y, nextDir)) { currentPill.d = nextDir; rotated = true; }
  else if (!checkCollision(currentPill.x - 1, currentPill.y, nextDir)) { currentPill.x--; currentPill.d = nextDir; rotated = true; }
  else if (!checkCollision(currentPill.x + 1, currentPill.y, nextDir)) { currentPill.x++; currentPill.d = nextDir; rotated = true; }
  else if (!checkCollision(currentPill.x, currentPill.y - 1, nextDir)) { currentPill.y--; currentPill.d = nextDir; rotated = true; }

  if (rotated) {
    SoundManager::play(SoundManager::SOUND_MENU_MOVE);
  }
}

void drawStartScreen() {
  tft.fillScreen(backgroundColor);
  tft.setTextColor(TFT_RED, backgroundColor);
  tft.setTextSize(3);
  tft.setCursor(34, 54);
  tft.print("DR. MARIO");

  tft.drawFastHLine(24, 92, 192, TFT_WHITE);

  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(34, 124);
  tft.print("LEFT/RIGHT: Move");
  tft.setCursor(34, 142);
  tft.print("UP: Rotate CW");
  tft.setCursor(34, 160);
  tft.print("START: Rotate CCW");
  tft.setCursor(34, 178);
  tft.print("DOWN: Soft Drop");
  tft.setCursor(34, 196);
  tft.print("A: Pause Game");
  tft.setCursor(34, 214);
  tft.print("B: Quit Prompt");

  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.setTextSize(2);
  tft.setCursor(32, 252);
  tft.print("START TO PLAY");
}

void drawBoardFrame() {
  tft.drawRect(boardX - 2, boardY - 2, boardCols * cellSize + 4, boardRows * cellSize + 4, tft.color565(78, 78, 153));
  tft.drawFastHLine(boardX + (3 * cellSize), boardY - 2, cellSize * 2, backgroundColor);
}

void drawBoard() {
  for (int r = 0; r < boardRows; r++) {
    for (int c = 0; c < boardCols; c++) {
      drawCell(c, r, board[r][c].t, board[r][c].c, board[r][c].link);
    }
  }

  if (inGame && !isCascading && !gameOver && !gamePaused) {
    int8_t oX = OFF_X[currentPill.d];
    int8_t oY = OFF_Y[currentPill.d];
    
    uint8_t link1 = LINK_NONE, link2 = LINK_NONE;
    if (currentPill.d == 0) { link1 = LINK_RIGHT; link2 = LINK_LEFT; }
    else if (currentPill.d == 1) { link1 = LINK_UP; link2 = LINK_DOWN; }
    else if (currentPill.d == 2) { link1 = LINK_LEFT; link2 = LINK_RIGHT; }
    else if (currentPill.d == 3) { link1 = LINK_DOWN; link2 = LINK_UP; }

    drawCell(currentPill.x, currentPill.y, 2, currentPill.c1, link1);
    drawCell(currentPill.x + oX, currentPill.y + oY, 2, currentPill.c2, link2);
  }
}

void drawCell(int col, int row, uint8_t type, uint8_t colorIndex, uint8_t link) {
  int px = boardX + col * cellSize;
  int py = boardY + row * cellSize;

  tft.fillRect(px, py, cellSize, cellSize, backgroundColor); 

  if (type == 0) return;

  uint16_t c = pieceColors[colorIndex];
  int rad = 5;

  if (type == 1) { 
    tft.fillCircle(px + cellSize/2, py + cellSize/2, cellSize/2 - 2, c);
    tft.fillRect(px + 4, py + 4, 3, 3, TFT_BLACK);
    tft.fillRect(px + 9, py + 4, 3, 3, TFT_BLACK);
    tft.drawFastHLine(px + 5, py + 10, 6, TFT_BLACK);
  } else if (type == 2) {
    if (link == LINK_NONE) tft.fillRoundRect(px + 1, py + 1, cellSize - 2, cellSize - 2, rad, c);
    else if (link == LINK_RIGHT) { tft.fillRoundRect(px + 1, py + 1, cellSize, cellSize - 2, rad, c); tft.fillRect(px + rad, py + 1, cellSize - rad, cellSize - 2, c); }
    else if (link == LINK_LEFT) { tft.fillRoundRect(px - 1, py + 1, cellSize, cellSize - 2, rad, c); tft.fillRect(px - 1, py + 1, cellSize - rad, cellSize - 2, c); }
    else if (link == LINK_DOWN) { tft.fillRoundRect(px + 1, py + 1, cellSize - 2, cellSize, rad, c); tft.fillRect(px + 1, py + rad, cellSize - 2, cellSize - rad, c); }
    else if (link == LINK_UP) { tft.fillRoundRect(px + 1, py - 1, cellSize - 2, cellSize, rad, c); tft.fillRect(px + 1, py - 1, cellSize - 2, cellSize - rad, c); }
    
    if (link != LINK_LEFT && link != LINK_UP) tft.fillRect(px + 4, py + 4, 3, 3, TFT_WHITE);
  }
}

void drawSidePanel() {
  tft.fillRect(sidePanelX, 0, maxX - sidePanelX + 1, maxY + 1, backgroundColor);

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setCursor(sidePanelX, 24);
  tft.print("NEXT");

  int pX = sidePanelX + 4;
  int pY = 44;
  tft.fillRoundRect(pX, pY, cellSize, cellSize, 5, pieceColors[nextPill.c1]);
  tft.fillRect(pX + 5, pY, cellSize - 5, cellSize, pieceColors[nextPill.c1]); 
  tft.fillRoundRect(pX + cellSize, pY, cellSize, cellSize, 5, pieceColors[nextPill.c2]);
  tft.fillRect(pX + cellSize, pY, cellSize - 5, cellSize, pieceColors[nextPill.c2]); 
  tft.fillRect(pX + 3, pY + 3, 3, 3, TFT_WHITE);
  tft.fillRect(pX + cellSize + 3, pY + 3, 3, 3, TFT_WHITE);

  tft.setCursor(sidePanelX, 116);
  tft.print("SCORE");
  tft.setCursor(sidePanelX, 130);
  tft.print(score);

  tft.setCursor(sidePanelX, 160);
  tft.print("VIRUS");
  tft.setCursor(sidePanelX, 174);
  tft.print(virusCount);

  tft.setCursor(sidePanelX, 204);
  tft.print("LEVEL");
  tft.setCursor(sidePanelX, 218);
  tft.print(level);
}

void drawPauseOverlay() {
  int boxW = 170;
  int boxH = 78;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;
  tft.fillRect(boxX, boxY, boxW, boxH, TFT_DARKGREY);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(boxX + 48, boxY + 18);
  tft.print("PAUSED");
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(boxX + 35, boxY + 52);
  tft.print("Press A to Resume");
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
  if (millis() - resumeCountdownLastMs >= 1000) {
    resumeCountdownLastMs += 1000;
    resumeCountdownValue--;
    if (resumeCountdownValue <= 0) {
      resumeCountdownActive = false;
      resumeCountdownBoxDrawn = false;
      boardNeedsRedraw = true; 
      return false;
    }
    drawResumeCountdown();
    SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Beep on countdown step
  }
  return true;
}

void drawQuitOverlay() {
  int boxW = 196;
  int boxH = 88;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;
  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_YELLOW);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(boxX + 42, boxY + 18);
  tft.print("QUIT GAME?");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(boxX + 28, boxY + 54);
  tft.print("ENTER quits   B cancels");
}

void cancelQuitOverlay() {
  quitConfirm = false;
  boardNeedsRedraw = true;
  pauseScreenDrawn = false;
}

void displayGameOver() {
  SD.begin(sdChipSelect);
  int rank = HighScoreManager::saveScore("drmario", score);

  int boxW = 196;
  int boxH = 110;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_RED);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(boxX + 38, boxY + 12);
  tft.print("GAME OVER");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(boxX + 42, boxY + 46);
  tft.print("Score: ");
  tft.print(score);

  unsigned long highScores[HighScoreManager::MAX_SCORES];
  if (HighScoreManager::loadScores("drmario", highScores) > 0) {
    tft.setCursor(boxX + 42, boxY + 62);
    tft.print("Best: ");
    tft.print(highScores[0]);
  }

  if (rank > 0 && rank <= HighScoreManager::MAX_SCORES) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(boxX + 42, boxY + 80);
    tft.print("Rank #");
    tft.print(rank);
    tft.print("!");
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(boxX + 32, boxY + 96);
  tft.print("START or ENTER");
}

void returnToMenu() {
  launcherExit = true;
  onStartScreen = true;
  inGame = false;
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  resumeCountdownActive = false;
  boardNeedsRedraw = true;
  isCascading = false;
}

void resetGame() {
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  resumeCountdownActive = false;
  boardNeedsRedraw = true;
  isCascading = false;
  comboChain = 0;
  endScreenShown = false;
  virusCount = min(4 + level * 4, 84); 
  calculateDropSpeed();

  for (int r = 0; r < boardRows; r++) {
    for (int c = 0; c < boardCols; c++) {
      board[r][c] = {0, 0, LINK_NONE};
    }
  }

  int placed = 0;
  while (placed < virusCount) {
    int r = random(5, 16); 
    int c = random(0, boardCols);
    if (board[r][c].t == 0) {
      board[r][c] = {1, (uint8_t)((placed % 3) + 1), LINK_NONE};
      placed++;
    }
  }

  nextPill.c1 = random(1, 4);
  nextPill.c2 = random(1, 4);

  SoundManager::play(SoundManager::SOUND_GAME_START); // Dynamic game intro chime
  spawnPill();
  lastFallTime = millis();
}

void advanceToNextLevel() {
  level++;
  SoundManager::play(SoundManager::SOUND_POWERUP); // Level clear fanfare 
  resetGame();
}

void spawnPill() {
  currentPill.x = 3;
  currentPill.y = 0;
  currentPill.d = 0;
  currentPill.c1 = nextPill.c1;
  currentPill.c2 = nextPill.c2;
  comboChain = 0; 

  nextPill.c1 = random(1, 4);
  nextPill.c2 = random(1, 4);

  if (checkCollision(currentPill.x, currentPill.y, currentPill.d)) {
    gameOver = true;
  } else {
    SoundManager::play(SoundManager::SOUND_MENU_SELECT); // Short "toss" pill sound
  }
  drawSidePanel();
}

void stepGravity() {
  if (gameOver || gamePaused || isCascading) return;

  if (!checkCollision(currentPill.x, currentPill.y + 1, currentPill.d)) {
    currentPill.y++;
  } else {
    int8_t x2 = currentPill.x + OFF_X[currentPill.d];
    int8_t y2 = currentPill.y + OFF_Y[currentPill.d];

    if (currentPill.d == 0) { board[currentPill.y][currentPill.x] = {2, currentPill.c1, LINK_RIGHT}; board[y2][x2] = {2, currentPill.c2, LINK_LEFT}; }
    else if (currentPill.d == 1) { board[currentPill.y][currentPill.x] = {2, currentPill.c1, LINK_UP}; board[y2][x2] = {2, currentPill.c2, LINK_DOWN}; }
    else if (currentPill.d == 2) { board[currentPill.y][currentPill.x] = {2, currentPill.c1, LINK_LEFT}; board[y2][x2] = {2, currentPill.c2, LINK_RIGHT}; }
    else if (currentPill.d == 3) { board[currentPill.y][currentPill.x] = {2, currentPill.c1, LINK_DOWN}; board[y2][x2] = {2, currentPill.c2, LINK_UP}; }

    SoundManager::play(SoundManager::SOUND_MENU_MOVE); // Landing / snap sound
    processMatches();
  }
}

void processMatches() {
  bool matched[boardRows][boardCols];
  memset(matched, false, sizeof(matched));
  bool clearFound = false;

  // Scan Horizontals
  for (int r = 0; r < boardRows; r++) {
    int len = 1;
    for (int c = 1; c <= boardCols; c++) {
      if (c < boardCols && board[r][c].t != 0 && board[r][c].c == board[r][c-1].c) {
        len++;
      } else {
        if (len >= 4) {
          for (int i = c - len; i < c; i++) matched[r][i] = true;
          clearFound = true;
        }
        len = 1;
      }
    }
  }

  // Scan Verticals
  for (int c = 0; c < boardCols; c++) {
    int len = 1;
    for (int r = 1; r <= boardRows; r++) {
      if (r < boardRows && board[r][c].t != 0 && board[r][c].c == board[r-1][c].c) {
        len++;
      } else {
        if (len >= 4) {
          for (int i = r - len; i < r; i++) matched[i][c] = true;
          clearFound = true;
        }
        len = 1;
      }
    }
  }

  if (clearFound) {
    int vClear = 0;
    int pClear = 0;
    
    for (int r = 0; r < boardRows; r++) {
      for (int c = 0; c < boardCols; c++) {
        if (matched[r][c]) {
          if (board[r][c].t == 1) { virusCount--; vClear++; }
          else if (board[r][c].t == 2) { pClear++; }
          board[r][c] = {0, 0, LINK_NONE};
        }
      }
    }

    score += (vClear * 100 + pClear * 10) * (1 << comboChain);
    
    // Play a distinct chime or a dynamic coin multiplier sound based on combo chains
    if (vClear > 0) {
      SoundManager::play(SoundManager::SOUND_COIN);
    } else {
      SoundManager::play(SoundManager::SOUND_SCORE_TALLY);
    }
    comboChain++;

    for (int r = 0; r < boardRows; r++) {
      for (int c = 0; c < boardCols; c++) {
        if (board[r][c].t == 2) {
          if (board[r][c].link == LINK_RIGHT && (c+1 >= boardCols || board[r][c+1].t != 2 || board[r][c+1].link != LINK_LEFT)) board[r][c].link = LINK_NONE;
          if (board[r][c].link == LINK_LEFT && (c-1 < 0 || board[r][c-1].t != 2 || board[r][c-1].link != LINK_RIGHT)) board[r][c].link = LINK_NONE;
          if (board[r][c].link == LINK_DOWN && (r+1 >= boardRows || board[r+1][c].t != 2 || board[r+1][c].link != LINK_UP)) board[r][c].link = LINK_NONE;
          if (board[r][c].link == LINK_UP && (r-1 < 0 || board[r-1][c].t != 2 || board[r-1][c].link != LINK_DOWN)) board[r][c].link = LINK_NONE;
        }
      }
    }

    drawSidePanel();
    isCascading = true;
  } else {
    if (virusCount <= 0) advanceToNextLevel();
    else spawnPill();
  }
}

void runCascadeEngine() {
  bool fallen = false;

  for (int r = boardRows - 2; r >= 0; r--) {
    for (int c = 0; c < boardCols; c++) {
      if (board[r][c].t == 2) {
        if (board[r][c].link == LINK_NONE) { 
          if (board[r+1][c].t == 0) {
            board[r+1][c] = board[r][c];
            board[r][c] = {0, 0, LINK_NONE};
            fallen = true;
          }
        } 
        else if (board[r][c].link == LINK_RIGHT) { 
          if (board[r+1][c].t == 0 && board[r+1][c+1].t == 0) {
            board[r+1][c] = board[r][c];
            board[r+1][c+1] = board[r][c+1];
            board[r][c] = {0, 0, LINK_NONE};
            board[r][c+1] = {0, 0, LINK_NONE};
            fallen = true;
          }
        } 
        else if (board[r][c].link == LINK_UP) { 
          if (board[r+1][c].t == 0) {
            board[r+1][c] = board[r][c];
            board[r][c] = board[r-1][c];
            board[r-1][c] = {0, 0, LINK_NONE};
            fallen = true;
          }
        }
      }
    }
  }

  if (fallen) {
    drawBoard();
    delay(100); 
  } else {
    isCascading = false;
    processMatches();
  }
}

} // namespace DrMarioGame