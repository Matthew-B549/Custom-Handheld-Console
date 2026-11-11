#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <esp_system.h>

#include "ButtonDefinitions.h"

#include "startImage.h"
#include "byeImage.h"
#include "hitPaddle.h"
#include "pongImage.h"

#include "XImage.h"
#include "OImage.h"
#include "winScreen.h"
#include "loseScreen.h"
#include "tieScreen.h"

#include "TetrisGame.h"
#include "SnakeGame.h"
#include "PacmanGame.h"
#include "PongGame.h"
#include "TicTacToeGame.h"
#include "FlappyGame.h"
#include "InvadersGame.h"
#include "FroggerGame.h"
#include "AsteroidsGame.h"
#include "BlackjackGame.h"

TFT_eSPI menuTft = TFT_eSPI();

enum LauncherState {
  LauncherMenu,
  RunningTetris,
  RunningSnake,
  RunningPacman,
  RunningPong,
  RunningTicTacToe,
  RunningFlappy,
  RunningInvaders,
  RunningFrogger,
  RunningAsteroids,
  RunningBlackjack
};

LauncherState launcherState = LauncherMenu;
int selectedGame = 0;
bool menuNeedsRedraw = true;
bool poweredOn = true;
bool powerLastPressed = false;
unsigned long lastMenuPressMs = 0;
unsigned long lastPowerPressMs = 0;
const unsigned long menuDebounceMs = 180;
const unsigned long powerDebounceMs = 120;

const char* gameNames[] = {
  "TETRIS",
  "SNAKE",
  "PAC-MAN",
  "PONG",
  "TIC-TAC-TOE",
  "FLAPPY BIRD",
  "SPACE INVADERS",
  "FROGGER",
  "ASTEROIDS",
  "BLACKJACK"
};
const int gameCount = 10;

void handlePowerButton();
void readLauncherButtons();
void updateLauncherButtons();
bool launcherButtonTapped(buttonName button);
void drawLauncherMenu();
void processLauncherMenu();
void startSelectedGame();
void runActiveGame();
void returnToLauncher();

void setup() {
  menuTft.init();
  menuTft.setRotation(1);
  menuTft.fillScreen(TFT_BLACK);

  Serial.begin(9600);
  randomSeed(esp_random());

  pinMode(buttons[0].pin, INPUT);
  for (int i = 1; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  powerLastPressed = digitalRead(buttons[Power].pin);
}

void loop() {
  handlePowerButton();
  if (!poweredOn) {
    delay(20);
    return;
  }

  if (launcherState == LauncherMenu) {
    readLauncherButtons();
    processLauncherMenu();
    updateLauncherButtons();
    delay(10);
    return;
  }

  runActiveGame();
}

void handlePowerButton() {
  bool powerPressed = digitalRead(buttons[Power].pin);
  if (powerPressed && !powerLastPressed && millis() - lastPowerPressMs > powerDebounceMs) {
    lastPowerPressMs = millis();
    poweredOn = !poweredOn;
    if (poweredOn) {
      menuTft.writecommand(0x29);
      if (launcherState == LauncherMenu) {
        menuNeedsRedraw = true;
      }
      for (int i = 0; i < buttonCount; i++) {
        buttons[i].pressed = false;
        buttons[i].lastPressed = false;
      }
      powerLastPressed = false;
    } else {
      menuTft.writecommand(0x28);
    }
  }
  powerLastPressed = powerPressed;
}

void readLauncherButtons() {
  buttons[0].pressed = digitalRead(buttons[0].pin);
  for (int i = 1; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

void updateLauncherButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

bool launcherButtonTapped(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed && millis() - lastMenuPressMs > menuDebounceMs;
}

void drawLauncherMenu() {
  menuTft.setRotation(1);
  menuTft.fillScreen(TFT_BLACK);

  menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
  menuTft.setTextSize(2);
  menuTft.setCursor(76, 18);
  menuTft.print("GAME LAUNCHER");
  menuTft.drawFastHLine(24, 44, 272, TFT_WHITE);

  for (int i = 0; i < gameCount; i++) {
    int y = 50 + i * 16;
    bool selected = i == selectedGame;
    uint16_t boxColor = selected ? TFT_YELLOW : TFT_DARKGREY;
    uint16_t textColor = selected ? TFT_BLACK : TFT_WHITE;

    menuTft.fillRect(46, y, 228, 14, selected ? TFT_YELLOW : TFT_BLACK);
    menuTft.drawRect(46, y, 228, 14, boxColor);
    menuTft.setTextColor(textColor, selected ? TFT_YELLOW : TFT_BLACK);
    menuTft.setTextSize(1);
    menuTft.setCursor(70, y + 4);
    menuTft.print(gameNames[i]);
  }

  menuTft.setTextColor(TFT_WHITE, TFT_BLACK);
  menuTft.setTextSize(1);
  menuTft.setCursor(34, 220);
  menuTft.print("UP/DOWN select   START/ENTER runs   POWER off");
}

void processLauncherMenu() {
  if (menuNeedsRedraw) {
    drawLauncherMenu();
    menuNeedsRedraw = false;
  }

  if (launcherButtonTapped(Up)) {
    lastMenuPressMs = millis();
    selectedGame = (selectedGame + gameCount - 1) % gameCount;
    drawLauncherMenu();
  }

  if (launcherButtonTapped(Down)) {
    lastMenuPressMs = millis();
    selectedGame = (selectedGame + 1) % gameCount;
    drawLauncherMenu();
  }

  if (launcherButtonTapped(Start) || launcherButtonTapped(Enter)) {
    lastMenuPressMs = millis();
    startSelectedGame();
  }
}

void startSelectedGame() {
  switch (selectedGame) {
    case 0:
      TetrisGame::launcherExit = false;
      launcherState = RunningTetris;
      TetrisGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 1:
      SnakeGame::launcherExit = false;
      launcherState = RunningSnake;
      SnakeGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 2:
      PacmanGame::launcherExit = false;
      launcherState = RunningPacman;
      PacmanGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 3:
      PongGame::launcherExit = false;
      launcherState = RunningPong;
      PongGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 4:
      TicTacToeGame::launcherExit = false;
      launcherState = RunningTicTacToe;
      TicTacToeGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 5:
      FlappyGame::launcherExit = false;
      launcherState = RunningFlappy;
      FlappyGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 6:
      InvadersGame::launcherExit = false;
      launcherState = RunningInvaders;
      InvadersGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 7:
      FroggerGame::launcherExit = false;
      launcherState = RunningFrogger;
      FroggerGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    case 9:
      BlackjackGame::launcherExit = false;
      launcherState = RunningBlackjack;
      BlackjackGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
    default:
      AsteroidsGame::launcherExit = false;
      launcherState = RunningAsteroids;
      AsteroidsGame::setupGame();
      for (int i = 0; i < buttonCount; i++) buttons[i].lastPressed = true;
      break;
  }
}

void runActiveGame() {
  switch (launcherState) {
    case RunningTetris:
      TetrisGame::loopGame();
      if (TetrisGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningSnake:
      SnakeGame::loopGame();
      if (SnakeGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningPacman:
      PacmanGame::loopGame();
      if (PacmanGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningPong:
      PongGame::loopGame();
      if (PongGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningTicTacToe:
      TicTacToeGame::loopGame();
      if (TicTacToeGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningFlappy:
      FlappyGame::loopGame();
      if (FlappyGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningInvaders:
      InvadersGame::loopGame();
      if (InvadersGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningFrogger:
      FroggerGame::loopGame();
      if (FroggerGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningAsteroids:
      AsteroidsGame::loopGame();
      if (AsteroidsGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningBlackjack:
      BlackjackGame::loopGame();
      if (BlackjackGame::shouldExitToLauncher()) returnToLauncher();
      break;
    default:
      returnToLauncher();
      break;
  }
}

void returnToLauncher() {
  launcherState = LauncherMenu;
  menuNeedsRedraw = true;
  lastMenuPressMs = millis();
  menuTft.init();
  menuTft.setRotation(1);
  menuTft.fillScreen(TFT_BLACK);

  for (int i = 0; i < buttonCount; i++) {
    buttons[i].pressed = false;
    buttons[i].lastPressed = false;
  }
}
