#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"
namespace TicTacToeGame {
bool launcherExit = false;
bool shouldExitToLauncher() {
  return launcherExit;
}

const int dacPin = 25;

// TFT LCD Setup
TFT_eSPI tft = TFT_eSPI();
const uint16_t backgroundColor = TFT_BLACK;
const uint16_t menuBackgroundColor = TFT_BLACK;
uint16_t colorToDraw = 0x0000;

bool drawTemporary = true;
const int screenCenterX = 160;
const int screenCenterY = 120;
const int maxX = 319;  
const int maxY = 239;

// State Engine Variables
bool onSettingsMenu = true;
bool inGame = false;
bool gameOver = false;
bool gamePaused = false;
bool menuNeedsRedraw = true;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
bool quitConfirm = false;

// Grid Drawing Helpers
bool ticTacToeDrawn = false;
bool notYetDrawn = true;

const int leftX = 70;
const int upperY = 30;
const int middleX = 130;
const int middleY = 95;
const int rightX = 190;
const int bottomY = 155;

int currentX = middleX;
int currentY = middleY;
int oldX = currentX;
int oldY = currentY;

bool userMoved = false;

enum boardCellState {
  Empty,
  X,
  O
};

boardCellState board[3][3] = {
  {Empty, Empty, Empty},
  {Empty, Empty, Empty},
  {Empty, Empty, Empty},
};

int currentRow = -1;
int currentCol = -1;
int oldRow = -1;
int oldCol = -1;

int emptyRow = -1;
int emptyCol = -1;
int emptyX = -1;
int emptyY = -1;

bool humanWins = false;
bool computerWins = false;
bool p1Won = false;
bool p2Won = false;
bool tie = false;

enum computerStrategies {
  firstCell,
  randomMove,
  smartComputer,
};

computerStrategies computerStrategy = smartComputer;
bool endScreenShown = false;

int emptyRows[9];
int emptyCols[9];
int emptyCount = 0;

boardCellState humanSymbol = X;
boardCellState computerSymbol = O;
bool isTwoPlayerMode = false; 
bool isPlayer1Turn = true;

// Menu Layout Options
const int menuItemCount = 3;
int selectedMenuItem = 0;
int settingMode = 0;   // 0: Vs CPU, 1: Local 2P
int settingSymbol = 0; // 0: X, 1: O
int settingDiff = 1;   // 0: Easy, 1: Smart

const char* modeLabels[] = {"Vs CPU", "Local 2P"};
const char* symbolLabels[] = {"X", "O"};
const char* diffLabels[] = {"Easy", "Smart"};

// Time debouncer configurations
unsigned long globalDebounceTime = 0;
const unsigned long debounceDelayMs = 250;

// Function Declarations
void drawTicTacToeGrid();
void drawXOrO(int shiftX, int shiftY, boardCellState symbolToDraw);
void clearXorO(int oldX, int oldY, uint16_t backgroundColor);
void getCurrentRowsAndColumns();
void getOldRowsAndColumns();
void checkForWins();
void processUserMovement();
void processComputerMove();
void displayWinner();
void getCoordsFromEmpty(int emptyRow, int emptyCol);
bool hasWin(boardCellState symbol);
bool findWinningMove(boardCellState symbol, int &outRow, int &outCol);
void placeComputerPiece(int row, int col);
void collectEmptyCells();
void resetGame();
void drawSettingsMenu();
void processSettingsMenu();
void drawToggleRow(int y, const char* label, const char* opt1, const char* opt2, int selection, bool isRowSelected);
void applySettingsFromMenu();
void drawPauseOverlay();
void drawResumeCountdown();
void startResumeCountdown();
bool updateResumeCountdown();
void drawQuitOverlay();
void cancelQuitOverlay();
void returnToMenu();
void redrawEntireBoardState();

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
  // Global input scan updates
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }

  // State: Settings Setup Menu
  if (onSettingsMenu) {
    processSettingsMenu();
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    return;
  }

  // State: Game Over Screen Screen Handler
  if (gameOver) {
    if (!endScreenShown) {
      displayWinner();
      endScreenShown = true;
    }
    if (((buttons[Start].pressed && !buttons[Start].lastPressed) || 
        (buttons[Enter].pressed && !buttons[Enter].lastPressed)) && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      onSettingsMenu = true;
      inGame = false;
      menuNeedsRedraw = true;
      delay(150); 
    }
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  if (quitConfirm) {
    if (buttons[B].pressed && !buttons[B].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      cancelQuitOverlay();
    }
    if (buttons[Enter].pressed && !buttons[Enter].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
      globalDebounceTime = millis();
      returnToMenu();
    }
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  if (buttons[B].pressed && !buttons[B].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    quitConfirm = true;
    drawQuitOverlay();
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  // Game Pause Control Gate (Button A)
  if (buttons[A].pressed && !buttons[A].lastPressed && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    gamePaused = !gamePaused;
    if (!gamePaused) {
      pauseScreenDrawn = false;
      startResumeCountdown();
    }
  }

  if (resumeCountdownActive) {
    if (updateResumeCountdown()) {
      for (int i = 0; i < buttonCount; i++) {
        buttons[i].lastPressed = buttons[i].pressed;
      }
      delay(10);
      return;
    }
  }

  if (gamePaused) {
    if (!pauseScreenDrawn) {
      drawPauseOverlay();
    }
    for (int i = 0; i < buttonCount; i++) {
      buttons[i].lastPressed = buttons[i].pressed;
    }
    delay(10);
    return;
  }

  // State: Core Match Engine Active Frame
  if (!ticTacToeDrawn) {
    drawTicTacToeGrid();
  }
  
  oldX = currentX;
  oldY = currentY;

  processUserMovement();
  
  if (oldX != currentX || oldY != currentY || notYetDrawn) {
    getOldRowsAndColumns();
    if (!notYetDrawn && board[oldRow][oldCol] == Empty) {
      clearXorO(oldX, oldY, backgroundColor);
    } else if (!notYetDrawn && board[oldRow][oldCol] != Empty) {
      drawTemporary = false;
      drawXOrO(oldX, oldY, board[oldRow][oldCol]);
    }
    getCurrentRowsAndColumns();
    if (board[currentRow][currentCol] == Empty) {
      drawTemporary = true;
      boardCellState turnSymbol = (isTwoPlayerMode) ? (isPlayer1Turn ? humanSymbol : computerSymbol) : humanSymbol;
      drawXOrO(currentX, currentY, turnSymbol);
    }
    notYetDrawn = false;
  }
  
  checkForWins();
  
  if (userMoved && !gameOver) {
    userMoved = false;
    if (!isTwoPlayerMode) {
      delay(600);
      processComputerMove();
      checkForWins();
    } else {
      isPlayer1Turn = !isPlayer1Turn;
      notYetDrawn = true; 
    }
  } else if (userMoved) {
    userMoved = false;
  }

  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
  delay(10);
}

// Function to redraw locked elements post-unpause frame
void redrawEntireBoardState() {
  drawTemporary = false; 
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      if (board[row][col] != Empty) {
        getCoordsFromEmpty(row, col);
        drawXOrO(emptyX, emptyY, board[row][col]);
      }
    }
  }
}

// Visual Pause Overlay System Function
void drawPauseOverlay() {
  int boxW = 200;
  int boxH = 80;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_DARKGREY);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setCursor(boxX + 44, boxY + 18);
  tft.print("PAUSED");

  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextSize(1);
  tft.setCursor(boxX + 38, boxY + 50);
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
      return false;
    }
  }

  drawResumeCountdown();
  return true;
}

void drawQuitOverlay() {
  int boxW = 210;
  int boxH = 84;
  int boxX = (maxX + 1 - boxW) / 2;
  int boxY = (maxY + 1 - boxH) / 2;

  tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
  tft.drawRect(boxX, boxY, boxW, boxH, TFT_YELLOW);
  tft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(boxX + 50, boxY + 18);
  tft.print("QUIT GAME?");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(boxX + 34, boxY + 54);
  tft.print("ENTER quits   B cancels");
}

void cancelQuitOverlay() {
  quitConfirm = false;
  tft.fillScreen(backgroundColor);
  drawTicTacToeGrid();
  redrawEntireBoardState();
  notYetDrawn = true;
  pauseScreenDrawn = false;
  if (gamePaused) {
    drawPauseOverlay();
  }
}

void returnToMenu() {
  launcherExit = true;
  quitConfirm = false;
  onSettingsMenu = true;
  inGame = false;
  gameOver = false;
  gamePaused = false;
  resumeCountdownActive = false;
  menuNeedsRedraw = true;
  pauseScreenDrawn = false;
  endScreenShown = false;
  tft.fillScreen(menuBackgroundColor);
}

// 2-Option Text Toggle Layout Component
void drawToggleRow(int y, const char* label, const char* opt1, const char* opt2, int selection, bool isRowSelected) {
  uint16_t titleColor = isRowSelected ? TFT_YELLOW : TFT_WHITE;
  tft.setTextColor(titleColor, menuBackgroundColor);
  tft.setTextSize(2);
  tft.setCursor(15, y);
  tft.print(label);

  int opt1X = 120;
  int opt2X = 220;

  if (selection == 0) {
    tft.setTextColor(TFT_GREEN, menuBackgroundColor);
    tft.setCursor(opt1X, y);
    tft.print(opt1);

    tft.setTextColor(TFT_DARKGREY, menuBackgroundColor);
    tft.setCursor(opt2X, y);
    tft.print(opt2);
  } else {
    tft.setTextColor(TFT_DARKGREY, menuBackgroundColor);
    tft.setCursor(opt1X, y);
    tft.print(opt1);

    tft.setTextColor(TFT_GREEN, menuBackgroundColor);
    tft.setCursor(opt2X, y);
    tft.print(opt2);
  }
}

void drawSettingsMenu() {
  tft.fillScreen(menuBackgroundColor);

  tft.setTextColor(TFT_CYAN, menuBackgroundColor);
  tft.setTextSize(2);
  tft.setCursor(55, 15);
  tft.print("TIC-TAC-TOE SETUP");
  tft.drawFastHLine(20, 38, 280, TFT_WHITE);

  drawToggleRow(60,  "Mode:",   "Vs CPU", "Local 2P", settingMode,   (selectedMenuItem == 0));
  drawToggleRow(110, "Symbol:", "X",      "O",        settingSymbol, (selectedMenuItem == 1));
  drawToggleRow(160, "Config:", "Easy",   "Smart",    settingDiff,   (selectedMenuItem == 2));

  tft.drawFastHLine(20, 202, 280, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, menuBackgroundColor);
  tft.setTextSize(1);
  tft.setCursor(35, 212);
  tft.print("UP/DOWN: Select Row  |  LEFT/RIGHT: Toggle");
  tft.setCursor(32, 225);
  tft.print("Press START or ENTER to save and begin match");
}

void processSettingsMenu() {
  bool changed = false;
  
  if ((buttons[Up].pressed && !buttons[Up].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    selectedMenuItem = (selectedMenuItem + menuItemCount - 1) % menuItemCount;
    changed = true;
  }
  if ((buttons[Down].pressed && !buttons[Down].lastPressed) && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    selectedMenuItem = (selectedMenuItem + 1) % menuItemCount;
    changed = true;
  }
  if (((buttons[Left].pressed && !buttons[Left].lastPressed) || (buttons[Right].pressed && !buttons[Right].lastPressed)) 
      && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    switch (selectedMenuItem) {
      case 0: settingMode = (settingMode == 0) ? 1 : 0; break;
      case 1: settingSymbol = (settingSymbol == 0) ? 1 : 0; break;
      case 2: settingDiff = (settingDiff == 0) ? 1 : 0; break;
    }
    changed = true;
  }
  
  if (((buttons[Start].pressed && !buttons[Start].lastPressed) || (buttons[Enter].pressed && !buttons[Enter].lastPressed))
      && (millis() - globalDebounceTime > debounceDelayMs)) {
    globalDebounceTime = millis();
    applySettingsFromMenu();
    onSettingsMenu = false;
    inGame = true;
    resetGame();
    return;
  }

  if (changed || menuNeedsRedraw) {
    drawSettingsMenu();
    menuNeedsRedraw = false;
  }
}

void applySettingsFromMenu() {
  TicTacToeGame::isTwoPlayerMode = (TicTacToeGame::settingMode == 1);
  
  if (TicTacToeGame::settingSymbol == 0) {
    TicTacToeGame::humanSymbol = TicTacToeGame::X;
    TicTacToeGame::computerSymbol = TicTacToeGame::O;
  } else {
    TicTacToeGame::humanSymbol = TicTacToeGame::O;
    TicTacToeGame::computerSymbol = TicTacToeGame::X;
  }

  if (TicTacToeGame::settingDiff == 0) {
    TicTacToeGame::computerStrategy = TicTacToeGame::randomMove;
  } else {
    TicTacToeGame::computerStrategy = TicTacToeGame::smartComputer;
  }
}

void processUserMovement(){
  for (int i = 0; i < buttonCount; i++) {
    if (buttons[i].pressed && !buttons[i].lastPressed) {
      switch (i) {
        case 1: // UP
          if (TicTacToeGame::currentY == TicTacToeGame::middleY) TicTacToeGame::currentY = TicTacToeGame::upperY;
          else if (TicTacToeGame::currentY == TicTacToeGame::bottomY) TicTacToeGame::currentY = TicTacToeGame::middleY;
          TicTacToeGame::drawTemporary = true;
          break;
        case 2: // DOWN
          if (TicTacToeGame::currentY == TicTacToeGame::upperY) TicTacToeGame::currentY = TicTacToeGame::middleY;
          else if (TicTacToeGame::currentY == TicTacToeGame::middleY) TicTacToeGame::currentY = TicTacToeGame::bottomY;
          TicTacToeGame::drawTemporary = true;
          break;
        case 3: // LEFT
          if (TicTacToeGame::currentX == TicTacToeGame::middleX) TicTacToeGame::currentX = TicTacToeGame::leftX;
          else if (TicTacToeGame::currentX == TicTacToeGame::rightX) TicTacToeGame::currentX = TicTacToeGame::middleX;
          TicTacToeGame::drawTemporary = true;
          break;
        case 4: // RIGHT
          if (TicTacToeGame::currentX == TicTacToeGame::leftX) TicTacToeGame::currentX = TicTacToeGame::middleX;
          else if (TicTacToeGame::currentX == TicTacToeGame::middleX) TicTacToeGame::currentX = TicTacToeGame::rightX;
          TicTacToeGame::drawTemporary = true;
          break;
        case 8: // ENTER
          TicTacToeGame::getCurrentRowsAndColumns();
          if (TicTacToeGame::board[TicTacToeGame::currentRow][TicTacToeGame::currentCol] == TicTacToeGame::Empty) {
            TicTacToeGame::drawTemporary = false;
            TicTacToeGame::boardCellState currentTurnSymbol = (TicTacToeGame::isTwoPlayerMode) ? (TicTacToeGame::isPlayer1Turn ? TicTacToeGame::humanSymbol : TicTacToeGame::computerSymbol) : TicTacToeGame::humanSymbol;
            TicTacToeGame::drawXOrO(TicTacToeGame::currentX, TicTacToeGame::currentY, currentTurnSymbol);
            TicTacToeGame::board[TicTacToeGame::currentRow][TicTacToeGame::currentCol] = currentTurnSymbol;
            TicTacToeGame::userMoved = true;
          }
          break;
      }
    }
  }
}

void drawTicTacToeGrid(){
  TicTacToeGame::tft.fillRect(69, 85, 180, 10, TFT_WHITE);
  TicTacToeGame::tft.fillRect(69, 145, 180, 10, TFT_WHITE);
  TicTacToeGame::tft.fillRect(119, 29, 10, 180, TFT_WHITE);
  TicTacToeGame::tft.fillRect(179, 29, 10, 180, TFT_WHITE);
  TicTacToeGame::ticTacToeDrawn = true;
}

void drawXOrO(int shiftX, int shiftY, TicTacToeGame::boardCellState symbolToDraw){
  const uint16_t (*image)[45] = nullptr;
  if(symbolToDraw == TicTacToeGame::X){
    image = XImage;
  }
  else if(symbolToDraw == TicTacToeGame::O){
    image = OImage;
  }
  else {
    return;
  }
  for(int y = 0; y < 45; y++){
    for(int x = 0; x < 45; x++){
      TicTacToeGame::colorToDraw = image[y][x];
      if(TicTacToeGame::drawTemporary){
        if(image[y][x] != 0x0000){
          TicTacToeGame::colorToDraw = (symbolToDraw == TicTacToeGame::X) ? 0xFFE0 : 0x07FF; // X = Yellow, O = Cyan
        }
      }
      if(TicTacToeGame::colorToDraw != 0x0000 || !TicTacToeGame::drawTemporary) {
         TicTacToeGame::tft.drawPixel(x + shiftX, y + shiftY, TicTacToeGame::colorToDraw);
      }
    }
  }
}

void clearXorO(int oldX, int oldY, uint16_t backgroundColor){
  TicTacToeGame::tft.fillRect(oldX, oldY, 45, 45, backgroundColor);
}

void getCurrentRowsAndColumns(){
  if (TicTacToeGame::currentX == TicTacToeGame::leftX) TicTacToeGame::currentCol = 0;
  else if (TicTacToeGame::currentX == TicTacToeGame::middleX) TicTacToeGame::currentCol = 1;
  else if (TicTacToeGame::currentX == TicTacToeGame::rightX) TicTacToeGame::currentCol = 2;

  if (TicTacToeGame::currentY == TicTacToeGame::upperY) TicTacToeGame::currentRow = 0;
  else if (TicTacToeGame::currentY == TicTacToeGame::middleY) TicTacToeGame::currentRow = 1;
  else if (TicTacToeGame::currentY == TicTacToeGame::bottomY) TicTacToeGame::currentRow = 2;
}

void getOldRowsAndColumns(){
  if (TicTacToeGame::oldX == TicTacToeGame::leftX) TicTacToeGame::oldCol = 0;
  else if (TicTacToeGame::oldX == TicTacToeGame::middleX) TicTacToeGame::oldCol = 1;
  else if (TicTacToeGame::oldX == TicTacToeGame::rightX) TicTacToeGame::oldCol = 2;

  if (TicTacToeGame::oldY == TicTacToeGame::upperY) TicTacToeGame::oldRow = 0;
  else if (TicTacToeGame::oldY == TicTacToeGame::middleY) TicTacToeGame::oldRow = 1;
  else if (TicTacToeGame::oldY == TicTacToeGame::bottomY) TicTacToeGame::oldRow = 2;
}

void checkForWins(){
  TicTacToeGame::boardCellState winningSymbol = TicTacToeGame::Empty;

  for(int x = 0; x < 3; x++){
    if(TicTacToeGame::board[0][x] == TicTacToeGame::board[1][x] && TicTacToeGame::board[1][x] == TicTacToeGame::board[2][x] && TicTacToeGame::board[0][x] != TicTacToeGame::Empty){
      winningSymbol = TicTacToeGame::board[0][x];
      TicTacToeGame::gameOver = true;
    }
  }
  for(int y = 0; y < 3; y++){
    if(TicTacToeGame::board[y][0] == TicTacToeGame::board[y][1] && TicTacToeGame::board[y][1] == TicTacToeGame::board[y][2] && TicTacToeGame::board[y][0] != TicTacToeGame::Empty){
      winningSymbol = TicTacToeGame::board[y][0];
      TicTacToeGame::gameOver = true;
    }
  }
  if(TicTacToeGame::board[0][0] == TicTacToeGame::board[1][1] && TicTacToeGame::board[1][1] == TicTacToeGame::board[2][2] && TicTacToeGame::board[0][0] != TicTacToeGame::Empty){
    winningSymbol = TicTacToeGame::board[0][0];
    TicTacToeGame::gameOver = true;
  }
  else if(TicTacToeGame::board[2][0] == TicTacToeGame::board[1][1] && TicTacToeGame::board[1][1] == TicTacToeGame::board[0][2] && TicTacToeGame::board[2][0] != TicTacToeGame::Empty){
    winningSymbol = TicTacToeGame::board[2][0];
    TicTacToeGame::gameOver = true;
  }
  
  if (TicTacToeGame::gameOver && winningSymbol != TicTacToeGame::Empty) {
    if (TicTacToeGame::isTwoPlayerMode) {
      if (winningSymbol == TicTacToeGame::humanSymbol) TicTacToeGame::p1Won = true;
      else TicTacToeGame::p2Won = true;
    } else {
      if (winningSymbol == TicTacToeGame::humanSymbol) TicTacToeGame::humanWins = true;
      else TicTacToeGame::computerWins = true;
    }
    return;
  }

  bool anyEmpty = false;
  for(int x = 0; x < 3; x++){
    for(int y = 0; y < 3; y++){
      if(TicTacToeGame::board[x][y] == TicTacToeGame::Empty) anyEmpty = true;
    }
  }
  if(!anyEmpty){
    TicTacToeGame::gameOver = true;
    TicTacToeGame::tie = true;
  }
}

void processComputerMove(){
  if(TicTacToeGame::computerStrategy == TicTacToeGame::firstCell){
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(TicTacToeGame::board[i][j] == TicTacToeGame::Empty){
          TicTacToeGame::placeComputerPiece(i, j);
          return;
        }
      }
    }
  }
  else if(TicTacToeGame::computerStrategy == TicTacToeGame::randomMove){
    TicTacToeGame::collectEmptyCells();
    if(TicTacToeGame::emptyCount == 0) return;
    int choice = random(0, TicTacToeGame::emptyCount);
    TicTacToeGame::placeComputerPiece(TicTacToeGame::emptyRows[choice], TicTacToeGame::emptyCols[choice]);
    return;
  }
  else if(TicTacToeGame::computerStrategy == TicTacToeGame::smartComputer){
    int moveRow = -1;
    int moveCol = -1;
    if(TicTacToeGame::findWinningMove(TicTacToeGame::computerSymbol, moveRow, moveCol)){
      TicTacToeGame::placeComputerPiece(moveRow, moveCol);
      return;
    }
    if(TicTacToeGame::findWinningMove(TicTacToeGame::humanSymbol, moveRow, moveCol)){
      TicTacToeGame::placeComputerPiece(moveRow, moveCol);
      return;
    }
    if(TicTacToeGame::board[1][1] == TicTacToeGame::Empty){
      TicTacToeGame::placeComputerPiece(1, 1);
      return;
    }

    int cornerRows[] = {0, 0, 2, 2};
    int cornerCols[] = {0, 2, 0, 2};
    int cornerStart = random(0, 4);
    for(int k = 0; k < 4; k++){
      int idx = (cornerStart + k) % 4;
      if(TicTacToeGame::board[cornerRows[idx]][cornerCols[idx]] == TicTacToeGame::Empty){
        TicTacToeGame::placeComputerPiece(cornerRows[idx], cornerCols[idx]);
        return;
      }
    }

    int sideRows[] = {0, 1, 1, 2};
    int sideCols[] = {1, 0, 2, 1};
    int sideStart = random(0, 4);
    for(int k = 0; k < 4; k++){
      int idx = (sideStart + k) % 4;
      if(TicTacToeGame::board[sideRows[idx]][sideCols[idx]] == TicTacToeGame::Empty){
        TicTacToeGame::placeComputerPiece(sideRows[idx], sideCols[idx]);
        return;
      }
    }

    TicTacToeGame::collectEmptyCells();
    if(TicTacToeGame::emptyCount > 0){
      int choice = random(0, TicTacToeGame::emptyCount);
      TicTacToeGame::placeComputerPiece(TicTacToeGame::emptyRows[choice], TicTacToeGame::emptyCols[choice]);
    }
  }
}

void displayWinner(){
  for(int y = 0; y < 119; y++){
    for(int x = 0; x < 159; x++){
      uint16_t pixelColor = TicTacToeGame::tie ? tieScreen[y][x] : ((TicTacToeGame::humanWins || TicTacToeGame::p1Won || TicTacToeGame::p2Won) ? winScreen[y][x] : loseScreen[y][x]);
      TicTacToeGame::tft.fillRect(2*x, 2*y, 2, 2, pixelColor);
    }
  }

  if (TicTacToeGame::isTwoPlayerMode && !TicTacToeGame::tie) {
    int boxW = 200;
    int boxH = 50;
    int boxX = (TicTacToeGame::maxX + 1 - boxW) / 2;
    int boxY = (TicTacToeGame::maxY + 1 - boxH) / 2 - 10;
    
    TicTacToeGame::tft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    TicTacToeGame::tft.drawRect(boxX, boxY, boxW, boxH, TFT_WHITE);
    TicTacToeGame::tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    TicTacToeGame::tft.setTextSize(2);
    if (TicTacToeGame::p1Won) {
      TicTacToeGame::tft.setCursor(boxX + 22, boxY + 16);
      TicTacToeGame::tft.print("PLAYER 1 WINS!");
    } else if (TicTacToeGame::p2Won) {
      TicTacToeGame::tft.setCursor(boxX + 22, boxY + 16);
      TicTacToeGame::tft.print("PLAYER 2 WINS!");
    }
  }
}

void getCoordsFromEmpty(int emptyRow, int emptyCol){
  if (emptyCol == 0) TicTacToeGame::emptyX = TicTacToeGame::leftX;
  else if (emptyCol == 1) TicTacToeGame::emptyX = TicTacToeGame::middleX;
  else if (emptyCol == 2) TicTacToeGame::emptyX = TicTacToeGame::rightX;

  if (emptyRow == 0) TicTacToeGame::emptyY = TicTacToeGame::upperY;
  else if (emptyRow == 1) TicTacToeGame::emptyY = TicTacToeGame::middleY;
  else if (emptyRow == 2) TicTacToeGame::emptyY = TicTacToeGame::bottomY;
}

void placeComputerPiece(int row, int col){
  TicTacToeGame::drawTemporary = false;
  TicTacToeGame::getCoordsFromEmpty(row, col);
  TicTacToeGame::drawXOrO(TicTacToeGame::emptyX, TicTacToeGame::emptyY, TicTacToeGame::computerSymbol);
  TicTacToeGame::board[row][col] = TicTacToeGame::computerSymbol;
}

void collectEmptyCells(){
  TicTacToeGame::emptyCount = 0;
  for(int i = 0; i < 3; i++){
    for(int j = 0; j < 3; j++){
      if(TicTacToeGame::board[i][j] == TicTacToeGame::Empty){
        TicTacToeGame::emptyRows[TicTacToeGame::emptyCount] = i;
        TicTacToeGame::emptyCols[TicTacToeGame::emptyCount] = j;
        TicTacToeGame::emptyCount++;
      }
    }
  }
}

bool hasWin(TicTacToeGame::boardCellState symbol){
  for(int col = 0; col < 3; col++){
    if(TicTacToeGame::board[0][col] == symbol && TicTacToeGame::board[1][col] == symbol && TicTacToeGame::board[2][col] == symbol) return true;
  }
  for(int row = 0; row < 3; row++){
    if(TicTacToeGame::board[row][0] == symbol && TicTacToeGame::board[row][1] == symbol && TicTacToeGame::board[row][2] == symbol) return true;
  }
  if(TicTacToeGame::board[0][0] == symbol && TicTacToeGame::board[1][1] == symbol && TicTacToeGame::board[2][2] == symbol) return true;
  if(TicTacToeGame::board[2][0] == symbol && TicTacToeGame::board[1][1] == symbol && TicTacToeGame::board[0][2] == symbol) return true;
  return false;
}

bool findWinningMove(TicTacToeGame::boardCellState symbol, int &outRow, int &outCol){
  for(int row = 0; row < 3; row++){
    for(int col = 0; col < 3; col++){
      if(TicTacToeGame::board[row][col] == TicTacToeGame::Empty){
        TicTacToeGame::board[row][col] = symbol;
        bool wins = TicTacToeGame::hasWin(symbol);
        TicTacToeGame::board[row][col] = TicTacToeGame::Empty;
        if(wins){
          outRow = row;
          outCol = col;
          return true;
        }
      }
    }
  }
  return false;
}

void resetGame(){
  TicTacToeGame::gameOver = false;
  TicTacToeGame::gamePaused = false;
  TicTacToeGame::quitConfirm = false;
  TicTacToeGame::resumeCountdownActive = false;
  TicTacToeGame::humanWins = false;
  TicTacToeGame::computerWins = false;
  TicTacToeGame::p1Won = false;
  TicTacToeGame::p2Won = false;
  TicTacToeGame::tie = false;
  TicTacToeGame::userMoved = false;
  TicTacToeGame::notYetDrawn = true;
  TicTacToeGame::endScreenShown = false;
  TicTacToeGame::pauseScreenDrawn = false;
  TicTacToeGame::isPlayer1Turn = true;
  TicTacToeGame::emptyCount = 0;
  TicTacToeGame::currentX = TicTacToeGame::middleX;
  TicTacToeGame::currentY = TicTacToeGame::middleY;
  TicTacToeGame::currentRow = -1;
  TicTacToeGame::currentCol = -1;
  TicTacToeGame::oldRow = -1;
  TicTacToeGame::oldCol = -1;

  for(int row = 0; row < 3; row++){
    for(int col = 0; col < 3; col++){
      TicTacToeGame::board[row][col] = TicTacToeGame::Empty;
    }
  }

  if (TicTacToeGame::settingSymbol == 0) {
    TicTacToeGame::humanSymbol = TicTacToeGame::X;
    TicTacToeGame::computerSymbol = TicTacToeGame::O;
  } else {
    TicTacToeGame::humanSymbol = TicTacToeGame::O;
    TicTacToeGame::computerSymbol = TicTacToeGame::X;
  }

  TicTacToeGame::tft.fillScreen(TicTacToeGame::backgroundColor);
  TicTacToeGame::ticTacToeDrawn = false;
}

} // namespace TicTacToeGame
