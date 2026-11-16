#pragma once
#include <Arduino.h>
#include <SD.h>                  // <-- Added SD library support
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"    // <-- Added HighScoreManager support
#include "SoundManager.h"        // <-- Added SoundManager Integration

namespace MinesweeperGame {

const int sdChipSelect = 5;      // <-- Standard SD card CS pin config
TFT_eSPI tft = TFT_eSPI();
bool launcherExit = false;

bool shouldExitToLauncher() {
  return launcherExit;
}

// ===================== CONSTANTS =====================
const int maxX = 319;
const int maxY = 239;

// Grid: 9x9 cells, each 24x24 pixels with 2px gap
const int gridCols = 9;
const int gridRows = 9;
const int cellSize = 24;
const int gridGap = 2;
const int gridOffsetX = 60;  // Center the 9x9 grid (9*24 + 8*2 = 232, (320-232)/2 = 44, use 60 for centering)
const int gridOffsetY = 20;

const int totalMines = 10;

// ===================== GAME STATE =====================
enum CellState {
  Hidden,
  Revealed,
  Flagged
};

struct Cell {
  bool mine;
  int adjacentMines;
  CellState state;
};

Cell grid[gridRows][gridCols];
int cursorRow = 4;
int cursorCol = 4;
int prevCursorRow = 4;
int prevCursorCol = 4;
int revealedCount = 0;
int flagCount = 0;
bool firstClick = true;
bool gameOver = false;
bool gameWon = false;
bool mineExploded = false;

// Timer State Engine Variables
unsigned long gameStartTime = 0;
unsigned long timeElapsed = 0;
unsigned long pauseStartTime = 0;
unsigned long totalPausedTime = 0;

bool onMenu = true;
bool quitConfirm = false;
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
int lastDrawnCountdownValue = -1;
bool needsDraw = true;
bool endScreenDrawn = false;

unsigned long lastDebounce = 0;
const unsigned long debounceMs = 160;

// Forward declaration to allow cell auto-drawing
void drawCell(int row, int col, bool forceRedraw);

// ===================== BUTTONS =====================
void scanButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

bool tapped(buttonName b) {
  return buttons[b].pressed && !buttons[b].lastPressed;
}

void updateButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

// ===================== GRID HELPERS =====================
int getCellX(int col) {
  return gridOffsetX + col * (cellSize + gridGap);
}

int getCellY(int row) {
  return gridOffsetY + row * (cellSize + gridGap);
}

bool isValidCell(int row, int col) {
  return row >= 0 && row < gridRows && col >= 0 && col < gridCols;
}

// ===================== MINE PLACEMENT =====================
void placeMines(int safeRow, int safeCol) {
  int placed = 0;
  while (placed < totalMines) {
    int r = random(0, gridRows);
    int c = random(0, gridCols);
    // Don't place mine on the first clicked cell or adjacent to it
    if (abs(r - safeRow) <= 1 && abs(c - safeCol) <= 1) continue;
    if (!grid[r][c].mine) {
      grid[r][c].mine = true;
      placed++;
    }
  }
}

void calculateAdjacent() {
  for (int r = 0; r < gridRows; r++) {
    for (int c = 0; c < gridCols; c++) {
      if (grid[r][c].mine) continue;
      int count = 0;
      for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
          if (dr == 0 && dc == 0) continue;
          int nr = r + dr;
          int nc = c + dc;
          if (isValidCell(nr, nc) && grid[nr][nc].mine) count++;
        }
      }
      grid[r][c].adjacentMines = count;
    }
  }
}

// ===================== FLOOD FILL =====================
void revealCell(int row, int col) {
  if (!isValidCell(row, col)) return;
  if (grid[row][col].state != Hidden) return;
  if (grid[row][col].mine) return;

  grid[row][col].state = Revealed;
  revealedCount++;
  
  // Directly draw the revealed cell to fix the core rendering bug
  drawCell(row, col, true);

  // If no adjacent mines, flood fill neighbors
  if (grid[row][col].adjacentMines == 0) {
    for (int dr = -1; dr <= 1; dr++) {
      for (int dc = -1; dc <= 1; dc++) {
        if (dr == 0 && dc == 0) continue;
        revealCell(row + dr, col + dc);
      }
    }
  }
}

// ===================== DRAWING =====================
uint16_t getNumberColor(int n) {
  switch (n) {
    case 1: return TFT_BLUE;
    case 2: return TFT_GREEN;
    case 3: return TFT_RED;
    case 4: return TFT_NAVY;
    case 5: return TFT_MAROON;
    case 6: return TFT_CYAN;
    case 7: return TFT_BLACK;
    case 8: return TFT_DARKGREY;
    default: return TFT_WHITE;
  }
}

void drawCell(int row, int col, bool forceRedraw) {
  int x = getCellX(col);
  int y = getCellY(row);
  Cell& cell = grid[row][col];

  // Clear cell area
  tft.fillRect(x, y, cellSize, cellSize, TFT_BLACK);

  if (cell.state == Hidden) {
    // Draw raised 3D button effect
    tft.fillRect(x, y, cellSize, cellSize, TFT_DARKGREY);
    tft.drawRect(x, y, cellSize, cellSize, TFT_LIGHTGREY);
    // Top-left highlight
    tft.drawFastHLine(x + 1, y + 1, cellSize - 2, TFT_WHITE);
    tft.drawFastVLine(x + 1, y + 1, cellSize - 2, TFT_WHITE);
    // Bottom-right shadow
    tft.drawFastHLine(x + 1, y + cellSize - 2, cellSize - 2, TFT_DARKGREY);
    tft.drawFastVLine(x + cellSize - 2, y + 1, cellSize - 2, TFT_DARKGREY);
  } else if (cell.state == Flagged) {
    // Draw flag
    tft.fillRect(x, y, cellSize, cellSize, TFT_DARKGREY);
    tft.drawRect(x, y, cellSize, cellSize, TFT_LIGHTGREY);
    // Flag pole
    tft.drawFastVLine(x + 6, y + 4, 16, TFT_BLACK);
    // Flag
    tft.fillTriangle(x + 7, y + 4, x + 7, y + 12, x + 18, y + 8, TFT_RED);
  } else if (cell.state == Revealed) {
    // Revealed cell - sunken effect
    tft.fillRect(x, y, cellSize, cellSize, TFT_LIGHTGREY);
    tft.drawRect(x, y, cellSize, cellSize, TFT_DARKGREY);

    if (cell.mine) {
      // Draw mine
      tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 7, TFT_BLACK);
      tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 5, TFT_RED);
      // Mine highlight
      tft.drawPixel(x + cellSize / 2 - 1, y + cellSize / 2 - 2, TFT_WHITE);
      // Mine spikes
      for (int a = 0; a < 8; a++) {
        int sx = x + cellSize / 2 + (int)(cos(a * 0.785) * 8);
        int sy = y + cellSize / 2 + (int)(sin(a * 0.785) * 8);
        tft.drawPixel(sx, sy, TFT_BLACK);
      }
    } else if (cell.adjacentMines > 0) {
      // Draw number
      tft.setTextColor(getNumberColor(cell.adjacentMines), TFT_LIGHTGREY);
      tft.setTextSize(1);
      tft.setCursor(x + 8, y + 6);
      tft.print(cell.adjacentMines);
    }
  }
}

void drawCursor() {
  int x = getCellX(cursorCol);
  int y = getCellY(cursorRow);
  // Draw yellow highlight border
  tft.drawRect(x - 1, y - 1, cellSize + 2, cellSize + 2, TFT_YELLOW);
  tft.drawRect(x - 2, y - 2, cellSize + 4, cellSize + 4, TFT_YELLOW);
}

void clearCursor(int row, int col) {
  // Redraw the cell to remove cursor highlight
  drawCell(row, col, true);
}

void drawHud() {
  tft.fillRect(0, 0, 320, 16, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(4, 4);
  tft.print("MINES: ");
  tft.print(totalMines - flagCount);
  tft.setCursor(114, 4);
  tft.print("CELLS: ");
  tft.print(revealedCount);
  tft.print("/");
  tft.print(gridRows * gridCols - totalMines);
  tft.setCursor(240, 4);
  tft.print("TIME: ");
  tft.print(timeElapsed);
  tft.print("s");
}

void drawGrid() {
  for (int r = 0; r < gridRows; r++) {
    for (int c = 0; c < gridCols; c++) {
      drawCell(r, c, true);
    }
  }
}

void drawMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 30);
  tft.print("MINESWEEPER");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(50, 90);
  tft.print("ARROWS to move cursor");
  tft.setCursor(50, 108);
  tft.print("ENTER to reveal cell");
  tft.setCursor(50, 126);
  tft.print("A to flag/unflag cell");
  tft.setCursor(50, 144);
  tft.print("A to pause, B to quit");

  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(64, 190);
  tft.print("START TO PLAY");
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
    lastDrawnCountdownValue = -1;
  }

  if (resumeCountdownValue != lastDrawnCountdownValue) {
    tft.fillRect(boxX + 42, boxY + 34, 48, 30, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(4);
    tft.setCursor(boxX + 50, boxY + 36);
    tft.print(resumeCountdownValue);
    lastDrawnCountdownValue = resumeCountdownValue;
  }
}

void startResumeCountdown() {
  resumeCountdownActive = true;
  resumeCountdownBoxDrawn = false;
  resumeCountdownValue = 3;
  lastDrawnCountdownValue = -1;
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
      lastDrawnCountdownValue = -1;
      // Accumulate paused time delta when counting down completes
      totalPausedTime += (millis() - pauseStartTime);
      return false;
    }
    drawResumeCountdown();
    SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Timer beep
  }
  return true;
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

void drawEndScreen() {
  // Save or load score from SD card leaderboard system
  int rank = -1;
  unsigned long highScores[HighScoreManager::MAX_SCORES];
  int count = 0;

  SD.begin(sdChipSelect);
  if (gameWon) {
    // Save completion time (lower time is better, handled by HighScoreManager sorting)
    rank = HighScoreManager::saveScore("minesweeper", timeElapsed);
    SoundManager::play(SoundManager::SOUND_GAME_START); // Victory tune
  } else {
    SoundManager::play(SoundManager::SOUND_GAME_OVER); // Defeat tone
  }
  count = HighScoreManager::loadScores("minesweeper", highScores);

  tft.fillScreen(TFT_BLACK);
  drawHud();

  if (gameWon) {
    tft.fillRect(54, 54, 212, 132, TFT_BLACK);
    tft.drawRect(52, 52, 216, 136, TFT_GREEN);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(110, 64);
    tft.print("VICTORY!");
    
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(88, 96);
    tft.print("YOUR TIME: ");
    tft.print(timeElapsed);
    tft.print("s");

    if (count > 0) {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setCursor(88, 114);
      tft.print("BEST TIME: ");
      tft.print(highScores[0]);
      tft.print("s");
    }

    if (rank > 0 && rank <= HighScoreManager::MAX_SCORES) {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(88, 132);
      tft.print("RANK ACHIEVED: #");
      tft.print(rank);
    }
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(102, 162);
    tft.print("START again");
  } else {
    // Game over - reveal all remaining mines cleanly
    for (int r = 0; r < gridRows; r++) {
      for (int c = 0; c < gridCols; c++) {
        if (grid[r][c].mine) {
          grid[r][c].state = Revealed;
          drawCell(r, c, true);
        }
      }
    }
    drawCursor();

    tft.fillRect(54, 76, 212, 98, TFT_BLACK);
    tft.drawRect(52, 74, 216, 102, TFT_RED);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(110, 88);
    tft.print("DEFEAT");
    
    if (count > 0) {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(1);
      tft.setCursor(88, 118);
      tft.print("RECORD BEST: ");
      tft.print(highScores[0]);
      tft.print("s");
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(102, 146);
    tft.print("START again");
  }
}

// ===================== GAME LOGIC =====================
void initGrid() {
  for (int r = 0; r < gridRows; r++) {
    for (int c = 0; c < gridCols; c++) {
      grid[r][c].mine = false;
      grid[r][c].adjacentMines = 0;
      grid[r][c].state = Hidden;
    }
  }
  revealedCount = 0;
  flagCount = 0;
  firstClick = true;
  mineExploded = false;
  cursorRow = 4;
  cursorCol = 4;
  prevCursorRow = 4;
  prevCursorCol = 4;
  timeElapsed = 0;
  totalPausedTime = 0;
}

void resetGame() {
  gameOver = false;
  gameWon = false;
  quitConfirm = false;
  gamePaused = false;
  pauseScreenDrawn = false;
  resumeCountdownActive = false;
  endScreenDrawn = false;
  needsDraw = true;
  initGrid();
  SoundManager::play(SoundManager::SOUND_MENU_SELECT);
}

void checkWin() {
  int totalSafe = gridRows * gridCols - totalMines;
  if (revealedCount >= totalSafe) {
    gameWon = true;
    // Auto-flag remaining mines
    for (int r = 0; r < gridRows; r++) {
      for (int c = 0; c < gridCols; c++) {
        if (grid[r][c].mine && grid[r][c].state == Hidden) {
          grid[r][c].state = Flagged;
          flagCount++;
        }
      }
    }
  }
}

void handleReveal() {
  if (firstClick) {
    firstClick = false;
    placeMines(cursorRow, cursorCol);
    calculateAdjacent();
    gameStartTime = millis(); // Track game starting instant
  }

  Cell& cell = grid[cursorRow][cursorCol];

  if (cell.state == Flagged) return;  // Can't reveal flagged cells
  if (cell.state == Revealed) {
    // Chord: if number matches adjacent flags, reveal remaining
    if (cell.adjacentMines > 0) {
      int adjFlags = 0;
      for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
          if (dr == 0 && dc == 0) continue;
          int nr = cursorRow + dr;
          int nc = cursorCol + dc;
          if (isValidCell(nr, nc) && grid[nr][nc].state == Flagged) adjFlags++;
        }
      }
      if (adjFlags == cell.adjacentMines) {
        bool hitMine = false;
        for (int dr = -1; dr <= 1; dr++) {
          for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = cursorRow + dr;
            int nc = cursorCol + dc;
            if (isValidCell(nr, nc) && grid[nr][nc].state == Hidden) {
              if (grid[nr][nc].mine) {
                hitMine = true;
                mineExploded = true;
                gameOver = true;
                grid[nr][nc].state = Revealed;
                drawCell(nr, nc, true);
              } else {
                revealCell(nr, nc);
              }
            }
          }
        }
        if (hitMine) {
          SoundManager::play(SoundManager::SOUND_PLAYER_DEATH); // Bomb explosion
        } else {
          SoundManager::play(SoundManager::SOUND_COIN); // Successful chord clearing
        }
        checkWin();
      }
    }
    return;
  }

  if (cell.mine) {
    // Game over
    mineExploded = true;
    gameOver = true;
    cell.state = Revealed;
    drawCell(cursorRow, cursorCol, true);
    SoundManager::play(SoundManager::SOUND_PLAYER_DEATH); // Clicked a bomb!
    return;
  }

  revealCell(cursorRow, cursorCol);
  SoundManager::play(SoundManager::SOUND_MENU_MOVE); // Safe tile reveal click
  checkWin();
}

void handleFlag() {
  Cell& cell = grid[cursorRow][cursorCol];
  if (cell.state == Revealed) return;

  if (cell.state == Hidden) {
    cell.state = Flagged;
    flagCount++;
    SoundManager::play(SoundManager::SOUND_POWERUP); // Plant flag sound
  } else if (cell.state == Flagged) {
    cell.state = Hidden;
    flagCount--;
    SoundManager::play(SoundManager::SOUND_MENU_BACK); // Remove flag sound
  }
  drawCell(cursorRow, cursorCol, true);
}

// ===================== SETUP & LOOP =====================
void setupGame() {
  launcherExit = false;
  tft.init();
  tft.setRotation(3);
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
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
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
    if (needsDraw) {
      drawMenu();
      needsDraw = false;
    }
    if ((tapped(Start) || tapped(Enter)) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      onMenu = false;
      resetGame();
      needsDraw = true;
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
      needsDraw = true;
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
      needsDraw = true;
      startResumeCountdown();
    } else {
      pauseStartTime = millis(); // Log pause timestamp
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

  // Live Timer Computation Loop
  if (!firstClick) {
    timeElapsed = (millis() - gameStartTime - totalPausedTime) / 1000;
  }

  // Handle cursor movement
  prevCursorRow = cursorRow;
  prevCursorCol = cursorCol;

  bool cursorMoved = false;
  if (tapped(Up) && cursorRow > 0) { cursorRow--; cursorMoved = true; }
  if (tapped(Down) && cursorRow < gridRows - 1) { cursorRow++; cursorMoved = true; }
  if (tapped(Left) && cursorCol > 0) { cursorCol--; cursorMoved = true; }
  if (tapped(Right) && cursorCol < gridCols - 1) { cursorCol++; cursorMoved = true; }
  
  if (cursorMoved) {
    SoundManager::play(SoundManager::SOUND_SCORE_TALLY); // Soft sub-tick for navigation
  }

  // Handle actions
  if (tapped(Enter) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    handleReveal();
    if (gameOver || gameWon) {
      endScreenDrawn = false;
    }
  }

  if (tapped(A) && millis() - lastDebounce > debounceMs) {
    lastDebounce = millis();
    handleFlag();
  }

  // Draw Engine Router
  if (needsDraw) {
    tft.fillScreen(TFT_BLACK);
    drawHud();
    drawGrid();
    drawCursor();
    needsDraw = false;
  } else {
    // Only redraw cursor alterations and update timer components dynamically
    if (prevCursorRow != cursorRow || prevCursorCol != cursorCol) {
      clearCursor(prevCursorRow, prevCursorCol);
      drawCursor();
    }
    drawHud();
  }

  updateButtons();
  delay(16);
}

}  // namespace MinesweeperGame