#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"
#include "SoundManager.h"        // Added SoundManager Integration

namespace TetrisGame {
const int sdChipSelect = 5;
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

// Time & Modern Mechanic Configurations
unsigned long globalDebounceTime = 0;
unsigned long lastFallTime = 0;
unsigned long lastSoftDropTime = 0;
unsigned long lockTimer = 0;
const unsigned long debounceDelayMs = 180;
const unsigned long softDropRepeatMs = 85;
const unsigned long startFallDelayMs = 700;
const unsigned long minFallDelayMs = 95;
const unsigned long lockDelayMs = 500;
unsigned long fallDelayMs = startFallDelayMs;

bool isLocking = false;
bool hasHeldThisTurn = false;
int heldPieceType = -1;

int score = 0;
int clearedLines = 0;
int level = 1;

int bag[7];
int bagIndex = 7;

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
  0x0F00,  // I
  0x8E00,  // J
  0x2E00,  // L
  0x6600,  // O
  0x6C00,  // S
  0x4E00,  // T
  0xC600   // Z
};

// Function Declarations
void scanButtons();
bool buttonTapped(buttonName button);
bool pieceCell(int type, int rotation, int x, int y);
bool pieceCollides(Tetromino piece);
void fillBag();
int getNextBagPiece();
void holdPiece();
void drawStartScreen();
void drawBoardFrame();
void drawBoard();
void drawCell(int col, int row, uint16_t color);
void drawPiece(Tetromino piece, uint16_t color);
void drawGhostPiece();
void clearGhostPiece();
Tetromino getGhostPosition();
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
      SoundManager::play(SoundManager::SOUND_MENU_SELECT);
      onStartScreen = false;
      inGame = true;
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
    }

    if ((buttonTapped(Start) || buttonTapped(Enter)) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      SoundManager::play(SoundManager::SOUND_MENU_SELECT);
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
    drawGhostPiece();
    drawPiece(currentPiece, pieceColors[currentPiece.type]);
    boardNeedsRedraw = false;
  }

  processGameInput();
  processGravity();

  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
  delay(10);
}

void scanButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

bool buttonTapped(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed;
}

// 7-Bag Randomizer System Implementation
void fillBag() {
  for (int i = 0; i < 7; i++) bag[i] = i;
  for (int i = 6; i > 0; i--) {
    int j = random(0, i + 1);
    int temp = bag[i];
    bag[i] = bag[j];
    bag[j] = temp;
  }
  bagIndex = 0;
}

int getNextBagPiece() {
  if (bagIndex >= 7) fillBag();
  return bag[bagIndex++];
}

bool pieceCell(int type, int rotation, int x, int y) {
  int rotatedX = x;
  int rotatedY = y;

  switch (rotation % 4) {
    case 1:
      rotatedX = y;
      rotatedY = pieceSize - 1 - x;
      break;
    case 2:
      rotatedX = pieceSize - 1 - x;
      rotatedY = pieceSize - 1 - y;
      break;
    case 3:
      rotatedX = pieceSize - 1 - y;
      rotatedY = x;
      break;
  }

  return (pieceMasks[type] & (0x8000 >> (rotatedY * pieceSize + rotatedX))) != 0;
}

bool pieceCollides(Tetromino piece) {
  for (int y = 0; y < pieceSize; y++) {
    for (int x = 0; x < pieceSize; x++) {
      if (!pieceCell(piece.type, piece.rotation, x, y)) continue;

      int boardCol = piece.col + x;
      int boardRow = piece.row + y;

      if (boardCol < 0 || boardCol >= boardCols || boardRow >= boardRows) return true;
      if (boardRow >= 0 && board[boardRow][boardCol] != emptyCell) return true;
    }
  }
  return false;
}

void drawStartScreen() {
  tft.fillScreen(backgroundColor);
  tft.setTextColor(TFT_CYAN, backgroundColor);
  tft.setTextSize(3);
  tft.setCursor(44, 44);
  tft.print("TETRIS");

  tft.drawFastHLine(24, 82, 192, TFT_WHITE);

  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(34, 110);
  tft.print("LEFT/RIGHT: Move");
  tft.setCursor(34, 128);
  tft.print("UP: Rotate");
  tft.setCursor(34, 146);
  tft.print("DOWN: Soft drop");
  tft.setCursor(34, 164);
  tft.print("ENTER: Hard drop");
  tft.setCursor(34, 182);
  tft.print("START: Hold piece");
  tft.setCursor(34, 200);
  tft.print("A: Pause game");
  tft.setCursor(34, 218);
  tft.print("B: Quit prompt");

  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.setTextSize(2);
  tft.setCursor(32, 260);
  tft.print("START TO PLAY");
}

void drawBoardFrame() {
  tft.drawRect(boardX - 2, boardY - 2, boardCols * cellSize + 4, boardRows * cellSize + 4, TFT_WHITE);
}

void drawBoard() {
  for (int row = 0; row < boardRows; row++) {
    for (int col = 0; col < boardCols; col++) {
      drawCell(col, row, board[row][col]);
    }
  }
}

void drawCell(int col, int row, uint16_t color) {
  int x = boardX + col * cellSize;
  int y = boardY + row * cellSize;
  tft.fillRect(x + 1, y + 1, cellSize - 2, cellSize - 2, color);
  tft.drawRect(x, y, cellSize, cellSize, TFT_DARKGREY);
}

void drawPiece(Tetromino piece, uint16_t color) {
  for (int y = 0; y < pieceSize; y++) {
    for (int x = 0; x < pieceSize; x++) {
      if (!pieceCell(piece.type, piece.rotation, x, y)) continue;

      int boardCol = piece.col + x;
      int boardRow = piece.row + y;
      if (boardRow >= 0 && boardRow < boardRows && boardCol >= 0 && boardCol < boardCols) {
        drawCell(boardCol, boardRow, color);
      }
    }
  }
}

Tetromino getGhostPosition() {
  Tetromino ghost = currentPiece;
  while (!pieceCollides(ghost)) {
    ghost.row++;
  }
  ghost.row--;  // Step back to the last valid placement
  return ghost;
}

void drawGhostPiece() {
  Tetromino ghost = getGhostPosition();
  for (int y = 0; y < pieceSize; y++) {
    for (int x = 0; x < pieceSize; x++) {
      if (!pieceCell(ghost.type, ghost.rotation, x, y)) continue;

      int boardCol = ghost.col + x;
      int boardRow = ghost.row + y;
      if (boardRow >= 0 && boardRow < boardRows && boardCol >= 0 && boardCol < boardCols) {
        int px = boardX + boardCol * cellSize;
        int py = boardY + boardRow * cellSize;
        tft.drawRect(px, py, cellSize, cellSize, pieceColors[ghost.type]);
      }
    }
  }
}

void clearGhostPiece() {
  Tetromino ghost = getGhostPosition();
  for (int y = 0; y < pieceSize; y++) {
    for (int x = 0; x < pieceSize; x++) {
      if (!pieceCell(ghost.type, ghost.rotation, x, y)) continue;

      int boardCol = ghost.col + x;
      int boardRow = ghost.row + y;
      if (boardRow >= 0 && boardRow < boardRows && boardCol >= 0 && boardCol < boardCols) {
        int px = boardX + boardCol * cellSize;
        int py = boardY + boardRow * cellSize;
        tft.fillRect(px + 1, py + 1, cellSize - 2, cellSize - 2, backgroundColor);
        tft.drawRect(px, py, cellSize, cellSize, TFT_DARKGREY);
      }
    }
  }
}

void drawSidePanel() {
  // Wipe whole side panel space for clean redraws 
  tft.fillRect(sidePanelX, 0, maxX - sidePanelX + 1, maxY + 1, backgroundColor);

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, backgroundColor);

  int previewCell = 10;
  int px, py;

  // Render HOLD Queue
  tft.setCursor(sidePanelX, 10);
  tft.print("HOLD");
  px = sidePanelX + 2;
  py = 24;
  if (heldPieceType != -1) {
    for (int y = 0; y < pieceSize; y++) {
      for (int x = 0; x < pieceSize; x++) {
        uint16_t color = pieceCell(heldPieceType, 0, x, y) ? pieceColors[heldPieceType] : backgroundColor;
        tft.fillRect(px + x * previewCell + 1, py + y * previewCell + 1, previewCell - 2, previewCell - 2, color);
      }
    }
  }

  // Render NEXT Queue
  tft.setCursor(sidePanelX, 74);
  tft.print("NEXT");
  px = sidePanelX + 2;
  py = 88;
  for (int y = 0; y < pieceSize; y++) {
    for (int x = 0; x < pieceSize; x++) {
      uint16_t color = pieceCell(nextPiece.type, 0, x, y) ? pieceColors[nextPiece.type] : backgroundColor;
      tft.fillRect(px + x * previewCell + 1, py + y * previewCell + 1, previewCell - 2, previewCell - 2, color);
    }
  }

  // Stats Breakdown
  tft.setCursor(sidePanelX, 140);
  tft.print("SCORE");
  tft.setCursor(sidePanelX, 154);
  tft.print(score);

  tft.setCursor(sidePanelX, 184);
  tft.print("LINES");
  tft.setCursor(sidePanelX, 198);
  tft.print(clearedLines);

  tft.setCursor(sidePanelX, 228);
  tft.print("LEVEL");
  tft.setCursor(sidePanelX, 242);
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
  unsigned long now = millis();
  if (now - resumeCountdownLastMs >= 1000) {
    resumeCountdownLastMs += 1000;
    resumeCountdownValue--;
    if (resumeCountdownValue <= 0) {
      resumeCountdownActive = false;
      resumeCountdownBoxDrawn = false;
      boardNeedsRedraw = true;  
      return false;
    }
    drawResumeCountdown();
    SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Resume countdown audio tick
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
  tft.fillScreen(backgroundColor);
  drawBoardFrame();
  drawBoard();
  drawSidePanel();
  drawGhostPiece();
  drawPiece(currentPiece, pieceColors[currentPiece.type]);
  boardNeedsRedraw = false;
  pauseScreenDrawn = false;
  if (gamePaused) {
    drawPauseOverlay();
  }
}

void displayGameOver() {
  SD.begin(sdChipSelect);
  int rank = HighScoreManager::saveScore("tetris", score);

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
  int count = HighScoreManager::loadScores("tetris", highScores);
  if (count > 0) {
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
  pauseScreenDrawn = false;
  endScreenShown = false;
  tft.fillScreen(backgroundColor);
}

void resetGame() {
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  resumeCountdownActive = false;
  pauseScreenDrawn = false;
  endScreenShown = false;
  boardNeedsRedraw = true;
  score = 0;
  clearedLines = 0;
  level = 1;
  fallDelayMs = startFallDelayMs;

  bagIndex = 7; 
  heldPieceType = -1;
  hasHeldThisTurn = false;
  isLocking = false;

  for (int row = 0; row < boardRows; row++) {
    for (int col = 0; col < boardCols; col++) {
      board[row][col] = emptyCell;
    }
  }

  nextPiece.type = getNextBagPiece();
  nextPiece.rotation = 0;
  spawnPiece();
  
  lastFallTime = millis();
  lastSoftDropTime = 0;
  SoundManager::play(SoundManager::SOUND_GAME_START); // Setup game start trigger
}

void holdPiece() {
  if (hasHeldThisTurn) return;

  clearGhostPiece();
  drawPiece(currentPiece, backgroundColor);

  if (heldPieceType == -1) {
    heldPieceType = currentPiece.type;
    spawnPiece(); 
  } else {
    int temp = currentPiece.type;
    currentPiece.type = heldPieceType;
    heldPieceType = temp;
    
    currentPiece.col = 3;
    currentPiece.row = -1;
    currentPiece.rotation = 0;
    
    isLocking = false;
    hasHeldThisTurn = true;
    drawSidePanel();
    
    if (pieceCollides(currentPiece)) {
      gameOver = true;
      SoundManager::play(SoundManager::SOUND_GAME_OVER);
    }
  }

  if (!gameOver) {
    drawPiece(currentPiece, pieceColors[currentPiece.type]);
    drawGhostPiece();
    lastFallTime = millis(); // Reset gravity timer post hold
    SoundManager::play(SoundManager::SOUND_MENU_MOVE); // Piece shift click feedback
  }
}

void spawnPiece() {
  currentPiece = nextPiece;
  currentPiece.col = 3;
  currentPiece.row = -1;
  
  nextPiece.type = getNextBagPiece();
  nextPiece.rotation = 0;
  
  isLocking = false;
  hasHeldThisTurn = false;

  drawSidePanel();

  if (pieceCollides(currentPiece)) {
    gameOver = true;
    SoundManager::play(SoundManager::SOUND_GAME_OVER);
  }
}

bool movePiece(int deltaCol, int deltaRow, int deltaRotation) {
  Tetromino movedPiece = currentPiece;
  movedPiece.col += deltaCol;
  movedPiece.row += deltaRow;
  movedPiece.rotation = (movedPiece.rotation + deltaRotation + 4) % 4;

  bool collided = pieceCollides(movedPiece);

  // Super Rotation System (SRS) Basic Wall Kick Logic Frame
  if (collided && deltaRotation != 0) {
    movedPiece.col += 1;
    if (!pieceCollides(movedPiece)) collided = false;
    else {
      movedPiece.col -= 2;
      if (!pieceCollides(movedPiece)) collided = false;
      else {
        movedPiece.col += 1; // reset horizontal alignment
        movedPiece.row -= 1; // test vertical lift kick
        if (!pieceCollides(movedPiece)) collided = false;
        else {
          movedPiece.row += 1; // return false, rotation strictly blocked
        }
      }
    }
  }

  if (collided) return false;

  clearGhostPiece();
  drawPiece(currentPiece, backgroundColor);
  currentPiece = movedPiece;

  // Track and refresh Lock Delay Window mechanism explicitly
  Tetromino groundTest = currentPiece;
  groundTest.row += 1;
  if (!pieceCollides(groundTest)) {
    isLocking = false;
  } else if (isLocking) {
    lockTimer = millis(); // Refresh delay bounds actively when manipulated
  }

  drawPiece(currentPiece, pieceColors[currentPiece.type]);
  drawGhostPiece();
  return true;
}

void hardDropPiece() {
  int droppedRows = 0;
  while (movePiece(0, 1, 0)) {
    droppedRows++;
  }
  score += droppedRows * 2;
  isLocking = false; // Bypass delays upon hard slam trigger

  lockPiece();
  int lineCount = clearFullLines();
  updateScore(lineCount);
  drawBoard();
  spawnPiece();

  if (gameOver) {
    displayGameOver();
  } else {
    drawPiece(currentPiece, pieceColors[currentPiece.type]);
    drawGhostPiece();
  }
}

void softDropPiece() {
  if (movePiece(0, 1, 0)) {
    score++;
    drawSidePanel();
    lastFallTime = millis();
  } else if (!isLocking) {
    isLocking = true;
    lockTimer = millis();
  }
}

void lockPiece() {
  for (int y = 0; y < pieceSize; y++) {
    for (int x = 0; x < pieceSize; x++) {
      if (!pieceCell(currentPiece.type, currentPiece.rotation, x, y)) continue;

      int boardCol = currentPiece.col + x;
      int boardRow = currentPiece.row + y;
      if (boardRow >= 0 && boardRow < boardRows && boardCol >= 0 && boardCol < boardCols) {
        board[boardRow][boardCol] = pieceColors[currentPiece.type];
      }
    }
  }
  SoundManager::play(SoundManager::SOUND_MENU_MOVE); // Thud/Lock feedback click
}

int clearFullLines() {
  int lineCount = 0;

  for (int row = boardRows - 1; row >= 0; row--) {
    bool rowFull = true;
    for (int col = 0; col < boardCols; col++) {
      if (board[row][col] == emptyCell) {
        rowFull = false;
        break;
      }
    }

    if (!rowFull) continue;

    lineCount++;
    for (int pullRow = row; pullRow > 0; pullRow--) {
      for (int col = 0; col < boardCols; col++) {
        board[pullRow][col] = board[pullRow - 1][col];
      }
    }
    for (int col = 0; col < boardCols; col++) {
      board[0][col] = emptyCell;
    }
    row++;
  }

  return lineCount;
}

void updateScore(int lineCount) {
  if (lineCount <= 0) {
    drawSidePanel();
    return;
  }

  // Classic Scoring logic multiplied dynamically against baseline player execution levels
  int lineScores[] = { 0, 40, 100, 300, 1200 };
  score += lineScores[lineCount] * level;
  clearedLines += lineCount;
  level = 1 + clearedLines / 10;

  unsigned long speedUp = (level - 1) * 45;
  fallDelayMs = (startFallDelayMs > speedUp + minFallDelayMs) ? startFallDelayMs - speedUp : minFallDelayMs;
  drawSidePanel();
  SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Clear line alert tone
}

void processGameInput() {
  if (buttonTapped(Left) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    movePiece(-1, 0, 0);
  }
  if (buttonTapped(Right) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    movePiece(1, 0, 0);
  }
  if (buttonTapped(Up) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    movePiece(0, 0, 1);
  }
  if (buttonTapped(Start) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    holdPiece();
  }
  if (buttons[Down].pressed) {
    if (buttonTapped(Down) || millis() - lastSoftDropTime > softDropRepeatMs) {
      lastSoftDropTime = millis();
      softDropPiece();
    }
  } else {
    lastSoftDropTime = 0;
  }
  if (buttonTapped(Enter) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    hardDropPiece();
  }
}

void processGravity() {
  if (isLocking) {
    if (millis() - lockTimer >= lockDelayMs) {
      isLocking = false;
      lockPiece();
      int lineCount = clearFullLines();
      updateScore(lineCount);
      drawBoard();
      spawnPiece();
      
      if (gameOver) displayGameOver();
      else {
        drawPiece(currentPiece, pieceColors[currentPiece.type]);
        drawGhostPiece();
      }
    }
    return; 
  }

  if (millis() - lastFallTime < fallDelayMs) return;
  lastFallTime = millis();

  Tetromino testPiece = currentPiece;
  testPiece.row += 1;

  if (pieceCollides(testPiece)) {
    isLocking = true;
    lockTimer = millis();
  } else {
    movePiece(0, 1, 0);
  }
}

}  // namespace TetrisGame