#pragma once
#include <Arduino.h>
#include <FS.h>
#include <SD.h>

namespace HighScoreManager {

const int MAX_SCORES = 10;
const char* SCORES_FILE = "/scores.txt";

// Loads scores for a specific game into the provided array.
// Returns the number of scores loaded.
int loadScores(const char* gameKey, unsigned long* scoresOut) {
  int count = 0;
  String filePath = String("/") + gameKey + ".txt";

  if (SD.exists(filePath)) {
    File file = SD.open(filePath, FILE_READ);
    if (file) {
      while (file.available() && count < MAX_SCORES) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
          scoresOut[count] = line.toInt();
          count++;
        }
      }
      file.close();
    }
  }
  return count;
}

// Saves a new score for a specific game, maintaining a sorted leaderboard.
// Returns the rank (1-based) the new score achieved, or 0 if it didn't make the top 10.
int saveScore(const char* gameKey, unsigned long newScore) {
  unsigned long scores[MAX_SCORES + 1];
  int count = 0;

  // Load existing scores
  count = loadScores(gameKey, scores);

  // Add new score
  scores[count] = newScore;
  count++;

  // Sort descending (highest first) using bubble sort
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (scores[j] < scores[j + 1]) {
        unsigned long temp = scores[j];
        scores[j] = scores[j + 1];
        scores[j + 1] = temp;
      }
    }
  }

  // Cap to MAX_SCORES
  if (count > MAX_SCORES) {
    count = MAX_SCORES;
  }

  // Determine rank (1-based) of the new score
  int rank = 0;
  for (int i = 0; i < count; i++) {
    if (scores[i] == newScore) {
      rank = i + 1;
      break;
    }
  }

  // Write back to SD card
  String filePath = String("/") + gameKey + ".txt";
  File file = SD.open(filePath, FILE_WRITE);
  if (file) {
    for (int i = 0; i < count; i++) {
      file.println(scores[i]);
    }
    file.close();
  }

  return rank;
}

// Resets (deletes) all high scores for a specific game.
void resetScores(const char* gameKey) {
  String filePath = String("/") + gameKey + ".txt";
  if (SD.exists(filePath)) {
    SD.remove(filePath);
  }
}

// Resets all high scores for all games.
void resetAllScores() {
  // List of all game keys
  const char* gameKeys[] = {
    "tetris", "snake", "pacman", "pong", "tictactoe",
    "flappy", "invaders", "frogger", "asteroids", "blackjack",
    "galaga", "cube3d", "minesweeper", "dino"
  };
  int gameCount = 14;

  for (int i = 0; i < gameCount; i++) {
    resetScores(gameKeys[i]);
  }
}

}  // namespace HighScoreManager
