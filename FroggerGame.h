#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace FroggerGame {
TFT_eSPI tft=TFT_eSPI();
const int maxX = 319;
const int maxY = 239;
bool launcherExit=false,onMenu=true,gameOver=false,quitConfirm=false,won=false,needsDraw=true;
int frogX=152,frogY=216,score=0,lives=3;
int carX[8],carY[8],carSpeed[8];
int prevFrogX=152,prevFrogY=216,prevCarX[8],prevCarY[8];
bool gamePaused=false;
bool pauseScreenDrawn=false;
bool resumeCountdownActive=false;
int resumeCountdownValue=0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
unsigned long lastFrame=0,lastDebounce=0;
const unsigned long debounceMs=160;

bool shouldExitToLauncher(){return launcherExit;}
void scanButtons(){buttons[0].pressed=digitalRead(buttons[0].pin); for(int i=1;i<buttonCount;i++) buttons[i].pressed=!digitalRead(buttons[i].pin);}
bool tapped(buttonName b){return buttons[b].pressed&&!buttons[b].lastPressed;}
void updateButtons(){for(int i=0;i<buttonCount;i++) buttons[i].lastPressed=buttons[i].pressed;}

void drawMenu(){tft.fillScreen(TFT_DARKGREEN); tft.setTextColor(TFT_YELLOW,TFT_DARKGREEN); tft.setTextSize(3); tft.setCursor(92,32); tft.print("FROGGER"); tft.setTextColor(TFT_WHITE,TFT_DARKGREEN); tft.setTextSize(1); tft.setCursor(64,98); tft.print("Cross the road and river"); tft.setCursor(64,116); tft.print("Arrows hop, B asks to quit"); tft.setTextSize(2); tft.setCursor(76,190); tft.print("START TO PLAY");}
void resetLevel(){frogX=152; frogY=216;}
void resetGame(){gameOver=false;quitConfirm=false;won=false;gamePaused=false;pauseScreenDrawn=false;resumeCountdownActive=false;needsDraw=true;score=0;lives=3;resetLevel(); for(int i=0;i<8;i++){carX[i]=random(0,300); carY[i]=68+(i%4)*32; carSpeed[i]=(i%2==0?3:-4);} lastFrame=millis();}
void drawQuit(){tft.fillRect(54,76,212,88,TFT_BLACK);tft.drawRect(52,74,216,92,TFT_YELLOW);tft.setTextColor(TFT_YELLOW,TFT_BLACK);tft.setTextSize(2);tft.setCursor(104,96);tft.print("QUIT?");tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.setTextSize(1);tft.setCursor(88,132);tft.print("ENTER quits   B cancels");}
void drawPauseOverlay(){tft.fillRect(88,98,144,44,TFT_BLACK);tft.drawRect(86,96,148,48,TFT_WHITE);tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.setTextSize(2);tft.setCursor(104,112);tft.print("PAUSED");pauseScreenDrawn=true;}
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
void fillBand(int rx,int ry,int rw,int rh,int bandTop,int bandBottom,uint16_t color){int top=max(ry,bandTop),bottom=min(ry+rh,bandBottom);if(top<bottom)tft.fillRect(rx,top,rw,bottom-top,color);}
void restoreBackground(int rx,int ry,int rw,int rh){if(rx<0){rw+=rx;rx=0;}if(ry<0){rh+=ry;ry=0;}if(rx+rw>320)rw=320-rx;if(ry+rh>240)rh=240-ry;if(rw<=0||rh<=0)return;fillBand(rx,ry,rw,rh,0,38,TFT_BLUE);fillBand(rx,ry,rw,rh,38,40,TFT_BLACK);fillBand(rx,ry,rw,rh,40,68,TFT_DARKGREEN);fillBand(rx,ry,rw,rh,68,200,TFT_DARKGREY);fillBand(rx,ry,rw,rh,200,204,TFT_BLACK);fillBand(rx,ry,rw,rh,204,240,TFT_DARKGREEN);for(int y=84;y<196;y+=32)if(y>=ry&&y<ry+rh)for(int x=0;x<320;x+=28)tft.drawFastHLine(x,y,14,TFT_WHITE);}
void drawBackground(){tft.fillScreen(TFT_BLACK);tft.fillRect(0,0,320,38,TFT_BLUE);tft.fillRect(0,40,320,28,TFT_DARKGREEN);tft.fillRect(0,68,320,132,TFT_DARKGREY);tft.fillRect(0,204,320,36,TFT_DARKGREEN);for(int y=84;y<196;y+=32)for(int x=0;x<320;x+=28)tft.drawFastHLine(x,y,14,TFT_WHITE);}
void drawHud(){tft.fillRect(0,0,320,20,TFT_BLUE);tft.setTextColor(TFT_WHITE,TFT_BLUE);tft.setTextSize(1);tft.setCursor(8,8);tft.print("SCORE ");tft.print(score);tft.setCursor(246,8);tft.print("LIVES ");tft.print(lives);}
void erasePreviousFrame(){restoreBackground(prevFrogX-10,prevFrogY-12,20,22);for(int i=0;i<8;i++)restoreBackground(prevCarX[i]-2,prevCarY[i]-2,32,20);}
void rememberFrame(){prevFrogX=frogX;prevFrogY=frogY;for(int i=0;i<8;i++){prevCarX[i]=carX[i];prevCarY[i]=carY[i];}}
void drawFrame(){if(needsDraw){drawBackground();needsDraw=false;}else erasePreviousFrame();for(int i=0;i<8;i++)tft.fillRoundRect(carX[i],carY[i],28,16,3,i%2?TFT_RED:TFT_ORANGE);tft.fillCircle(frogX,frogY,8,TFT_GREEN);tft.fillCircle(frogX-4,frogY-5,2,TFT_WHITE);tft.fillCircle(frogX+4,frogY-5,2,TFT_WHITE);drawHud();rememberFrame();}
void drawEnd(){tft.fillRect(68,78,184,86,TFT_BLACK); tft.drawRect(66,76,188,90,won?TFT_GREEN:TFT_RED); tft.setTextColor(won?TFT_GREEN:TFT_RED,TFT_BLACK); tft.setTextSize(2); tft.setCursor(won?112:94,96); tft.print(won?"YOU WIN":"GAME OVER"); tft.setTextColor(TFT_WHITE,TFT_BLACK); tft.setTextSize(1); tft.setCursor(102,130); tft.print("START again");}

void setupGame(){launcherExit=false;tft.init();tft.setRotation(1);randomSeed(esp_random());pinMode(buttons[0].pin,INPUT);for(int i=1;i<buttonCount;i++)pinMode(buttons[i].pin,INPUT_PULLUP);onMenu=true;needsDraw=true;}
void loopGame(){scanButtons(); if(onMenu){if(needsDraw){drawMenu();needsDraw=false;} if((tapped(Start)||tapped(Enter))&&millis()-lastDebounce>debounceMs){lastDebounce=millis();onMenu=false;resetGame();} updateButtons();delay(10);return;} if(quitConfirm){if(tapped(B)){quitConfirm=false;needsDraw=true;} if(tapped(Enter))launcherExit=true; updateButtons();delay(10);return;} if(tapped(B)&&millis()-lastDebounce>debounceMs){lastDebounce=millis();quitConfirm=true;drawQuit();updateButtons();return;} if(gameOver||won){if(tapped(Start)||tapped(Enter))resetGame();updateButtons();delay(10);return;} if(tapped(Start)&&millis()-lastDebounce>debounceMs){lastDebounce=millis();gamePaused=!gamePaused;if(!gamePaused){pauseScreenDrawn=false;startResumeCountdown();}else drawPauseOverlay();} if(resumeCountdownActive){if(updateResumeCountdown()){updateButtons();delay(10);return;}} if(gamePaused){if(!pauseScreenDrawn)drawPauseOverlay();updateButtons();delay(10);return;}
  if(millis()-lastDebounce>debounceMs){ if(tapped(Up)){frogY-=18;lastDebounce=millis();score++;} if(tapped(Down)){frogY+=18;lastDebounce=millis();} if(tapped(Left)){frogX-=18;lastDebounce=millis();} if(tapped(Right)){frogX+=18;lastDebounce=millis();} }
  frogX=constrain(frogX,8,312); frogY=constrain(frogY,20,224); if(frogY<36){won=true;score+=100;}
  if(millis()-lastFrame>32){lastFrame=millis(); for(int i=0;i<8;i++){carX[i]+=carSpeed[i]; if(carX[i]>330)carX[i]=-30; if(carX[i]<-30)carX[i]=330; if(abs(frogX-(carX[i]+14))<18&&abs(frogY-(carY[i]+8))<14){lives--; if(lives<=0)gameOver=true; else resetLevel();}} drawFrame(); if(gameOver||won)drawEnd();}
  updateButtons(); delay(8);
}

} // namespace FroggerGame
