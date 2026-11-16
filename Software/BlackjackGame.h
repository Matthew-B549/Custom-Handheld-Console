#pragma once
#include <Arduino.h>
#include <string.h>
#include <SD.h>                  // <-- Add SD library
#include "ButtonDefinitions.h"
#include "HighScoreManager.h"    // <-- Add HighScoreManager
#include "SoundManager.h"        // <-- Add SoundManager Integration

namespace BlackjackGame {
const int sdChipSelect = 5;      // <-- Add your SD CS pin (same as Tetris)
TFT_eSPI tft = TFT_eSPI();
bool launcherExit = false;
bool onMenu = true;
bool roundOver = false;
bool quitConfirm = false;
bool needsDraw = true;
bool gameOver = false;           // <-- Tracks when bankroll hits 0
bool endScreenShown = false;     // <-- Prevents rapid SD file system writing

const int maxHand = 12;
int deck[52];
int deckPos = 0;
int playerHand[maxHand];
int dealerHand[maxHand];
int playerCount = 0;
int dealerCount = 0;
bool dealerHoleHidden = true;
char resultText[20] = "";
int resultColor = TFT_WHITE;
unsigned long lastDebounce = 0;
const unsigned long debounceMs = 160;

// Game State Tracking
int bankroll = 1000;
int currentBet = 100;
int prevBankroll = -1;
int prevCurrentBet = -1;

bool shouldExitToLauncher() { return launcherExit; }
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

int cardRank(int card) {
  return card % 13;
}

int cardValue(int card) {
  int r = cardRank(card);
  if (r == 0) return 11;
  if (r >= 9) return 10;
  return r + 1;
}

const char* cardRankText(int card) {
  static const char* names[] = {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
  return names[cardRank(card)];
}
const char* cardSuitText(int card){
  static const char* suits[] = {"H","D","C","S"};
  return suits[(card / 13) % 4];
}

void shuffleDeck(){
  for(int i=0;i<52;i++) deck[i]=i;
  for(int i=51;i>0;i--){
    int j=random(0, i+1);
    int temp=deck[i];
    deck[i]=deck[j];
    deck[j]=temp;
  }
  deckPos=0;
}

int drawCard(){
  if(deckPos >= 52) shuffleDeck();
  return deck[deckPos++];
}

void dealCardTo(int hand[], int &count){
  if(count < maxHand){
    hand[count++] = drawCard();
    // Play a crisp short sound whenever a card slides onto the table
    SoundManager::play(SoundManager::SOUND_MENU_MOVE);
    delay(40); // Brief delay so initial dealing sounds rhythmically distinct
  }
}

int handScore(int hand[], int count){
  int score=0, aces=0;
  for(int i=0;i<count;i++){
    int v = cardValue(hand[i]);
    score += v;
    if(cardRank(hand[i]) == 0) aces++;
  }
  while(score > 21 && aces > 0){
    score -= 10;
    aces--;
  }
  return score;
}

void drawCardGraphics(int card, int x, int y, bool hidden){
  if(hidden){
    tft.fillRect(x, y, 34, 48, TFT_RED);
    tft.drawRect(x, y, 34, 48, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString("B", x+14, y+18, 2);
    return;
  }
  tft.fillRect(x, y, 34, 48, TFT_WHITE);
  tft.drawRect(x, y, 34, 48, TFT_BLACK);
  int suit = (card / 13) % 4;
  uint16_t c = (suit < 2) ? TFT_RED : TFT_BLACK;
  tft.setTextColor(c, TFT_WHITE);
  tft.drawString(cardRankText(card), x+4, y+4, 1);
  tft.drawString(cardSuitText(card), x+4, y+34, 1);
}

void drawHud() {
  tft.fillRect(0, 0, 320, 24, TFT_DARKGREEN);
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
  tft.drawString("BANKROLL: $" + String(bankroll), 8, 4, 2);
  tft.drawString("BET: $" + String(currentBet), 200, 4, 2);
}

void drawMenu() {
  tft.fillScreen(TFT_DARKGREEN);
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
  tft.drawCentreString("BLACKJACK", 160, 40, 4);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.drawString("A: Hit      Enter: Stand", 60, 100, 2);
  tft.drawString("Up/Down: Adjust Bet before round", 40, 130, 2);
  tft.drawString("B: Quit Game", 60, 160, 2);
  tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
  tft.drawCentreString("PRESS START TO PLAY", 160, 200, 2);
}

void drawQuit() {
  tft.fillRect(54, 76, 212, 88, TFT_BLACK);
  tft.drawRect(52, 74, 216, 92, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("QUIT?", 160, 90, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("ENTER quits    B cancels", 76, 126, 2);
}

void resetEntireGame() {
  bankroll = 1000;
  currentBet = 100;
  gameOver = false;
  endScreenShown = false;
  onMenu = false;
  roundOver = false;
  quitConfirm = false;
  needsDraw = true;
  shuffleDeck();
  
  // Clean screen buffer explicitly
  tft.fillScreen(TFT_DARKGREEN);
  
  // Play game start fanfare
  SoundManager::play(SoundManager::SOUND_GAME_START);
  
  // Instantly start first standard round
  playerCount = 0;
  dealerCount = 0;
  dealerHoleHidden = true;
  strcpy(resultText, "");
  dealCardTo(playerHand, playerCount);
  dealCardTo(dealerHand, dealerCount);
  dealCardTo(playerHand, playerCount);
  dealCardTo(dealerHand, dealerCount);
  
  if(handScore(playerHand, playerCount) == 21) {
    int dScore = handScore(dealerHand, dealerCount);
    dealerHoleHidden = false;
    roundOver = true;
    if(dScore == 21) {
      strcpy(resultText, "PUSH (NATURAL)");
      resultColor = TFT_WHITE;
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
    } else {
      strcpy(resultText, "BLACKJACK!");
      resultColor = TFT_GREEN;
      bankroll += (currentBet * 1.5);
      SoundManager::play(SoundManager::SOUND_POWERUP);
    }
  }
}

void drawGameOver() {
  // Fire up SD access and save high bankroll record metrics
  SD.begin(sdChipSelect);
  int rank = HighScoreManager::saveScore("blackjack", bankroll);

  tft.fillRect(66, 56, 188, 128, TFT_BLACK);
  tft.drawRect(64, 54, 192, 132, TFT_RED);
  
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawCentreString("GAME OVER", 160, 68, 2);
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("FINAL CASH: $0", 82, 98, 1);

  // Load overall local history lists
  unsigned long highScores[HighScoreManager::MAX_SCORES];
  int count = HighScoreManager::loadScores("blackjack", highScores);
  if (count > 0) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("BEST BANK: $" + String(highScores[0]), 82, 116, 1);
  }

  // Display achieved rank if valid
  if (rank > 0 && rank <= HighScoreManager::MAX_SCORES) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("RANK:  #" + String(rank) + "!", 82, 134, 1);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("START again", 160, 158, 1);
}

void startRound(){
  if (bankroll <= 0) {
    gameOver = true;
    return;
  }
  if(currentBet > bankroll) currentBet = bankroll;
  playerCount = 0;
  dealerCount = 0;
  dealerHoleHidden = true;
  strcpy(resultText, "");
  shuffleDeck();
  dealCardTo(playerHand, playerCount);
  dealCardTo(dealerHand, dealerCount);
  dealCardTo(playerHand, playerCount);
  dealCardTo(dealerHand, dealerCount);
  
  int pScore = handScore(playerHand, playerCount);
  if(pScore == 21){
    dealerHoleHidden = false;
    roundOver = true;
    int dScore = handScore(dealerHand, dealerCount);
    if(dScore == 21){
      strcpy(resultText, "PUSH (NATURAL)");
      resultColor = TFT_WHITE;
      SoundManager::play(SoundManager::SOUND_MENU_BACK);
    } else {
      strcpy(resultText, "BLACKJACK!");
      resultColor = TFT_GREEN;
      bankroll += (currentBet * 1.5);
      SoundManager::play(SoundManager::SOUND_POWERUP);
    }
  } else {
    roundOver = false;
  }
  needsDraw = true;
}

void finishDealerTurn(){
  dealerHoleHidden = false;
  int pScore = handScore(playerHand, playerCount);
  if(pScore > 21){
    strcpy(resultText, "PLAYER BUSTS!");
    resultColor = TFT_RED;
    bankroll -= currentBet;
    roundOver = true;
    needsDraw = true;
    SoundManager::play(SoundManager::SOUND_PLAYER_HURT);
    return;
  }
  while(handScore(dealerHand, dealerCount) < 17){
    dealCardTo(dealerHand, dealerCount);
  }
  int dScore = handScore(dealerHand, dealerCount);
  if(dScore > 21){
    strcpy(resultText, "DEALER BUSTS!");
    resultColor = TFT_GREEN;
    bankroll += currentBet;
    SoundManager::play(SoundManager::SOUND_COIN);
  } else if(pScore > dScore){
    strcpy(resultText, "YOU WIN!");
    resultColor = TFT_GREEN;
    bankroll += currentBet;
    SoundManager::play(SoundManager::SOUND_COIN);
  } else if(pScore < dScore){
    strcpy(resultText, "DEALER WINS!");
    resultColor = TFT_RED;
    bankroll -= currentBet;
    SoundManager::play(SoundManager::SOUND_PLAYER_DEATH);
  } else {
    strcpy(resultText, "PUSH");
    resultColor = TFT_WHITE;
    SoundManager::play(SoundManager::SOUND_MENU_BACK);
  }
  roundOver = true;
  needsDraw = true;
}

void drawScene(){
  tft.fillScreen(TFT_DARKGREEN);
  drawHud();
  
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.drawString("Dealer Hand:", 20, 32, 2);
  for(int i=0; i<dealerCount; i++){
    bool isHidden = (i == 1 && dealerHoleHidden);
    drawCardGraphics(dealerHand[i], 20 + i*40, 52, isHidden);
  }
  if(!dealerHoleHidden){
    tft.drawString("Score: " + String(handScore(dealerHand, dealerCount)), 20, 104, 1);
  }
  
  tft.drawString("Player Hand:", 20, 124, 20);
  for(int i=0; i<playerCount; i++){
    drawCardGraphics(playerHand[i], 20 + i*40, 144, false);
  }
  tft.drawString("Score: " + String(handScore(playerHand, playerCount)), 20, 196, 1);
  
  if(roundOver){
    tft.setTextColor(resultColor, TFT_DARKGREEN);
    tft.drawCentreString(resultText, 160, 108, 2);
    tft.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
    tft.drawCentreString("START for next round", 160, 218, 2);
  } else {
    tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    tft.drawString("A: Hit   Enter: Stand", 80, 218, 2);
  }
}

void setupGame() {
  launcherExit = false;
  tft.init();
  tft.setRotation(3);
  randomSeed(esp_random());
  for(int i=0; i<buttonCount; i++) pinMode(buttons[i].pin, INPUT_PULLUP);
  onMenu = true;
  gameOver = false;
  endScreenShown = false;
  bankroll = 1000;
  currentBet = 100;
  needsDraw = true;
}

void returnToMenu() {
  launcherExit = true;
  onMenu = true;
  quitConfirm = false;
  roundOver = false;
  gameOver = false;
  endScreenShown = false;
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
      returnToMenu();
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
      startRound();
    }
    updateButtons();
    delay(10);
    return;
  }

  if (gameOver) {
    if (!endScreenShown) {
      drawGameOver();
      endScreenShown = true;
      SoundManager::play(SoundManager::SOUND_GAME_OVER);
    }
    if (tapped(Start) || tapped(Enter)) resetEntireGame();
    updateButtons();
    delay(10);
    return;
  }

  if (!roundOver) {
    if (tapped(A) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      dealCardTo(playerHand, playerCount);
      if (handScore(playerHand, playerCount) > 21) finishDealerTurn();
      needsDraw = true;
    }
    if (tapped(Enter) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      finishDealerTurn();
    }
  } else {
    if ((tapped(Start) || tapped(Enter)) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      startRound();
    }
    if (tapped(Up) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      if(currentBet + 50 <= bankroll) {
        currentBet += 50;
        needsDraw = true;
        SoundManager::play(SoundManager::SOUND_SCORE_TALLY);
      } else {
        SoundManager::play(SoundManager::SOUND_ERROR);
      }
    }
    if (tapped(Down) && millis() - lastDebounce > debounceMs) {
      lastDebounce = millis();
      if(currentBet - 50 >= 50) {
        currentBet -= 50;
        needsDraw = true;
        SoundManager::play(SoundManager::SOUND_SCORE_TALLY);
      } else {
        SoundManager::play(SoundManager::SOUND_ERROR);
      }
    }
  }

  if (needsDraw) {
    drawScene();
    needsDraw = false;
  }

  updateButtons();
  delay(10);
}

}  // namespace BlackjackGame