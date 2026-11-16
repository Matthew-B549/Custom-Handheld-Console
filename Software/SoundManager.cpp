#include "SoundManager.h"

namespace SoundManager {

TaskHandle_t SoundTaskHandle = NULL;
volatile int currentFx = SOUND_NONE;

// Background worker thread processing
void soundTaskWorker(void * pvParameters) {
  pinMode(audioPin, OUTPUT);
  
  while(true) {
    if (currentFx != SOUND_NONE) {
      int fx = currentFx;
      currentFx = SOUND_NONE; // Reset instantly to look for next frame trigger

      // ----------------------------------------------------
      // CATEGORY 1: COMBAT & WEAPONS
      // ----------------------------------------------------
      if (fx == SOUND_LASER) {
        for (int freq = 2000; freq > 150; freq -= 40) {
          tone(audioPin, freq); vTaskDelay(pdMS_TO_TICKS(4));
        }
      } 
      else if (fx == SOUND_LASER_HEAVY) {
        for (int freq = 1200; freq > 80; freq -= 20) {
          tone(audioPin, freq); vTaskDelay(pdMS_TO_TICKS(6));
        }
      }
      else if (fx == SOUND_LASER_ENEMY) {
        for (int freq = 600; freq < 1500; freq += 45) {
          tone(audioPin, freq); vTaskDelay(pdMS_TO_TICKS(3));
        }
      }
      else if (fx == SOUND_MISSILE_LAUNCH) {
        for (int freq = 150; freq < 900; freq += 15) {
          tone(audioPin, freq + random(-20, 20)); vTaskDelay(pdMS_TO_TICKS(5));
        }
      }
      else if (fx == SOUND_BOMB_DROP) {
        for (int freq = 1800; freq > 200; freq -= 15) {
          tone(audioPin, freq + random(-15, 15)); vTaskDelay(pdMS_TO_TICKS(4));
        }
      }
      else if (fx == SOUND_EXPLOSION) {
        for (int i = 0; i < 35; i++) {
          tone(audioPin, random(50, 350)); vTaskDelay(pdMS_TO_TICKS(5));
        }
      }

      // ----------------------------------------------------
      // CATEGORY 2: DAMAGE & DEFEAT
      // ----------------------------------------------------
      else if (fx == SOUND_EXPLOSION_BOSS) {
        for (int i = 0; i < 120; i++) {
          tone(audioPin, random(30, 200)); vTaskDelay(pdMS_TO_TICKS(6));
        }
      }
      else if (fx == SOUND_PLAYER_HURT) {
        tone(audioPin, 180); vTaskDelay(pdMS_TO_TICKS(60));
        tone(audioPin, 100); vTaskDelay(pdMS_TO_TICKS(120));
      }
      else if (fx == SOUND_PLAYER_DEATH) {
        for (int freq = 400; freq > 60; freq -= 10) {
          tone(audioPin, freq); vTaskDelay(pdMS_TO_TICKS(15));
        }
      }
      else if (fx == SOUND_SHIELD_HIT) {
        for (int freq = 1500; freq > 900; freq -= 80) {
          tone(audioPin, freq + random(-40, 40)); vTaskDelay(pdMS_TO_TICKS(4));
        }
      }
      else if (fx == SOUND_SHIELD_DOWN) {
        for (int freq = 800; freq > 200; freq -= 30) {
          tone(audioPin, freq); vTaskDelay(pdMS_TO_TICKS(12));
        }
      }

      // ----------------------------------------------------
      // CATEGORY 3: POWERUPS & SCORE
      // ----------------------------------------------------
      else if (fx == SOUND_COIN) {
        tone(audioPin, 988); vTaskDelay(pdMS_TO_TICKS(70));
        tone(audioPin, 1319); vTaskDelay(pdMS_TO_TICKS(200));
      }
      else if (fx == SOUND_POWERUP) {
        int notes[] = {523, 659, 784, 1047}; // C, E, G, C
        for (int n = 0; n < 4; n++) {
          tone(audioPin, notes[n]); vTaskDelay(pdMS_TO_TICKS(80));
        }
      }
      else if (fx == SOUND_POWERDOWN) {
        int notes[] = {1047, 784, 659, 523};
        for (int n = 0; n < 4; n++) {
          tone(audioPin, notes[n]); vTaskDelay(pdMS_TO_TICKS(90));
        }
      }
      else if (fx == SOUND_EXTRA_LIFE) {
        for (int i = 0; i < 3; i++) {
          tone(audioPin, 1568); vTaskDelay(pdMS_TO_TICKS(60));
          tone(audioPin, 1976); vTaskDelay(pdMS_TO_TICKS(60));
        }
      }
      else if (fx == SOUND_SCORE_TALLY) {
        tone(audioPin, 1200); vTaskDelay(pdMS_TO_TICKS(30));
        noTone(audioPin); vTaskDelay(pdMS_TO_TICKS(20));
        tone(audioPin, 1500); vTaskDelay(pdMS_TO_TICKS(30));
      }
// ----------------------------------------------------
      // CATEGORY 4: USER INTERFACE & MENUS
      // ----------------------------------------------------
      else if (fx == SOUND_MENU_MOVE) {
        tone(audioPin, 600); vTaskDelay(pdMS_TO_TICKS(25));
      }
      else if (fx == SOUND_MENU_SELECT) {
        tone(audioPin, 1200); vTaskDelay(pdMS_TO_TICKS(50));
        tone(audioPin, 1800); vTaskDelay(pdMS_TO_TICKS(60));
      }
      else if (fx == SOUND_MENU_BACK) {
        tone(audioPin, 800); vTaskDelay(pdMS_TO_TICKS(40));
        tone(audioPin, 500); vTaskDelay(pdMS_TO_TICKS(70));
      }
      else if (fx == SOUND_ERROR) {
        tone(audioPin, 110); vTaskDelay(pdMS_TO_TICKS(180));
      }
      else if (fx == SOUND_GAME_START) {
        int melody[] = {587, 587, 587, 784, 1175}; // Classic arcade fanfare
        int tempo[]  = {100, 100, 100, 200, 400};
        for (int n = 0; n < 5; n++) {
          tone(audioPin, melody[n]); vTaskDelay(pdMS_TO_TICKS(tempo[n]));
          noTone(audioPin); vTaskDelay(pdMS_TO_TICKS(20));
        }
      }
      else if (fx == SOUND_START_MUSIC) {
        // Retro arcade chirps & low buzzes sequence
        for (int b = 0; b < 2; b++) {
          tone(audioPin, 100); vTaskDelay(pdMS_TO_TICKS(35));
          tone(audioPin, 60);  vTaskDelay(pdMS_TO_TICKS(35));
        }
        noTone(audioPin); vTaskDelay(pdMS_TO_TICKS(30));

        // Upward chirp sweep
        for (int freq = 800; freq < 2400; freq += 160) {
          tone(audioPin, freq); vTaskDelay(pdMS_TO_TICKS(12));
        }

        // Mid buzz accent
        tone(audioPin, 120); vTaskDelay(pdMS_TO_TICKS(45));
        tone(audioPin, 160); vTaskDelay(pdMS_TO_TICKS(45));

        // Down/Up chirp sequence
        int notes[] = {1200, 1600, 2000, 2400, 1800, 2800};
        for (int n = 0; n < 6; n++) {
          tone(audioPin, notes[n]); vTaskDelay(pdMS_TO_TICKS(35));
          if (n % 2 == 1) {
            tone(audioPin, 90); vTaskDelay(pdMS_TO_TICKS(15));
          }
        }

        // Rapid chirp burst end
        for (int freq = 2200; freq > 1000; freq -= 200) {
          tone(audioPin, freq + random(-40, 40)); vTaskDelay(pdMS_TO_TICKS(10));
        }
      }
      else if (fx == SOUND_CONFIRM_YES) {
        // Upbeat ascending confirmation chime
        tone(audioPin, 1047); vTaskDelay(pdMS_TO_TICKS(60));
        tone(audioPin, 1568); vTaskDelay(pdMS_TO_TICKS(100));
      }
      else if (fx == SOUND_CONFIRM_NO) {
        // Descending cancellation tone
        tone(audioPin, 880); vTaskDelay(pdMS_TO_TICKS(60));
        tone(audioPin, 587); vTaskDelay(pdMS_TO_TICKS(100));
      }
      
      // ----------------------------------------------------
      // CATEGORY 5: LEVEL EVENTS & AMBIENT
      // ----------------------------------------------------
      else if (fx == SOUND_LEVEL_COMPLETE) {
        int fanfare[] = {523, 659, 784, 1047, 784, 1047};
        for (int n = 0; n < 6; n++) {
          tone(audioPin, fanfare[n]); vTaskDelay(pdMS_TO_TICKS(110));
        }
      }
      else if (fx == SOUND_GAME_OVER) {
        int sadMelody[] = {392, 370, 349, 294}; // G, F#, F, D down
        for (int n = 0; n < 4; n++) {
          tone(audioPin, sadMelody[n]); vTaskDelay(pdMS_TO_TICKS(220));
        }
      }
      else if (fx == SOUND_WARNING_ALARM) {
        for (int w = 0; w < 2; w++) {
          tone(audioPin, 880); vTaskDelay(pdMS_TO_TICKS(100));
          tone(audioPin, 440); vTaskDelay(pdMS_TO_TICKS(100));
        }
      }
      else if (fx == SOUND_TELEPORT) {
        for (int f = 200; f < 2500; f += 80) {
          tone(audioPin, f); vTaskDelay(pdMS_TO_TICKS(2));
          tone(audioPin, f / 2); vTaskDelay(pdMS_TO_TICKS(2));
        }
      }

      // Cut audio cleanly after any effect concludes
      noTone(audioPin);
    }
    
    // Prevent task starving CPU when idle
    vTaskDelay(pdMS_TO_TICKS(10)); 
  }
}

void begin() {
  currentFx = SOUND_NONE;
  
  xTaskCreatePinnedToCore(
    soundTaskWorker,   
    "SoundTask",       
    3072, // Stack size increased slightly to accommodate extensive arrays safety             
    NULL,               
    1,                 
    &SoundTaskHandle,  
    0                  
  );
}

void play(SoundID fx) {
  currentFx = fx;
}

} // namespace SoundManager