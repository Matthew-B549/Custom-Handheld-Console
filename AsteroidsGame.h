#pragma once
#include <Arduino.h>
#include "ButtonDefinitions.h"

namespace AsteroidsGame {
TFT_eSPI tft=TFT_eSPI();
const int maxX = 319;
const int maxY = 239;
bool launcherExit=false,onMenu=true,gameOver=false,quitConfirm=false,needsDraw=true;
float shipX=160,shipY=120,shipVX=0,shipVY=0; int dir=0,score=0,lives=3,bulletX=-1,bulletY=-1,bulletVX=0,bulletVY=0;
float rockX[6],rockY[6],rockVX[6],rockVY[6]; bool rockAlive[6];
int prevShipX=160,prevShipY=120,prevDir=0,prevBulletX=-1,prevBulletY=-1,prevScore=-1,prevLives=-1;
float prevRockX[6],prevRockY[6]; bool prevRockAlive[6];
bool gamePaused = false;
bool pauseScreenDrawn = false;
bool resumeCountdownActive = false;
int resumeCountdownValue = 0;
unsigned long resumeCountdownLastMs = 0;
bool resumeCountdownBoxDrawn = false;
unsigned long lastFrame=0,lastDebounce=0; const unsigned long debounceMs=150; const unsigned long frameMs=50;

bool shouldExitToLauncher(){return launcherExit;}
void scanButtons(){buttons[0].pressed=digitalRead(buttons[0].pin);for(int i=1;i<buttonCount;i++)buttons[i].pressed=!digitalRead(buttons[i].pin);}
bool tapped(buttonName b){return buttons[b].pressed&&!buttons[b].lastPressed;}
void updateButtons(){for(int i=0;i<buttonCount;i++)buttons[i].lastPressed=buttons[i].pressed;}
int dxForDir(int d){int v[8]={0,4,5,4,0,-4,-5,-4};return v[d];}
int dyForDir(int d){int v[8]={-5,-4,0,4,5,4,0,-4};return v[d];}
int dxForDir(){return dxForDir(dir);}
int dyForDir(){return dyForDir(dir);}

void fillClipped(int x,int y,int w,int h,uint16_t color){if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>320)w=320-x;if(y+h>240)h=240-y;if(w>0&&h>0)tft.fillRect(x,y,w,h,color);}
void drawHud(){tft.fillRect(0,0,320,20,TFT_BLACK);tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.setTextSize(1);tft.setCursor(8,8);tft.print("SCORE ");tft.print(score);tft.setCursor(246,8);tft.print("LIVES ");tft.print(lives);}
void drawShipAt(int x,int y,int d,uint16_t color){int noseX=x+dxForDir(d)*3,noseY=y+dyForDir(d)*3;tft.drawLine(x-dyForDir(d)*2,y+dxForDir(d)*2,noseX,noseY,color);tft.drawLine(x+dyForDir(d)*2,y-dxForDir(d)*2,noseX,noseY,color);tft.drawLine(x-dyForDir(d)*2,y+dxForDir(d)*2,x+dyForDir(d)*2,y-dxForDir(d)*2,color);tft.drawLine(x,y,noseX,noseY,TFT_RED);}
void erasePreviousFrame(){fillClipped(prevShipX-18,prevShipY-18,36,36,TFT_BLACK);if(prevBulletX>=0)fillClipped(prevBulletX-4,prevBulletY-4,8,8,TFT_BLACK);for(int i=0;i<6;i++)if(prevRockAlive[i])fillClipped((int)prevRockX[i]-12,(int)prevRockY[i]-12,24,24,TFT_BLACK);}
void rememberFrame(){prevShipX=(int)shipX;prevShipY=(int)shipY;prevDir=dir;prevBulletX=bulletX;prevBulletY=bulletY;prevScore=score;prevLives=lives;for(int i=0;i<6;i++){prevRockX[i]=rockX[i];prevRockY[i]=rockY[i];prevRockAlive[i]=rockAlive[i];}}

void drawMenu(){tft.fillScreen(TFT_BLACK);tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.setTextSize(3);tft.setCursor(74,32);tft.print("ASTEROIDS");tft.setTextSize(1);tft.setCursor(58,96);tft.print("LEFT/RIGHT rotate, UP thrust");tft.setCursor(58,114);tft.print("A or ENTER shoots");tft.setCursor(58,132);tft.print("B asks to quit");tft.setTextSize(2);tft.setCursor(76,192);tft.print("START TO PLAY");}
void drawPauseOverlay(); void drawResumeCountdown(); void startResumeCountdown(); bool updateResumeCountdown();
void resetRocks(){for(int i=0;i<6;i++){rockX[i]=random(20,300);rockY[i]=random(30,210);rockVX[i]=random(-20,21)/10.0;rockVY[i]=random(-20,21)/10.0;if(abs(rockVX[i])<0.5)rockVX[i]=1.0;if(abs(rockVY[i])<0.5)rockVY[i]=-1.0;rockAlive[i]=true;}}
void resetGame(){gameOver=false;quitConfirm=false;gamePaused=false;pauseScreenDrawn=false;resumeCountdownActive=false;needsDraw=true;shipX=160;shipY=120;shipVX=0;shipVY=0;dir=0;score=0;lives=3;bulletX=-1;resetRocks();lastFrame=millis();}
void drawQuit(){tft.fillRect(54,76,212,88,TFT_BLACK);tft.drawRect(52,74,216,92,TFT_YELLOW);tft.setTextColor(TFT_YELLOW,TFT_BLACK);tft.setTextSize(2);tft.setCursor(104,96);tft.print("QUIT?");tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.setTextSize(1);tft.setCursor(88,132);tft.print("ENTER quits   B cancels");}
void drawFrame(){if(needsDraw){tft.fillScreen(TFT_BLACK);needsDraw=false;}else erasePreviousFrame();drawHud();drawShipAt((int)shipX,(int)shipY,dir,TFT_CYAN);if(bulletX>=0)tft.fillCircle(bulletX,bulletY,2,TFT_YELLOW);for(int i=0;i<6;i++)if(rockAlive[i])tft.drawCircle((int)rockX[i],(int)rockY[i],10,TFT_DARKGREY);rememberFrame();}
void drawEnd(){tft.fillRect(68,78,184,86,TFT_BLACK);tft.drawRect(66,76,188,90,TFT_RED);tft.setTextColor(TFT_RED,TFT_BLACK);tft.setTextSize(2);tft.setCursor(94,96);tft.print("GAME OVER");tft.setTextColor(TFT_WHITE,TFT_BLACK);tft.setTextSize(1);tft.setCursor(102,130);tft.print("START again");}
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
void setupGame(){launcherExit=false;tft.init();tft.setRotation(1);randomSeed(esp_random());pinMode(buttons[0].pin,INPUT);for(int i=1;i<buttonCount;i++)pinMode(buttons[i].pin,INPUT_PULLUP);onMenu=true;needsDraw=true;}
void loopGame(){scanButtons();if(onMenu){if(needsDraw){drawMenu();needsDraw=false;}if((tapped(Start)||tapped(Enter))&&millis()-lastDebounce>debounceMs){lastDebounce=millis();onMenu=false;resetGame();}updateButtons();delay(10);return;}if(quitConfirm){if(tapped(B)){quitConfirm=false;needsDraw=true;}if(tapped(Enter))launcherExit=true;updateButtons();delay(10);return;}if(tapped(B)&&millis()-lastDebounce>debounceMs){lastDebounce=millis();quitConfirm=true;drawQuit();updateButtons();return;}if(gameOver){if(tapped(Start)||tapped(Enter))resetGame();updateButtons();delay(10);return;}if(tapped(Start)&&millis()-lastDebounce>debounceMs){lastDebounce=millis();gamePaused=!gamePaused;if(!gamePaused){pauseScreenDrawn=false;startResumeCountdown();}else drawPauseOverlay();}if(resumeCountdownActive){if(updateResumeCountdown()){updateButtons();delay(10);return;}}if(gamePaused){if(!pauseScreenDrawn)drawPauseOverlay();updateButtons();delay(10);return;}
  if(tapped(Left))dir=(dir+7)%8;if(tapped(Right))dir=(dir+1)%8;if(buttons[Up].pressed){shipVX+=dxForDir()*0.04;shipVY+=dyForDir()*0.04;}if((tapped(A)||tapped(Enter))&&bulletX<0){bulletX=shipX;bulletY=shipY;bulletVX=dxForDir()*2;bulletVY=dyForDir()*2;}
  if(millis()-lastFrame>frameMs){lastFrame=millis();shipX+=shipVX;shipY+=shipVY;shipVX*=0.99;shipVY*=0.99;if(shipX<0)shipX=319;if(shipX>319)shipX=0;if(shipY<20)shipY=239;if(shipY>239)shipY=20;if(bulletX>=0){bulletX+=bulletVX;bulletY+=bulletVY;if(bulletX<0||bulletX>319||bulletY<0||bulletY>239)bulletX=-1;}bool any=false;for(int i=0;i<6;i++)if(rockAlive[i]){any=true;rockX[i]+=rockVX[i];rockY[i]+=rockVY[i];if(rockX[i]<0)rockX[i]=319;if(rockX[i]>319)rockX[i]=0;if(rockY[i]<20)rockY[i]=239;if(rockY[i]>239)rockY[i]=20;if(bulletX>=0&&abs(bulletX-rockX[i])<12&&abs(bulletY-rockY[i])<12){rockAlive[i]=false;bulletX=-1;score+=20;}if(abs(shipX-rockX[i])<13&&abs(shipY-rockY[i])<13){lives--;shipX=160;shipY=120;shipVX=0;shipVY=0;if(lives<=0)gameOver=true;}}if(!any)resetRocks();drawFrame();if(gameOver)drawEnd();}
  updateButtons();delay(8);
}

} // namespace AsteroidsGame
