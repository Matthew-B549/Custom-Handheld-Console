#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace PacmanGame {

bool launcherExit = false;

bool shouldExitToLauncher() {
  return launcherExit;
}

const int dacPin = 25;

TFT_eSPI tft = TFT_eSPI();
const uint16_t backgroundColor = TFT_BLACK;
const int maxX = 319;
const int maxY = 239;

const int mazeCols = 19;
const int mazeRows = 15;
const int cellSize = 12;
const int mazeX = 46;
const int mazeY = 42;

const char levelMap[mazeRows][mazeCols + 1] = {
  "###################",
  "#o.......#.......o#",
  "#.###.##.#.##.###.#",
  "#.................#",
  "#.###.#.###.#.###.#",
  "#.....#..#..#.....#",
  "#####.## # ##.#####",
  "    #.#     #.#    ",
  "#####.# ### #.#####",
  "#.....#..#..#.....#",
  "#.###.#.###.#.###.#",
  "#........C........#",
  "#.###.##.#.##.###.#",
  "#o.......#.......o#",
  "###################"
};

char maze[mazeRows][mazeCols];

struct Actor {
  int col;
  int row;
  int dirX;
  int dirY;
  uint16_t color;
};

Actor pacman;
Actor ghosts[3];
const int ghostCount = 3;

bool onMenu = true;
bool inGame = false;
bool gameOver = false;
bool gamePaused = false;
bool quitConfirm = false;
bool menuNeedsRedraw = true;
bool boardNeedsRedraw = true;
bool pauseScreenDrawn = false;
bool endScreenShown = false;
bool levelWon = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;

int nextDirX = 1;
int nextDirY = 0;
int score = 0;
int lives = 3;
int pelletsLeft = 0;
unsigned long powerUntilMs = 0;

unsigned long globalDebounceTime = 0;
unsigned long lastPacmanMoveMs = 0;
unsigned long lastGhostMoveMs = 0;
const unsigned long debounceDelayMs = 170;
const unsigned long pacmanMoveDelayMs = 180;
const unsigned long ghostMoveDelayMs = 280;
const unsigned long frightenedMs = 7000;

// Forward Declarations
void scanButtons();
bool buttonTapped(buttonName button);
void drawMenu();
void resetGame();
void loadMaze();
void resetActors();
void drawHud();
void drawMaze();
void drawCell(int col, int row);
void drawActor(Actor actor, bool isPacman);
void eraseActor(Actor actor);
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void drawQuitOverlay();
void cancelQuitOverlay();
void displayGameOver();
void returnToMenu();
void processGameInput();
void movePacman();
void moveGhosts();
bool isWall(int col, int row);
bool canMoveTo(int col, int row);
void eatCell();
void checkGhostCollisions();
void chooseGhostDirection(int index);
int distanceToPacman(int col, int row);

void setupGame() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(backgroundColor);
  randomSeed(esp_random());

  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void loopGame() {
  scanButtons();

  if (onMenu) {
    if (menuNeedsRedraw) {
      drawMenu();
      menuNeedsRedraw = false;
    }

    if ((buttonTapped(Start) || buttonTapped(Enter)) && millis() - globalDebounceTime > debounceDelayMs) {
      globalDebounceTime = millis();
      onMenu = false;
      inGame = true;
      resetGame();
    }

    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (gameOver || levelWon) {
    if (!endScreenShown) {
      displayGameOver();
      endScreenShown = true;
    }
    if ((buttonTapped(Start) || buttonTapped(Enter)) && millis() - globalDebounceTime > debounceDelayMs) {
      globalDebounceTime = millis();
      returnToMenu();
    }
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (quitConfirm) {
    if (buttonTapped(B) && millis() - globalDebounceTime > debounceDelayMs) {
      globalDebounceTime = millis();
      cancelQuitOverlay();
    }
    if (buttonTapped(Enter) && millis() - globalDebounceTime > debounceDelayMs) {
      globalDebounceTime = millis();
      returnToMenu();
    }
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (buttonTapped(B) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    quitConfirm = true;
    drawQuitOverlay();
    for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
    delay(10);
    return;
  }

  if (buttonTapped(A) && millis() - globalDebounceTime > debounceDelayMs) {
    globalDebounceTime = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      startResumeCountdown();
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
    drawHud();
    drawMaze();
    drawActor(pacman, true);
    for (int i = 0; i < ghostCount; i++) drawActor(ghosts[i], false);
    boardNeedsRedraw = false;
  }

  processGameInput();
  movePacman();
  moveGhosts();
  checkGhostCollisions();

  for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = buttons[i].pressed;
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

void drawMenu() {
  tft.setRotation(1);
  tft.fillScreen(backgroundColor);
  tft.setTextColor(TFT_YELLOW, backgroundColor);
  tft.setTextSize(3);
  tft.setCursor(86, 32);
  tft.print("PAC-MAN");
  tft.drawFastHLine(40, 72, 240, TFT_BLUE);

  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(48, 104);
  tft.print("Eat dots, power pellets, and cherries");
  tft.setCursor(48, 122);
  tft.print("Avoid ghosts unless powered up");
  tft.setCursor(48, 140);
  tft.print("A pauses   B asks to quit");

  tft.fillCircle(82, 182, 10, TFT_YELLOW);
  tft.fillCircle(146, 182, 7, TFT_RED);
  tft.fillCircle(200, 182, 4, TFT_WHITE);

  tft.setTextColor(TFT_CYAN, backgroundColor);
  tft.setTextSize(2);
  tft.setCursor(62, 210);
  tft.print("START TO PLAY");
}

void resetGame() {
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  resumeCountdownActive = false;
  pauseScreenDrawn = false;
  endScreenShown = false;
  levelWon = false;
  boardNeedsRedraw = true;
  score = 0;
  lives = 3;
  powerUntilMs = 0;
  loadMaze();
  resetActors();
  lastPacmanMoveMs = millis();
  lastGhostMoveMs = millis();
}

void loadMaze() {
  pelletsLeft = 0;
  for (int row = 0; row < mazeRows; row++) {
    for (int col = 0; col < mazeCols; col++) {
      maze[row][col] = levelMap[row][col];
      if (maze[row][col] == '.' || maze[row][col] == 'o' || maze[row][col] == 'C') pelletsLeft++;
    }
  }
}

void resetActors() {
  pacman = {9, 11, 1, 0, TFT_YELLOW};
  nextDirX = 1;
  nextDirY = 0;
  ghosts[0] = {9, 7, -1, 0, TFT_RED};
  ghosts[1] = {8, 7, 1, 0, TFT_MAGENTA};
  ghosts[2] = {10, 7, 0, -1, TFT_CYAN};
}

void drawHud() {
  tft.fillRect(0, 0, maxX + 1, 34, backgroundColor);
  tft.setTextColor(TFT_WHITE, backgroundColor);
  tft.setTextSize(1);
  tft.setCursor(12, 10);
  tft.print("SCORE ");
  tft.print(score);
  tft.setCursor(128, 10);
  tft.print("LIVES ");
  tft.print(lives);
  if (millis() < powerUntilMs) {
    tft.setTextColor(TFT_CYAN, backgroundColor);
    tft.setCursor(224, 10);
    tft.print("POWER");
  }
}

void drawMaze() {
  for (int row = 0; row < mazeRows; row++) {
    for (int col = 0; col < mazeCols; col++) {
      drawCell(col, row);
    }
  }
}

void drawCell(int col, int row) {
  int x = mazeX + col * cellSize;
  int y = mazeY + row * cellSize;
  char tile = maze[row][col];

  if (tile == '#') {
    tft.fillRect(x, y, cellSize, cellSize, TFT_BLUE);
  } else {
    tft.fillRect(x, y, cellSize, cellSize, backgroundColor);
    if (tile == '.') {
      tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 2, TFT_WHITE);
    } else if (tile == 'o') {
      tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 4, TFT_WHITE);
    } else if (tile == 'C') {
      tft.fillCircle(x + 4, y + 6, 3, TFT_RED);
      tft.fillCircle(x + 8, y + 6, 3, TFT_RED);
      tft.drawPixel(x + 6, y + 3, TFT_GREEN);
    }
  }
}

void drawActor(Actor actor, bool isPacman) {
  int x = mazeX + actor.col * cellSize;
  int y = mazeY + actor.row * cellSize;
  uint16_t color = actor.color;
  if (!isPacman && millis() < powerUntilMs) color = TFT_NAVY;

  if (isPacman) {
    tft.fillCircle(x + cellSize / 2, y + cellSize / 2, 5, color);
    if (actor.dirX == 1) {
      tft.fillTriangle(x + 6, y + 6, x + 11, y + 3, x + 11, y + 9, backgroundColor);
    } else if (actor.dirX == -1) {
      tft.fillTriangle(x + 10, y + 6, x + 5, y + 3, x + 5, y + 9, backgroundColor);
    } else if (actor.dirY == -1) {
      tft.fillTriangle(x + 6, y + 10, x + 3, y + 5, x + 9, y + 5, backgroundColor);
    } else {
      tft.fillTriangle(x + 6, y + 4, x + 3, y + 9, x + 9, y + 9, backgroundColor);
    }
  } else {
    tft.fillRoundRect(x + 1, y + 1, cellSize - 2, cellSize - 2, 3, color);
    tft.fillCircle(x + 4, y + 5, 1, TFT_WHITE);
    tft.fillCircle(x + 8, y + 5, 1, TFT_WHITE);
  }
}

void eraseActor(Actor actor) {
  drawCell(actor.col, actor.row);
}

void drawPauseOverlay() {
  tft.fillRect(86, 78, 148, 70, TFT_BLACK);
  tft.drawRect(84, 76, 152, 74, TFT_WHITE);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(122, 98);
  tft.print("PAUSED");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(104, 126);
  tft.print("Press A to resume");
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

void drawQuitOverlay() {
  tft.fillRect(54, 76, 212, 88, TFT_BLACK);
  tft.drawRect(52, 74, 216, 92, TFT_YELLOW);
  tft.drawRect(54, 76, 212, 88, TFT_WHITE);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(104, 96);
  tft.print("QUIT?");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(88, 132);
  tft.print("ENTER quits   B cancels");
}

void cancelQuitOverlay() {
  quitConfirm = false;
  boardNeedsRedraw = true;
}

void displayGameOver() {
  tft.fillRect(50, 72, 220, 96, TFT_BLACK);
  tft.drawRect(48, 70, 224, 100, levelWon ? TFT_GREEN : TFT_RED);
  tft.setTextColor(levelWon ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(levelWon ? 102 : 88, 92);
  tft.print(levelWon ? "YOU WIN!" : "GAME OVER");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(106, 124);
  tft.print("Score: ");
  tft.print(score);
  tft.setCursor(72, 146);
  tft.print("START or ENTER for menu");
}

void returnToMenu() {
  launcherExit = true;
  onMenu = true;
  inGame = false;
  gameOver = false;
  gamePaused = false;
  quitConfirm = false;
  resumeCountdownActive = false;
  menuNeedsRedraw = true;
  boardNeedsRedraw = true;
  pauseScreenDrawn = false;
  endScreenShown = false;
  levelWon = false;
  tft.fillScreen(backgroundColor);
}

void processGameInput() {
  if (buttonTapped(Up) && pacman.dirY != 1) {
    nextDirX = 0;
    nextDirY = -1;
  }
  if (buttonTapped(Down) && pacman.dirY != -1) {
    nextDirX = 0;
    nextDirY = 1;
  }
  if (buttonTapped(Left) && pacman.dirX != 1) {
    nextDirX = -1;
    nextDirY = 0;
  }
  if (buttonTapped(Right) && pacman.dirX != -1) {
    nextDirX = 1;
    nextDirY = 0;
  }
}

void movePacman() {
  if (millis() - lastPacmanMoveMs < pacmanMoveDelayMs) return;
  lastPacmanMoveMs = millis();

  if (canMoveTo(pacman.col + nextDirX, pacman.row + nextDirY)) {
    pacman.dirX = nextDirX;
    pacman.dirY = nextDirY;
  }

  int newCol = pacman.col + pacman.dirX;
  int newRow = pacman.row + pacman.dirY;
  if (!canMoveTo(newCol, newRow)) return;

  eraseActor(pacman);
  pacman.col = newCol;
  pacman.row = newRow;
  eatCell();
  drawActor(pacman, true);
}

void moveGhosts() {
  if (millis() - lastGhostMoveMs < ghostMoveDelayMs) return;
  lastGhostMoveMs = millis();

  for (int i = 0; i < ghostCount; i++) {
    if (!canMoveTo(ghosts[i].col + ghosts[i].dirX, ghosts[i].row + ghosts[i].dirY) || random(0, 5) == 0) {
      chooseGhostDirection(i);
    }

    eraseActor(ghosts[i]);
    ghosts[i].col += ghosts[i].dirX;
    ghosts[i].row += ghosts[i].dirY;
    drawActor(ghosts[i], false);
  }
}

bool isWall(int col, int row) {
  if (col < 0 || col >= mazeCols || row < 0 || row >= mazeRows) return true;
  return maze[row][col] == '#';
}

bool canMoveTo(int col, int row) {
  return !isWall(col, row);
}

void eatCell() {
  char tile = maze[pacman.row][pacman.col];
  if (tile == '.') {
    score += 10;
    pelletsLeft--;
    maze[pacman.row][pacman.col] = ' ';
  } else if (tile == 'o') {
    score += 50;
    pelletsLeft--;
    powerUntilMs = millis() + frightenedMs;
    maze[pacman.row][pacman.col] = ' ';
  } else if (tile == 'C') {
    score += 100;
    pelletsLeft--;
    maze[pacman.row][pacman.col] = ' ';
  }
  drawHud();
  if (pelletsLeft <= 0) levelWon = true;
}

void checkGhostCollisions() {
  for (int i = 0; i < ghostCount; i++) {
    if (ghosts[i].col != pacman.col || ghosts[i].row != pacman.row) continue;

    if (millis() < powerUntilMs) {
      score += 200;
      eraseActor(ghosts[i]);
      ghosts[i].col = 9;
      ghosts[i].row = 7;
      ghosts[i].dirX = random(0, 2) == 0 ? -1 : 1;
      ghosts[i].dirY = 0;
      drawActor(ghosts[i], false);
      drawHud();
    } else {
      lives--;
      drawHud();
      if (lives <= 0) {
        gameOver = true;
      } else {
        delay(500);
        drawMaze();
        resetActors();
        drawActor(pacman, true);
        for (int ghost = 0; ghost < ghostCount; ghost++) drawActor(ghosts[ghost], false);
      }
    }
  }
}

void chooseGhostDirection(int index) {
  int options[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  int bestDir = -1;
  int bestScore = millis() < powerUntilMs ? -1 : 9999;

  for (int i = 0; i < 4; i++) {
    int dc = options[i][0];
    int dr = options[i][1];
    if (dc == -ghosts[index].dirX && dr == -ghosts[index].dirY) continue;
    int nextCol = ghosts[index].col + dc;
    int nextRow = ghosts[index].row + dr;
    if (!canMoveTo(nextCol, nextRow)) continue;

    int dist = distanceToPacman(nextCol, nextRow);
    if (millis() < powerUntilMs) {
      if (dist > bestScore) {
        bestScore = dist;
        bestDir = i;
      }
    } else {
      if (dist < bestScore) {
        bestScore = dist;
        bestDir = i;
      }
    }
  }

  if (bestDir < 0) {
    ghosts[index].dirX = -ghosts[index].dirX;
    ghosts[index].dirY = -ghosts[index].dirY;
  } else {
    ghosts[index].dirX = options[bestDir][0];
    ghosts[index].dirY = options[bestDir][1];
  }
}

int distanceToPacman(int col, int row) {
  return abs(pacman.col - col) + abs(pacman.row - row);
}

} // namespace PacmanGame