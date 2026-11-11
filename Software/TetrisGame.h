#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"
namespace TetrisGame {
bool launcherExit = false;
bool shouldExitToLauncher() {
  return launcherExit;
}
const int dacPin = 25;

// TFT LCD Setup
TFT_eSPI tft = TFT_eSPI();
const uint16_t backgroundColor = TFT_BLACK;
const int screenCenterX = 120;
const int screenCenterY = 160;
const int maxX = 239;
const int maxY = 319;

// Tetris Board
const int boardCols = 10;
const int boardRows = 20;
const int pieceSize = 4;
const int cellSize = 13;
const int boardX = 8;
const int boardY = 28;
const int sidePanelX = 150;

uint16_t board[boardRows][boardCols];
const uint16_t emptyCell = TFT_BLACK;

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

// Time configurations
unsigned long globalDebounceTime = 0;
unsigned long lastFallTime = 0;
unsigned long lastSoftDropTime = 0;
const unsigned long debounceDelayMs = 180;
const unsigned long softDropRepeatMs = 85;
const unsigned long startFallDelayMs = 650;
const unsigned long minFallDelayMs = 95;
unsigned long fallDelayMs = startFallDelayMs;

int score = 0;
int clearedLines = 0;
int level = 1;

struct Tetromino {
  int type;
  int rotation;
  int col;
  int row;
};

Tetromino currentPiece;
Tetromino nextPiece;

const uint16_t pieceColors[7] = {
  TFT_CYAN,
  TFT_BLUE,
  TFT_ORANGE,
  TFT_YELLOW,
  TFT_GREEN,
  TFT_MAGENTA,
  TFT_RED
};

const uint16_t pieceMasks[7] = {
  0x0F00, // I
  0x8E00, // J
  0x2E00, // L
  0x6600, // O
  0x6C00, // S
  0x4E00, // T
  0xC600  // Z
};

// Function Declarations
void scanButtons();
bool buttonTapped(buttonName button);
bool pieceCell(int type, int rotation, int x, int y);
bool pieceCollides(Tetromino piece);
void drawStartScreen();
void drawBoardFrame();
void drawBoard();
void drawCell(int col, int row, uint16_t color);
void drawPiece(Tetromino piece, uint16_t color);
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
Tetromino getRandomPiece();
void spawnPiece();
bool movePiece(int deltaCol, int deltaRow, int deltaRotation);
void hardDropPiece();
void softDropPiece();
void lockPiece();
int clearFullLines();
void updateScore(int lineCount);
void processGameInput();
void processGravity();

void setupGame() {
  TetrisGame::tft.init();
  TetrisGame::tft.setRotation(0);
  TetrisGame::tft.fillScreen(TetrisGame::backgroundColor);
  randomSeed(esp_random());

  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void loopGame() {
  TetrisGame::scanButtons();

  if (TetrisGame::onStartScreen) {
    if (TetrisGame::boardNeedsRedraw) {
      TetrisGame::drawStartScreen();
      TetrisGame::boardNeedsRedraw = false;
    }

    if ((TetrisGame::buttonTapped(Start) || TetrisGame::buttonTapped(Enter)) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
      TetrisGame::globalDebounceTime = millis();
      TetrisGame::onStartScreen = false;
      TetrisGame::inGame = true;
      TetrisGame::resetGame();
    }

    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (TetrisGame::gameOver) {
    if (!TetrisGame::endScreenShown) {
      TetrisGame::displayGameOver();
      TetrisGame::endScreenShown = true;
    }

    if ((TetrisGame::buttonTapped(Start) || TetrisGame::buttonTapped(Enter)) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
      TetrisGame::globalDebounceTime = millis();
      TetrisGame::resetGame();
    }

    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (TetrisGame::quitConfirm) {
    if (TetrisGame::buttonTapped(B) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
      TetrisGame::globalDebounceTime = millis();
      TetrisGame::cancelQuitOverlay();
    }
    if (TetrisGame::buttonTapped(Enter) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
      TetrisGame::globalDebounceTime = millis();
      TetrisGame::returnToMenu();
    }
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (TetrisGame::buttonTapped(B) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
    TetrisGame::globalDebounceTime = millis();
    TetrisGame::quitConfirm = true;
    TetrisGame::drawQuitOverlay();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (TetrisGame::buttonTapped(A) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
    TetrisGame::globalDebounceTime = millis();
    TetrisGame::gamePaused = !TetrisGame::gamePaused;
    if (!TetrisGame::gamePaused) {
      TetrisGame::pauseScreenDrawn = false;
      TetrisGame::startResumeCountdown();
    }
  }

  if (TetrisGame::resumeCountdownActive) {
    if (TetrisGame::updateResumeCountdown()) {
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
      delay(10);
      return;
    }
  }

  if (TetrisGame::gamePaused) {
    if (!TetrisGame::pauseScreenDrawn) TetrisGame::drawPauseOverlay();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (TetrisGame::boardNeedsRedraw) {
    TetrisGame::tft.fillScreen(TetrisGame::backgroundColor);
    TetrisGame::drawBoardFrame();
    TetrisGame::drawBoard();
    TetrisGame::drawSidePanel();
    TetrisGame::drawPiece(TetrisGame::currentPiece, TetrisGame::pieceColors[TetrisGame::currentPiece.type]);
    TetrisGame::boardNeedsRedraw = false;
  }

  TetrisGame::processGameInput();
  TetrisGame::processGravity();

  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
  delay(10);
}

void scanButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

bool buttonTapped(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed;
}

bool pieceCell(int type, int rotation, int x, int y) {
  int rotatedX = x;
  int rotatedY = y;

  switch (rotation % 4) {
    case 1:
      rotatedX = y;
      rotatedY = TetrisGame::pieceSize - 1 - x;
      break;
    case 2:
      rotatedX = TetrisGame::pieceSize - 1 - x;
      rotatedY = TetrisGame::pieceSize - 1 - y;
      break;
    case 3:
      rotatedX = TetrisGame::pieceSize - 1 - y;
      rotatedY = x;
      break;
  }

  return (TetrisGame::pieceMasks[type] & (0x8000 >> (rotatedY * TetrisGame::pieceSize + rotatedX))) != 0;
}

bool pieceCollides(Tetromino piece) {
  for (int y = 0; y < TetrisGame::pieceSize; y++) {
    for (int x = 0; x < TetrisGame::pieceSize; x++) {
      if (!TetrisGame::pieceCell(piece.type, piece.rotation, x, y)) continue;

      int boardCol = piece.col + x;
      int boardRow = piece.row + y;

      if (boardCol < 0 || boardCol >= TetrisGame::boardCols || boardRow >= TetrisGame::boardRows) return true;
      if (boardRow >= 0 && TetrisGame::board[boardRow][boardCol] != TetrisGame::emptyCell) return true;
    }
  }
  return false;
}

void drawStartScreen() {
  TetrisGame::tft.fillScreen(TetrisGame::backgroundColor);
  TetrisGame::tft.setTextColor(TFT_CYAN, TetrisGame::backgroundColor);
  TetrisGame::tft.setTextSize(3);
  TetrisGame::tft.setCursor(44, 54);
  TetrisGame::tft.print("TETRIS");

  TetrisGame::tft.drawFastHLine(24, 92, 192, TFT_WHITE);

  TetrisGame::tft.setTextColor(TFT_WHITE, TetrisGame::backgroundColor);
  TetrisGame::tft.setTextSize(1);
  TetrisGame::tft.setCursor(34, 124);
  TetrisGame::tft.print("LEFT/RIGHT: Move");
  TetrisGame::tft.setCursor(34, 142);
  TetrisGame::tft.print("UP: Rotate");
  TetrisGame::tft.setCursor(34, 160);
  TetrisGame::tft.print("DOWN: Soft drop");
  TetrisGame::tft.setCursor(34, 178);
  TetrisGame::tft.print("A: Pause");
  TetrisGame::tft.setCursor(34, 196);
  TetrisGame::tft.print("B: Quit prompt");
  TetrisGame::tft.setCursor(34, 214);
  TetrisGame::tft.print("ENTER: Hard drop");

  TetrisGame::tft.setTextColor(TFT_YELLOW, TetrisGame::backgroundColor);
  TetrisGame::tft.setTextSize(2);
  TetrisGame::tft.setCursor(32, 252);
  TetrisGame::tft.print("START TO PLAY");
}

void drawBoardFrame() {
  TetrisGame::tft.drawRect(TetrisGame::boardX - 2, TetrisGame::boardY - 2, TetrisGame::boardCols * TetrisGame::cellSize + 4, TetrisGame::boardRows * TetrisGame::cellSize + 4, TFT_WHITE);
}

void drawBoard() {
  for (int row = 0; row < TetrisGame::boardRows; row++) {
    for (int col = 0; col < TetrisGame::boardCols; col++) {
      TetrisGame::drawCell(col, row, TetrisGame::board[row][col]);
    }
  }
}

void drawCell(int col, int row, uint16_t color) {
  int x = TetrisGame::boardX + col * TetrisGame::cellSize;
  int y = TetrisGame::boardY + row * TetrisGame::cellSize;
  TetrisGame::tft.fillRect(x + 1, y + 1, TetrisGame::cellSize - 2, TetrisGame::cellSize - 2, color);
  TetrisGame::tft.drawRect(x, y, TetrisGame::cellSize, TetrisGame::cellSize, TFT_DARKGREY);
}

void drawPiece(Tetromino piece, uint16_t color) {
  for (int y = 0; y < TetrisGame::pieceSize; y++) {
    for (int x = 0; x < TetrisGame::pieceSize; x++) {
      if (!TetrisGame::pieceCell(piece.type, piece.rotation, x, y)) continue;

      int boardCol = piece.col + x;
      int boardRow = piece.row + y;
      if (boardRow >= 0 && boardRow < TetrisGame::boardRows && boardCol >= 0 && boardCol < TetrisGame::boardCols) {
        TetrisGame::drawCell(boardCol, boardRow, color);
      }
    }
  }
}

void drawSidePanel() {
  TetrisGame::tft.fillRect(TetrisGame::sidePanelX, 0, TetrisGame::maxX - TetrisGame::sidePanelX + 1, TetrisGame::maxY + 1, TetrisGame::backgroundColor);

  TetrisGame::tft.setTextSize(1);
  TetrisGame::tft.setTextColor(TFT_WHITE, TetrisGame::backgroundColor);
  TetrisGame::tft.setCursor(TetrisGame::sidePanelX, 24);
  TetrisGame::tft.print("NEXT");

  int previewCell = 12;
  int previewX = TetrisGame::sidePanelX + 2;
  int previewY = 44;
  for (int y = 0; y < TetrisGame::pieceSize; y++) {
    for (int x = 0; x < TetrisGame::pieceSize; x++) {
      uint16_t color = TetrisGame::pieceCell(TetrisGame::nextPiece.type, 0, x, y) ? TetrisGame::pieceColors[TetrisGame::nextPiece.type] : TetrisGame::backgroundColor;
      TetrisGame::tft.fillRect(previewX + x * previewCell + 1, previewY + y * previewCell + 1, previewCell - 2, previewCell - 2, color);
    }
  }

  TetrisGame::tft.setCursor(TetrisGame::sidePanelX, 116);
  TetrisGame::tft.print("SCORE");
  TetrisGame::tft.setCursor(TetrisGame::sidePanelX, 130);
  TetrisGame::tft.print(TetrisGame::score);

  TetrisGame::tft.setCursor(TetrisGame::sidePanelX, 160);
  TetrisGame::tft.print("LINES");
  TetrisGame::tft.setCursor(TetrisGame::sidePanelX, 174);
  TetrisGame::tft.print(TetrisGame::clearedLines);

  TetrisGame::tft.setCursor(TetrisGame::sidePanelX, 204);
  TetrisGame::tft.print("LEVEL");
  TetrisGame::tft.setCursor(TetrisGame::sidePanelX, 218);
  TetrisGame::tft.print(TetrisGame::level);
}

void drawPauseOverlay() {
  int boxW = 170;
  int boxH = 78;
  int boxX = (TetrisGame::maxX + 1 - boxW) / 2;
  int boxY = (TetrisGame::maxY + 1 - boxH) / 2;

  TetrisGame::tft.fillRect(boxX, boxY, boxW, boxH, TFT_DARKGREY);
  TetrisGame::tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
  TetrisGame::tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_BLACK);

  TetrisGame::tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  TetrisGame::tft.setTextSize(2);
  TetrisGame::tft.setCursor(boxX + 48, boxY + 18);
  TetrisGame::tft.print("PAUSED");

  TetrisGame::tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  TetrisGame::tft.setTextSize(1);
  TetrisGame::tft.setCursor(boxX + 35, boxY + 52);
  TetrisGame::tft.print("Press A to Resume");
  TetrisGame::pauseScreenDrawn = true;
}

void drawResumeCountdown() {
  int boxW = 132;
  int boxH = 96;
  int boxX = (TetrisGame::maxX + 1 - boxW) / 2;
  int boxY = (TetrisGame::maxY + 1 - boxH) / 2;

  if (!TetrisGame::resumeCountdownBoxDrawn) {
    TetrisGame::tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    TetrisGame::tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    TetrisGame::tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    TetrisGame::tft.setTextSize(2);
    TetrisGame::tft.setCursor(boxX + 24, boxY + 12);
    TetrisGame::tft.print("RESUME");
    TetrisGame::tft.setTextColor(TFT_WHITE, TFT_BLACK);
    TetrisGame::tft.setTextSize(1);
    TetrisGame::tft.setCursor(boxX + 28, boxY + 74);
    TetrisGame::tft.print("A to resume");
    TetrisGame::resumeCountdownBoxDrawn = true;
  }

  TetrisGame::tft.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
  TetrisGame::tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  TetrisGame::tft.setTextSize(4);
  TetrisGame::tft.setCursor(boxX + 50, boxY + 36);
  TetrisGame::tft.print(TetrisGame::resumeCountdownValue);
}

void startResumeCountdown() {
  TetrisGame::resumeCountdownActive = true;
  TetrisGame::resumeCountdownBoxDrawn = false;
  TetrisGame::resumeCountdownValue = 3;
  TetrisGame::resumeCountdownLastMs = millis();
  TetrisGame::drawResumeCountdown();
}

bool updateResumeCountdown() {
  unsigned long now = millis();
  if (now - TetrisGame::resumeCountdownLastMs >= 1000) {
    TetrisGame::resumeCountdownLastMs += 1000;
    TetrisGame::resumeCountdownValue--;
    if (TetrisGame::resumeCountdownValue <= 0) {
      TetrisGame::resumeCountdownActive = false;
      TetrisGame::resumeCountdownBoxDrawn = false;
      return false;
    }
  }

  TetrisGame::drawResumeCountdown();
  return true;
}

void drawQuitOverlay() {
  int boxW = 196;
  int boxH = 88;
  int boxX = (TetrisGame::maxX + 1 - boxW) / 2;
  int boxY = (TetrisGame::maxY + 1 - boxH) / 2;

  TetrisGame::tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  TetrisGame::tft.drawRect(boxX, boxY, boxW, boxH, TFT_YELLOW);
  TetrisGame::tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

  TetrisGame::tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  TetrisGame::tft.setTextSize(2);
  TetrisGame::tft.setCursor(boxX + 42, boxY + 18);
  TetrisGame::tft.print("QUIT GAME?");

  TetrisGame::tft.setTextColor(TFT_WHITE, TFT_BLACK);
  TetrisGame::tft.setTextSize(1);
  TetrisGame::tft.setCursor(boxX + 28, boxY + 54);
  TetrisGame::tft.print("ENTER quits   B cancels");
}

void cancelQuitOverlay() {
  TetrisGame::quitConfirm = false;
  TetrisGame::tft.fillScreen(TetrisGame::backgroundColor);
  TetrisGame::drawBoardFrame();
  TetrisGame::drawBoard();
  TetrisGame::drawSidePanel();
  TetrisGame::drawPiece(TetrisGame::currentPiece, TetrisGame::pieceColors[TetrisGame::currentPiece.type]);
  TetrisGame::boardNeedsRedraw = false;
  TetrisGame::pauseScreenDrawn = false;
  if (TetrisGame::gamePaused) {
    TetrisGame::drawPauseOverlay();
  }
}

void displayGameOver() {
  int boxW = 196;
  int boxH = 94;
  int boxX = (TetrisGame::maxX + 1 - boxW) / 2;
  int boxY = (TetrisGame::maxY + 1 - boxH) / 2;

  TetrisGame::tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  TetrisGame::tft.drawRect(boxX, boxY, boxW, boxH, TFT_RED);
  TetrisGame::tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

  TetrisGame::tft.setTextColor(TFT_RED, TFT_BLACK);
  TetrisGame::tft.setTextSize(2);
  TetrisGame::tft.setCursor(boxX + 38, boxY + 18);
  TetrisGame::tft.print("GAME OVER");

  TetrisGame::tft.setTextColor(TFT_WHITE, TFT_BLACK);
  TetrisGame::tft.setTextSize(1);
  TetrisGame::tft.setCursor(boxX + 42, boxY + 52);
  TetrisGame::tft.print("Score: ");
  TetrisGame::tft.print(TetrisGame::score);
  TetrisGame::tft.setCursor(boxX + 32, boxY + 70);
  TetrisGame::tft.print("START or ENTER");
}

void returnToMenu() {
  TetrisGame::launcherExit = true;
  TetrisGame::onStartScreen = true;
  TetrisGame::inGame = false;
  TetrisGame::gameOver = false;
  TetrisGame::gamePaused = false;
  TetrisGame::quitConfirm = false;
  TetrisGame::resumeCountdownActive = false;
  TetrisGame::boardNeedsRedraw = true;
  TetrisGame::pauseScreenDrawn = false;
  TetrisGame::endScreenShown = false;
  TetrisGame::tft.fillScreen(TetrisGame::backgroundColor);
}

void resetGame() {
  TetrisGame::gameOver = false;
  TetrisGame::gamePaused = false;
  TetrisGame::quitConfirm = false;
  TetrisGame::resumeCountdownActive = false;
  TetrisGame::pauseScreenDrawn = false;
  TetrisGame::endScreenShown = false;
  TetrisGame::boardNeedsRedraw = true;
  TetrisGame::score = 0;
  TetrisGame::clearedLines = 0;
  TetrisGame::level = 1;
  TetrisGame::fallDelayMs = TetrisGame::startFallDelayMs;

  for (int row = 0; row < TetrisGame::boardRows; row++) {
    for (int col = 0; col < TetrisGame::boardCols; col++) {
      TetrisGame::board[row][col] = TetrisGame::emptyCell;
    }
  }

  TetrisGame::nextPiece = TetrisGame::getRandomPiece();
  TetrisGame::spawnPiece();
  TetrisGame::lastFallTime = millis();
  TetrisGame::lastSoftDropTime = 0;
}

Tetromino getRandomPiece() {
  Tetromino piece;
  piece.type = random(0, 7);
  piece.rotation = 0;
  piece.col = 3;
  piece.row = -1;
  return piece;
}

void spawnPiece() {
  TetrisGame::currentPiece = TetrisGame::nextPiece;
  TetrisGame::currentPiece.col = 3;
  TetrisGame::currentPiece.row = -1;
  TetrisGame::nextPiece = TetrisGame::getRandomPiece();
  TetrisGame::drawSidePanel();

  if (TetrisGame::pieceCollides(TetrisGame::currentPiece)) {
    TetrisGame::gameOver = true;
  }
}

bool movePiece(int deltaCol, int deltaRow, int deltaRotation) {
  Tetromino movedPiece = TetrisGame::currentPiece;
  movedPiece.col += deltaCol;
  movedPiece.row += deltaRow;
  movedPiece.rotation = (movedPiece.rotation + deltaRotation + 4) % 4;

  if (TetrisGame::pieceCollides(movedPiece)) return false;

  TetrisGame::drawPiece(TetrisGame::currentPiece, TetrisGame::backgroundColor);
  TetrisGame::currentPiece = movedPiece;
  TetrisGame::drawPiece(TetrisGame::currentPiece, TetrisGame::pieceColors[TetrisGame::currentPiece.type]);
  return true;
}

void hardDropPiece() {
  int droppedRows = 0;
  while (movePiece(0, 1, 0)) {
    droppedRows++;
    delay(8);
  }
  TetrisGame::score += droppedRows * 2;

  TetrisGame::lockPiece();
  int lineCount = TetrisGame::clearFullLines();
  TetrisGame::updateScore(lineCount);
  TetrisGame::drawBoard();
  TetrisGame::spawnPiece();

  if (TetrisGame::gameOver) TetrisGame::displayGameOver();
  else TetrisGame::drawPiece(TetrisGame::currentPiece, TetrisGame::pieceColors[TetrisGame::currentPiece.type]);
}

void softDropPiece() {
  if (TetrisGame::movePiece(0, 1, 0)) {
    TetrisGame::score++;
    TetrisGame::drawSidePanel();
    TetrisGame::lastFallTime = millis();
  } else {
    TetrisGame::processGravity();
  }
}

void lockPiece() {
  for (int y = 0; y < TetrisGame::pieceSize; y++) {
    for (int x = 0; x < TetrisGame::pieceSize; x++) {
      if (!TetrisGame::pieceCell(TetrisGame::currentPiece.type, TetrisGame::currentPiece.rotation, x, y)) continue;

      int boardCol = TetrisGame::currentPiece.col + x;
      int boardRow = TetrisGame::currentPiece.row + y;
      if (boardRow >= 0 && boardRow < TetrisGame::boardRows && boardCol >= 0 && boardCol < TetrisGame::boardCols) {
        TetrisGame::board[boardRow][boardCol] = TetrisGame::pieceColors[TetrisGame::currentPiece.type];
      }
    }
  }
}

int clearFullLines() {
  int lineCount = 0;

  for (int row = TetrisGame::boardRows - 1; row >= 0; row--) {
    bool rowFull = true;
    for (int col = 0; col < TetrisGame::boardCols; col++) {
      if (TetrisGame::board[row][col] == TetrisGame::emptyCell) {
        rowFull = false;
        break;
      }
    }

    if (!rowFull) continue;

    lineCount++;
    for (int pullRow = row; pullRow > 0; pullRow--) {
      for (int col = 0; col < TetrisGame::boardCols; col++) {
        TetrisGame::board[pullRow][col] = TetrisGame::board[pullRow - 1][col];
      }
    }
    for (int col = 0; col < TetrisGame::boardCols; col++) {
      TetrisGame::board[0][col] = TetrisGame::emptyCell;
    }
    row++;
  }

  return lineCount;
}

void updateScore(int lineCount) {
  if (lineCount <= 0) {
    TetrisGame::drawSidePanel();
    return;
  }

  int lineScores[] = {0, 100, 300, 500, 800};
  TetrisGame::score += lineScores[lineCount] * TetrisGame::level;
  TetrisGame::clearedLines += lineCount;
  TetrisGame::level = 1 + TetrisGame::clearedLines / 10;

  unsigned long speedUp = (TetrisGame::level - 1) * 55;
  TetrisGame::fallDelayMs = (TetrisGame::startFallDelayMs > speedUp + TetrisGame::minFallDelayMs) ? TetrisGame::startFallDelayMs - speedUp : TetrisGame::minFallDelayMs;
  TetrisGame::drawSidePanel();
}

void processGameInput() {
  if (TetrisGame::buttonTapped(Left) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
    TetrisGame::globalDebounceTime = millis();
    TetrisGame::movePiece(-1, 0, 0);
  }
  if (TetrisGame::buttonTapped(Right) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
    TetrisGame::globalDebounceTime = millis();
    TetrisGame::movePiece(1, 0, 0);
  }
  if (TetrisGame::buttonTapped(Up) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
    TetrisGame::globalDebounceTime = millis();
    TetrisGame::movePiece(0, 0, 1);
  }
  if (buttons[Down].pressed) {
    if (TetrisGame::buttonTapped(Down) || millis() - TetrisGame::lastSoftDropTime > TetrisGame::softDropRepeatMs) {
      TetrisGame::lastSoftDropTime = millis();
      TetrisGame::softDropPiece();
    }
  } else {
    TetrisGame::lastSoftDropTime = 0;
  }
  if (TetrisGame::buttonTapped(Enter) && (millis() - TetrisGame::globalDebounceTime > TetrisGame::debounceDelayMs)) {
    TetrisGame::globalDebounceTime = millis();
    TetrisGame::hardDropPiece();
  }
}

void processGravity() {
  if (millis() - TetrisGame::lastFallTime < TetrisGame::fallDelayMs) return;
  TetrisGame::lastFallTime = millis();

  if (TetrisGame::movePiece(0, 1, 0)) return;

  TetrisGame::lockPiece();
  int lineCount = TetrisGame::clearFullLines();
  TetrisGame::updateScore(lineCount);
  TetrisGame::drawBoard();
  TetrisGame::spawnPiece();

  if (TetrisGame::gameOver) TetrisGame::displayGameOver();
  else TetrisGame::drawPiece(TetrisGame::currentPiece, TetrisGame::pieceColors[TetrisGame::currentPiece.type]);
}

} // namespace TetrisGame
