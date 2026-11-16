#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <esp_system.h>
#include <math.h>

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
#include "GalagaGame.h"
#include "Cube3D.h"
#include "MinesweeperGame.h"
#include "DinoGame.h"
#include "DrMarioGame.h" 
#include "HighScoreManager.h"
#include "SoundManager.h"
#include "OTAManager.h"

TFT_eSPI menuTft = TFT_eSPI();
TFT_eSprite animSprite = TFT_eSprite(&menuTft); // Sprite buffer

const int sdChipSelect = 32;

enum LauncherState {
  StartScreen,
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
  RunningBlackjack,
  RunningGalaga,
  RunningCube3D,
  RunningMinesweeper,
  RunningDino,
  RunningDrMario, 
  HighScoresMenu,
  ResetScoresConfirm,
  ExitToStartConfirm,
  OTAUpdateMenu
};

LauncherState launcherState = StartScreen;
int selectedGame = 0;
int menuScrollOffset = 0;
const int menuVisibleItems = 10;

bool menuNeedsRedraw = true;
bool startScreenNeedsRedraw = true;
bool highScoresNeedsRedraw = true;

unsigned long lastMenuPressMs = 0;
const unsigned long menuDebounceMs = 180;

const unsigned long menuRepeatDelay = 300;  
const unsigned long menuRepeatRate = 100;
unsigned long lastMenuRepeatMs = 0;

bool menuRepeatActive = false;
int menuRepeatDirection = 0;
int prevSelectedGame = -1;
int prevMenuScrollOffset = -1;

float wavePhase = 0.0; // Phase angle for clockwise continuous wave
bool spriteCreated = false;

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
  "BLACKJACK",
  "GALAGA",
  "3D CUBE",
  "MINESWEEPER",
  "DINO RUN",
  "DR. MARIO",
  "HIGH SCORES",
  "CHECK FOR UPDATES"
};

const int gameCount = 17; 

void readLauncherButtons();
void updateLauncherButtons();
bool launcherButtonTapped(buttonName button);
void getPerimeterWavePoint(float s, float phase, int &outX, int &outY);
void drawContinuousSineWaveBox();
void processStartScreen();
void drawLauncherMenu();
void processLauncherMenu();
void startSelectedGame();
void runActiveGame();
void returnToLauncher();
void processHighScoresMenu();
void processResetScoresConfirm();
void processExitToStartConfirm();
void clearButtonStates();

void setup() {
  menuTft.init();
  menuTft.setRotation(3);
  menuTft.fillScreen(TFT_BLACK);

  Serial.begin(9600);
  randomSeed(esp_random());

  animSprite.setColorDepth(8);
  if (animSprite.createSprite(320, 240) != NULL) {
    spriteCreated = true;
    Serial.println("eSprite created successfully (76.8 KB RAM).");
  } else {
    spriteCreated = false;
    Serial.println("Failed to allocate eSprite RAM!");
  }

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  SPI.begin(14, 12, 13, sdChipSelect);
  
  if (!SD.begin(sdChipSelect)) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialized.");
  }

  SoundManager::begin();
  clearButtonStates();
}

void loop() {
  if (launcherState == StartScreen) {
    readLauncherButtons();
    processStartScreen();
    updateLauncherButtons();
    delay(15);
    return;
  }

  if (launcherState == LauncherMenu) {
    readLauncherButtons();
    processLauncherMenu();
    updateLauncherButtons();
    delay(10);
    return;
  }

  if (launcherState == HighScoresMenu) {
    readLauncherButtons();
    processHighScoresMenu();
    updateLauncherButtons();
    delay(10);
    return;
  }

  if (launcherState == ResetScoresConfirm) {
    readLauncherButtons();
    processResetScoresConfirm();
    updateLauncherButtons();
    delay(10);
    return;
  }

  if (launcherState == ExitToStartConfirm) {
    readLauncherButtons();
    processExitToStartConfirm();
    updateLauncherButtons();
    delay(10);
    return;
  }

  if (launcherState == OTAUpdateMenu) {
    readLauncherButtons();
    OTAManager::loopOTA();
    updateLauncherButtons();

    if (OTAManager::shouldExitToLauncher()) {
      returnToLauncher();
    }
    delay(10);
    return;
  }

  runActiveGame();
}

void clearButtonStates() {
  for (int i = 0; i < buttonCount; i++) {
    bool current = !digitalRead(buttons[i].pin);
    buttons[i].pressed = current;
    buttons[i].lastPressed = true;
  }
}

void readLauncherButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].pressed = !digitalRead(buttons[i].pin);
  }
}

void updateLauncherButtons() {
  for (int i = 0; i < buttonCount; i++) {
    buttons[i].lastPressed = buttons[i].pressed;
  }
}

bool launcherButtonTapped(buttonName button) {
  return buttons[button].pressed && !buttons[button].lastPressed && (millis() - lastMenuPressMs > menuDebounceMs);
}

void getPerimeterWavePoint(float s, float phase, int &outX, int &outY) {
  const int minX = 20, maxX = 300;
  const int minY = 20, maxY = 220;
  const int W = maxX - minX; // 280
  const int H = maxY - minY; // 200
  const int P = 2 * W + 2 * H; // 960

  while (s >= P) s -= P;
  while (s < 0)  s += P;

  const float k = (2.0f * M_PI * 15.0f) / (float)P;
  const float amplitude = 7.0f;
  float disp = amplitude * sin(k * s - phase);

  // Smooth out corner transitions to keep the wave connected continuous perimeter-wide
  float edgeLen = 0;
  float edgePos = 0;

  if (s < W) { 
    edgeLen = W;
    edgePos = s;
  } else if (s < W + H) { 
    edgeLen = H;
    edgePos = s - W;
  } else if (s < W + H + W) { 
    edgeLen = W;
    edgePos = s - (W + H);
  } else { 
    edgeLen = H;
    edgePos = s - (W + H + W);
  }

  // Taper displacement within 15 pixels of any corner so line ends line up smoothly
  const float taperDist = 15.0f;
  float taper = 1.0f;
  if (edgePos < taperDist) {
    taper = edgePos / taperDist;
  } else if ((edgeLen - edgePos) < taperDist) {
    taper = (edgeLen - edgePos) / taperDist;
  }
  disp *= taper;

  // Calculate coordinates with roundf for smooth rendering
  if (s < W) { 
    outX = minX + (int)roundf(s);
    outY = minY - (int)roundf(disp);
  } else if (s < W + H) { 
    float s2 = s - W;
    outX = maxX + (int)roundf(disp);
    outY = minY + (int)roundf(s2);
  } else if (s < W + H + W) { 
    float s3 = s - (W + H);
    outX = maxX - (int)roundf(s3);
    outY = maxY + (int)roundf(disp);
  } else { 
    float s4 = s - (W + H + W);
    outX = minX - (int)roundf(disp);
    outY = maxY - (int)roundf(s4);
  }
}

void drawContinuousSineWaveBox() {
  const int W = 280;
  const int H = 200;
  const int P = 2 * W + 2 * H; 

  wavePhase += 0.15f;
  if (wavePhase > 6.28318f) wavePhase -= 6.28318f;

  int x0, y0, x1, y1;
  getPerimeterWavePoint(0, wavePhase, x0, y0);

  // Step size reduced to 1 for high precision, smooth line connections
  const int step = 1; 

  for (int s = step; s <= P; s += step) {
    getPerimeterWavePoint(s, wavePhase, x1, y1);

    // Single unified color: RED
    uint16_t color = TFT_RED; 

    if (spriteCreated) {
      animSprite.drawLine(x0, y0, x1, y1, color);
    } else {
      menuTft.drawLine(x0, y0, x1, y1, color);
    }
    x0 = x1;
    y0 = y1;
  }
}

// Helper function to draw a recursive green triangle fractal (Sierpinski pattern)
void drawTriangleFractal(int cx, int cy, int w, int h, int depth) {
  if (depth <= 0 || w < 8 || h < 8) return;

  // Alternate green shades per recursion level for a layered effect
  uint16_t color = (depth % 2 == 0) ? TFT_GREEN : TFT_DARKGREEN;

  int x0 = cx;          int y0 = cy - h / 2; // Top vertex
  int x1 = cx - w / 2;  int y1 = cy + h / 2; // Bottom-left vertex
  int x2 = cx + w / 2;  int y2 = cy + h / 2; // Bottom-right vertex

  // Render triangle outline
  if (spriteCreated) {
    animSprite.drawTriangle(x0, y0, x1, y1, x2, y2, color);
  } else {
    menuTft.drawTriangle(x0, y0, x1, y1, x2, y2, color);
  }

  // Recurse into 3 sub-triangles (Top, Bottom-Left, Bottom-Right)
  int nw = w / 2;
  int nh = h / 2;
  drawTriangleFractal(cx,         cy - h / 4, nw, nh, depth - 1);
  drawTriangleFractal(cx - w / 4, cy + h / 4, nw, nh, depth - 1);
  drawTriangleFractal(cx + w / 4, cy + h / 4, nw, nh, depth - 1);
}

void processStartScreen() {
  if (spriteCreated) {
    animSprite.fillSprite(TFT_BLACK);
    drawTriangleFractal(160, 120, 220, 180, 4);
    animSprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    animSprite.setTextSize(2);
    animSprite.setCursor(100, 95);
    animSprite.print("WELCOME TO");

    animSprite.setTextColor(TFT_MAGENTA, TFT_BLACK);
    animSprite.setTextSize(3);
    animSprite.setCursor(79, 121);
    animSprite.print("GREATNESS");

    animSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    animSprite.setTextSize(1);
    animSprite.setCursor(103, 190);
    animSprite.print("PRESS START TO PLAY");

    drawContinuousSineWaveBox();
    animSprite.pushSprite(0, 0);
  } else {
    menuTft.fillScreen(TFT_BLACK);
    drawTriangleFractal(160, 120, 220, 180, 4);
    drawContinuousSineWaveBox();
  }

  // Accepts either Start or Enter buttons to run launcher
  if (launcherButtonTapped(Start) || launcherButtonTapped(Enter)) {
    lastMenuPressMs = millis();
    launcherState = LauncherMenu;
    menuNeedsRedraw = true;
    clearButtonStates();
    
    // SOUND ADDED: Arcade Fanfare / Game Start Effect
    SoundManager::play(SoundManager::SOUND_GAME_START);
  }
}

void drawMenuItem(int index, bool selected) {
  int startY = 50;
  int itemH = 14;
  int gap = 2;
  int totalItemH = itemH + gap;
  int y = startY + index * totalItemH;
  uint16_t boxColor = selected ? TFT_YELLOW : TFT_DARKGREY;
  uint16_t textColor = selected ? TFT_BLACK : TFT_WHITE;
  int idx = menuScrollOffset + index;

  menuTft.fillRect(46, y, 228, itemH, selected ? TFT_YELLOW : TFT_BLACK);
  menuTft.drawRect(46, y, 228, itemH, boxColor);
  menuTft.setTextColor(textColor, selected ? TFT_YELLOW : TFT_BLACK);
  menuTft.setTextSize(1);
  menuTft.setCursor(70, y + 4);
  menuTft.print(gameNames[idx]);
}

void drawScrollIndicators() {
  bool showUp = menuScrollOffset > 0;
  bool showDown = menuScrollOffset + menuVisibleItems < gameCount;

  menuTft.fillRect(276, 48, 20, 170, TFT_BLACK);
  if (showUp) {
    menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(280, 60);
    menuTft.print("^");
  }
  if (showDown) {
    menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(280, 190);
    menuTft.print("v");
  }
}

void drawLauncherMenu() {
  menuTft.setRotation(3);
  menuTft.fillScreen(TFT_BLACK);
  menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
  menuTft.setTextSize(2);
  menuTft.setCursor(76, 18);
  menuTft.print("GAME LAUNCHER");
  menuTft.drawFastHLine(24, 44, 272, TFT_WHITE);

  for (int i = 0; i < menuVisibleItems && (menuScrollOffset + i) < gameCount; i++) {
    drawMenuItem(i, (menuScrollOffset + i) == selectedGame);
  }

  drawScrollIndicators();

  menuTft.setTextColor(TFT_WHITE, TFT_BLACK);
  menuTft.setTextSize(1);
  menuTft.setCursor(18, 220);
  menuTft.print("UP/DN:Sel  START:Run  B:Back to Title");

  prevSelectedGame = selectedGame;
  prevMenuScrollOffset = menuScrollOffset;
}

void moveSelection(int direction) {
  int oldSelected = selectedGame;
  int oldScrollOffset = menuScrollOffset;
  selectedGame = (selectedGame + direction + gameCount) % gameCount;

  SoundManager::play(SoundManager::SOUND_MENU_MOVE);

  if (selectedGame < menuScrollOffset) {
    menuScrollOffset = selectedGame;
  } else if (selectedGame >= menuScrollOffset + menuVisibleItems) {
    menuScrollOffset = selectedGame - menuVisibleItems + 1;
  }

  if (menuScrollOffset != oldScrollOffset) {
    for (int i = 0; i < menuVisibleItems && (menuScrollOffset + i) < gameCount; i++) {
      drawMenuItem(i, (menuScrollOffset + i) == selectedGame);
    }
    drawScrollIndicators();
  } else {
    int oldIndex = oldSelected - menuScrollOffset;
    if (oldIndex >= 0 && oldIndex < menuVisibleItems) {
      drawMenuItem(oldIndex, false);
    }
    int newIndex = selectedGame - menuScrollOffset;
    if (newIndex >= 0 && newIndex < menuVisibleItems) {
      drawMenuItem(newIndex, true);
    }
  }
}

void processLauncherMenu() {
  if (menuNeedsRedraw) {
    drawLauncherMenu();
    menuNeedsRedraw = false;
  }

  unsigned long now = millis();
  bool upHeld = buttons[Up].pressed;
  bool downHeld = buttons[Down].pressed;
  if (upHeld || downHeld) {
    int dir = upHeld ? -1 : 1;
    bool justTapped = (upHeld && !buttons[Up].lastPressed) || (downHeld && !buttons[Down].lastPressed);
    if (justTapped && (now - lastMenuPressMs > menuDebounceMs)) {
      lastMenuPressMs = now;
      lastMenuRepeatMs = now;
      menuRepeatActive = false;
      menuRepeatDirection = dir;
      moveSelection(dir);
    } else if (dir == menuRepeatDirection && !justTapped) {
      if (!menuRepeatActive) {
        if (now - lastMenuPressMs >= menuRepeatDelay) {
          menuRepeatActive = true;
          lastMenuRepeatMs = now;
          moveSelection(dir);
        }
      } else {
        if (now - lastMenuRepeatMs >= menuRepeatRate) {
          lastMenuRepeatMs = now;
          moveSelection(dir);
        }
      }
    } else if (!justTapped) {
      menuRepeatActive = false;
      menuRepeatDirection = dir;
      lastMenuPressMs = now;
      lastMenuRepeatMs = now;
    }
  } else {
    menuRepeatActive = false;
    menuRepeatDirection = 0;
  }

  // SOUND ADDED: Play select sound when launching a game
  if (launcherButtonTapped(Start) || launcherButtonTapped(Enter)) {
    lastMenuPressMs = millis();
    SoundManager::play(SoundManager::SOUND_MENU_SELECT);
    startSelectedGame();
  }

  // SOUND ADDED: Play back sound when pressing B
  if (launcherButtonTapped(B)) {
    lastMenuPressMs = millis();
    launcherState = ExitToStartConfirm;
    highScoresNeedsRedraw = true;
    clearButtonStates();
    SoundManager::play(SoundManager::SOUND_MENU_BACK);
  }
}

void processExitToStartConfirm() {
  if (highScoresNeedsRedraw) {
    menuTft.setRotation(3);
    menuTft.fillScreen(TFT_BLACK);
    int boxW = 240;
    int boxH = 100;
    int boxX = (320 - boxW) / 2;
    int boxY = (240 - boxH) / 2;
    menuTft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    menuTft.drawRect(boxX, boxY, boxW, boxH, TFT_CYAN);
    menuTft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

    menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(boxX + 20, boxY + 16);
    menuTft.print("EXIT TO TITLE?");

    menuTft.setTextColor(TFT_WHITE, TFT_BLACK);
    menuTft.setTextSize(1);
    menuTft.setCursor(boxX + 28, boxY + 48);
    menuTft.print("Return to title screen?");

    menuTft.setTextColor(TFT_YELLOW, TFT_BLACK);
    menuTft.setCursor(boxX + 28, boxY + 68);
    menuTft.print("ENTER: yes   B: no");
    highScoresNeedsRedraw = false;
  }

  unsigned long now = millis();
  if (launcherButtonTapped(Enter) || launcherButtonTapped(Start)) {
    lastMenuPressMs = now;
    launcherState = StartScreen;
    startScreenNeedsRedraw = true;
    clearButtonStates();
    SoundManager::play(SoundManager::SOUND_MENU_SELECT); // SOUND ADDED
  }

  if (launcherButtonTapped(B)) {
    lastMenuPressMs = now;
    launcherState = LauncherMenu;
    menuNeedsRedraw = true;
    clearButtonStates();
    SoundManager::play(SoundManager::SOUND_MENU_BACK); // SOUND ADDED
  }
}

void startSelectedGame() {
  switch (selectedGame) {
    case 0:
      TetrisGame::launcherExit = false;
      launcherState = RunningTetris;
      TetrisGame::setupGame();
      clearButtonStates();
      break;
    case 1:
      SnakeGame::launcherExit = false;
      launcherState = RunningSnake;
      SnakeGame::setupGame();
      clearButtonStates();
      break;
    case 2:
      PacmanGame::launcherExit = false;
      launcherState = RunningPacman;
      PacmanGame::setupGame();
      clearButtonStates();
      break;
    case 3:
      PongGame::launcherExit = false;
      launcherState = RunningPong;
      PongGame::setupGame();
      clearButtonStates();
      break;
    case 4:
      TicTacToeGame::launcherExit = false;
      launcherState = RunningTicTacToe;
      TicTacToeGame::setupGame();
      clearButtonStates();
      break;
    case 5:
      FlappyGame::launcherExit = false;
      launcherState = RunningFlappy;
      FlappyGame::setupGame();
      clearButtonStates();
      break;
    case 6:
      InvadersGame::launcherExit = false;
      launcherState = RunningInvaders;
      InvadersGame::setupGame();
      clearButtonStates();
      break;
    case 7:
      FroggerGame::launcherExit = false;
      launcherState = RunningFrogger;
      FroggerGame::setupGame();
      clearButtonStates();
      break;
    case 8:
      AsteroidsGame::launcherExit = false;
      launcherState = RunningAsteroids;
      AsteroidsGame::setupGame();
      clearButtonStates();
      break;
    case 9:
      BlackjackGame::launcherExit = false;
      launcherState = RunningBlackjack;
      BlackjackGame::setupGame();
      clearButtonStates();
      break;
    case 10:
      GalagaGame::launcherExit = false;
      launcherState = RunningGalaga;
      GalagaGame::setupGame();
      clearButtonStates();
      break;
    case 11:
      Cube3D::launcherExit = false;
      launcherState = RunningCube3D;
      Cube3D::setupGame();
      clearButtonStates();
      break;
    case 12:
      MinesweeperGame::launcherExit = false;
      launcherState = RunningMinesweeper;
      MinesweeperGame::setupGame();
      clearButtonStates();
      break;
    case 13:
      DinoGame::launcherExit = false;
      launcherState = RunningDino;
      DinoGame::setupGame();
      clearButtonStates();
      break;
    case 14: 
      DrMarioGame::launcherExit = false;
      launcherState = RunningDrMario;
      DrMarioGame::setupGame();
      clearButtonStates();
      break;
    case 15:
      launcherState = HighScoresMenu;
      highScoresNeedsRedraw = true;
      clearButtonStates();
      break;
    case 16:
      launcherState = OTAUpdateMenu;
      OTAManager::setupOTA(); // Init wifi, keyboard ui
      clearButtonStates();
    default:
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
    case RunningGalaga:
      GalagaGame::loopGame();
      if (GalagaGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningCube3D:
      Cube3D::loopGame();
      if (Cube3D::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningMinesweeper:
      MinesweeperGame::loopGame();
      if (MinesweeperGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningDino:
      DinoGame::loopGame();
      if (DinoGame::shouldExitToLauncher()) returnToLauncher();
      break;
    case RunningDrMario: 
      DrMarioGame::loopGame();
      if (DrMarioGame::shouldExitToLauncher()) returnToLauncher();
      break;
    default:
      returnToLauncher();
      break;
  }
}

int highScoresScrollOffset = 0;
const int highScoresVisibleItems = 8;

const char* highScoreGameKeys[] = {
  "tetris", "snake", "pacman", "pong", "tictactoe",
  "flappy", "invaders", "frogger", "asteroids", "blackjack",
  "galaga", "cube3d", "minesweeper", "dino", "drmario"
};
const char* highScoreGameNames[] = {
  "TETRIS", "SNAKE", "PAC-MAN", "PONG", "TIC-TAC-TOE",
  "FLAPPY BIRD", "SPACE INVADERS", "FROGGER", "ASTEROIDS", "BLACKJACK",
  "GALAGA", "3D CUBE", "MINESWEEPER", "DINO RUN", "DR. MARIO" 
};
const int highScoreGameCount = 15; 

void drawHighScoresMenu() {
  menuTft.setRotation(3);
  menuTft.fillScreen(TFT_BLACK);

  menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
  menuTft.setTextSize(2);
  menuTft.setCursor(64, 10);
  menuTft.print("HIGH SCORES");
  menuTft.drawFastHLine(24, 34, 272, TFT_WHITE);

  menuTft.setTextColor(TFT_YELLOW, TFT_BLACK);
  menuTft.setTextSize(1);
  menuTft.setCursor(30, 42);
  menuTft.print("GAME");
  menuTft.setCursor(200, 42);
  menuTft.print("BEST");

  unsigned long bestScores[highScoreGameCount];
  bool hasScore[highScoreGameCount];
  for (int i = 0; i < highScoreGameCount; i++) {
    unsigned long scores[HighScoreManager::MAX_SCORES];
    int count = HighScoreManager::loadScores(highScoreGameKeys[i], scores);
    if (count > 0) {
      bestScores[i] = scores[0];
      hasScore[i] = true;
    } else {
      hasScore[i] = false;
    }
  }

  int startY = 56;
  int rowH = 14;
  int gap = 2;
  int totalRowH = rowH + gap;
  for (int i = 0; i < highScoresVisibleItems && (highScoresScrollOffset + i) < highScoreGameCount; i++) {
    int idx = highScoresScrollOffset + i;
    int y = startY + i * totalRowH;

    menuTft.setTextColor(TFT_WHITE, TFT_BLACK);
    menuTft.setCursor(30, y + 2);
    menuTft.print(highScoreGameNames[idx]);

    menuTft.setCursor(200, y + 2);
    if (hasScore[idx]) {
      menuTft.setTextColor(TFT_GREEN, TFT_BLACK);
      menuTft.print(bestScores[idx]);
    } else {
      menuTft.setTextColor(TFT_DARKGREY, TFT_BLACK);
      menuTft.print("---");
    }
  }

  bool showUp = highScoresScrollOffset > 0;
  bool showDown = highScoresScrollOffset + highScoresVisibleItems < highScoreGameCount;
  menuTft.fillRect(276, 48, 20, 170, TFT_BLACK);
  if (showUp) {
    menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(280, 60);
    menuTft.print("^");
  }
  if (showDown) {
    menuTft.setTextColor(TFT_CYAN, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(280, 190);
    menuTft.print("v");
  }

  menuTft.setTextColor(TFT_WHITE, TFT_BLACK);
  menuTft.setTextSize(1);
  menuTft.setCursor(34, 220);
  menuTft.print("UP/DOWN scroll   START: reset all   B: back");

  highScoresNeedsRedraw = false;
}

void processHighScoresMenu() {
  if (highScoresNeedsRedraw) {
    drawHighScoresMenu();
  }

  unsigned long now = millis();
  if (launcherButtonTapped(Up) && highScoresScrollOffset > 0) {
    lastMenuPressMs = now;
    highScoresScrollOffset--;
    highScoresNeedsRedraw = true;
    SoundManager::play(SoundManager::SOUND_MENU_MOVE);
  }

  if (launcherButtonTapped(Down) && highScoresScrollOffset + highScoresVisibleItems < highScoreGameCount) {
    lastMenuPressMs = now;
    highScoresScrollOffset++;
    highScoresNeedsRedraw = true;
    SoundManager::play(SoundManager::SOUND_MENU_MOVE);
  }

  if (launcherButtonTapped(B)) {
    lastMenuPressMs = now;
    SoundManager::play(SoundManager::SOUND_MENU_BACK); // SOUND ADDED
    returnToLauncher();
  }

  if (launcherButtonTapped(Start) || launcherButtonTapped(Enter)) {
    lastMenuPressMs = now;
    launcherState = ResetScoresConfirm;
    highScoresNeedsRedraw = true;
    SoundManager::play(SoundManager::SOUND_MENU_SELECT); // SOUND ADDED
  }
}

void processResetScoresConfirm() {
  if (highScoresNeedsRedraw) {
    menuTft.setRotation(3);
    menuTft.fillScreen(TFT_BLACK);
    int boxW = 240;
    int boxH = 100;
    int boxX = (320 - boxW) / 2;
    int boxY = (240 - boxH) / 2;
    menuTft.fillRect(boxX, boxY, boxW, boxH, TFT_BLACK);
    menuTft.drawRect(boxX, boxY, boxW, boxH, TFT_RED);
    menuTft.drawRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, TFT_WHITE);

    menuTft.setTextColor(TFT_RED, TFT_BLACK);
    menuTft.setTextSize(2);
    menuTft.setCursor(boxX + 20, boxY + 16);
    menuTft.print("RESET ALL");

    menuTft.setTextColor(TFT_WHITE, TFT_BLACK);
    menuTft.setTextSize(1);
    menuTft.setCursor(boxX + 28, boxY + 48);
    menuTft.print("Delete all high scores?");

    menuTft.setTextColor(TFT_YELLOW, TFT_BLACK);
    menuTft.setCursor(boxX + 28, boxY + 68);
    menuTft.print("ENTER: yes   B: no");
    highScoresNeedsRedraw = false;
  }

  unsigned long now = millis();
  if (launcherButtonTapped(Enter) || launcherButtonTapped(Start)) {
    lastMenuPressMs = now;
    HighScoreManager::resetAllScores();
    highScoresNeedsRedraw = true;
    launcherState = HighScoresMenu;
    SoundManager::play(SoundManager::SOUND_ERROR); // SOUND ADDED (Alert sound for wipe)
  }

  if (launcherButtonTapped(B)) {
    lastMenuPressMs = now;
    highScoresNeedsRedraw = true;
    launcherState = HighScoresMenu;
    SoundManager::play(SoundManager::SOUND_MENU_BACK); // SOUND ADDED
  }
}

void returnToLauncher() {
  launcherState = LauncherMenu;
  menuNeedsRedraw = true;
  highScoresNeedsRedraw = true;
  highScoresScrollOffset = 0;
  lastMenuPressMs = millis();

  delay(10);
  menuTft.init();
  menuTft.setRotation(3);
  clearButtonStates();
}