/**
 * Chromablast - LED Color Shooter Game
 *
 * An arcade-style game where players shoot colored LEDs
 * advancing toward them by pressing matching colored buttons.
 *
 * Hardware:
 * - WS2812B LED strip (60+ LEDs)
 * - 3-4 arcade buttons (one per color)
 * - Piezo buzzer
 *
 * Part of Waukesha Makerspace arcade collection
 * https://github.com/WaukeshaMakerspace/chromablast
 */

#include <FastLED.h>

// ===================
// Configuration
// ===================

// Hardware pins
#define LED_PIN           6
#define BUTTON_RED_PIN    2
#define BUTTON_GREEN_PIN  3
#define BUTTON_BLUE_PIN   4
#define BUTTON_YELLOW_PIN 7
#define BUZZER_PIN        5

// LED configuration
#define NUM_LEDS          60
#define BRIGHTNESS        128
#define LED_TYPE          WS2812B
#define COLOR_ORDER       GRB

// Game tuning
#define DANGER_ZONE           3     // LEDs 0-2 trigger game over
#define INITIAL_ADVANCE_MS    500   // Starting pattern speed
#define MIN_ADVANCE_MS        150   // Fastest pattern speed
#define SHOT_SPEED_MS         20    // Shot animation speed
#define MAX_PATTERN_LENGTH    20    // Maximum colors in pattern
#define INITIAL_PATTERN_LENGTH 5    // Starting pattern size
#define COUNTDOWN_MS          1000  // Time per countdown number

// ===================
// Types
// ===================

enum GameColor : uint8_t {
  COLOR_NONE = 0,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW
};

enum GameState : uint8_t {
  STATE_IDLE,
  STATE_COUNTDOWN,
  STATE_PLAYING,
  STATE_LEVEL_COMPLETE,
  STATE_GAME_OVER
};

struct PatternElement {
  GameColor color;
  uint8_t position;
  bool active;
};

struct Shot {
  GameColor color;
  float position;
  bool active;
};

struct Game {
  GameState state;
  uint8_t level;
  uint16_t score;
  unsigned long lastAdvance;
  unsigned long lastShotUpdate;
  uint16_t advanceInterval;
  PatternElement pattern[MAX_PATTERN_LENGTH];
  uint8_t patternLength;
  Shot currentShot;
  uint8_t countdownValue;
  unsigned long countdownStart;
};

// ===================
// Globals
// ===================

CRGB leds[NUM_LEDS];
Game game;

// Button states (for edge detection)
bool buttonState[4] = {false, false, false, false};
bool lastButtonState[4] = {false, false, false, false};

// Color mappings
const CRGB gameColors[] = {
  CRGB::Black,    // COLOR_NONE
  CRGB::Red,      // COLOR_RED
  CRGB::Green,    // COLOR_GREEN
  CRGB::Blue,     // COLOR_BLUE
  CRGB::Yellow    // COLOR_YELLOW
};

// ===================
// Setup
// ===================

void setup() {
  Serial.begin(115200);
  Serial.println("Chromablast starting...");

  // Initialize LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // Initialize buttons (INPUT_PULLUP = pressed when LOW)
  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_YELLOW_PIN, INPUT_PULLUP);

  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize game state
  resetGame();

  Serial.println("Ready! Press any button to start.");
}

// ===================
// Main Loop
// ===================

void loop() {
  unsigned long now = millis();

  // Read button inputs
  readButtons();

  // Update game based on state
  switch (game.state) {
    case STATE_IDLE:
      updateIdleState(now);
      break;

    case STATE_COUNTDOWN:
      updateCountdownState(now);
      break;

    case STATE_PLAYING:
      updatePlayingState(now);
      break;

    case STATE_LEVEL_COMPLETE:
      updateLevelCompleteState(now);
      break;

    case STATE_GAME_OVER:
      updateGameOverState(now);
      break;
  }

  // Render and display
  renderLEDs();
  FastLED.show();

  // Small delay for stability
  delay(10);
}

// ===================
// State Updates
// ===================

void updateIdleState(unsigned long now) {
  // Attract mode animation
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue + (i * 4), 255, 128);
  }
  hue++;

  // Check for any button press to start
  if (anyButtonJustPressed()) {
    startCountdown();
  }
}

void updateCountdownState(unsigned long now) {
  if (now - game.countdownStart >= COUNTDOWN_MS) {
    game.countdownValue--;
    game.countdownStart = now;

    if (game.countdownValue == 0) {
      // GO!
      playGoSound();
      game.state = STATE_PLAYING;
      game.lastAdvance = now;
    } else {
      playCountdownSound();
    }
  }

  // Countdown visualization
  FastLED.clear();
  CRGB countColor = (game.countdownValue == 3) ? CRGB::Red :
                    (game.countdownValue == 2) ? CRGB::Yellow :
                    CRGB::Green;
  for (int i = 0; i < game.countdownValue * 10; i++) {
    if (i < NUM_LEDS) leds[i] = countColor;
  }
}

void updatePlayingState(unsigned long now) {
  // Advance pattern periodically
  if (now - game.lastAdvance >= game.advanceInterval) {
    advancePattern();
    game.lastAdvance = now;
  }

  // Update shot if active
  if (game.currentShot.active) {
    if (now - game.lastShotUpdate >= SHOT_SPEED_MS) {
      updateShot();
      game.lastShotUpdate = now;
    }
  } else {
    // Check for new shot
    GameColor pressedColor = getJustPressedColor();
    if (pressedColor != COLOR_NONE) {
      fireShot(pressedColor);
      game.lastShotUpdate = now;
    }
  }

  // Check win/lose conditions
  if (checkGameOver()) {
    game.state = STATE_GAME_OVER;
    playGameOverSound();
  } else if (checkLevelComplete()) {
    game.state = STATE_LEVEL_COMPLETE;
    playLevelCompleteSound();
  }
}

void updateLevelCompleteState(unsigned long now) {
  static unsigned long levelCompleteStart = 0;

  if (levelCompleteStart == 0) {
    levelCompleteStart = now;
  }

  // Green celebration animation
  uint8_t progress = (now - levelCompleteStart) / 20;
  FastLED.clear();
  for (int i = 0; i < progress && i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }

  // After animation, advance to next level
  if (now - levelCompleteStart > 2000) {
    levelCompleteStart = 0;
    game.level++;
    game.advanceInterval = max((int)MIN_ADVANCE_MS,
                               (int)INITIAL_ADVANCE_MS - (game.level * 50));
    generatePattern();
    game.state = STATE_PLAYING;
    game.lastAdvance = now;
  }
}

void updateGameOverState(unsigned long now) {
  static unsigned long gameOverStart = 0;

  if (gameOverStart == 0) {
    gameOverStart = now;
  }

  // Red fade animation
  uint8_t brightness = 255 - min(255UL, (now - gameOverStart) / 4);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(brightness, 0, 0);
  }

  // Return to idle after animation
  if (now - gameOverStart > 3000) {
    gameOverStart = 0;
    resetGame();
  }
}

// ===================
// Game Logic
// ===================

void resetGame() {
  game.state = STATE_IDLE;
  game.level = 1;
  game.score = 0;
  game.advanceInterval = INITIAL_ADVANCE_MS;
  game.currentShot.active = false;
  game.patternLength = 0;
}

void startCountdown() {
  game.state = STATE_COUNTDOWN;
  game.countdownValue = 3;
  game.countdownStart = millis();
  generatePattern();
  playCountdownSound();
}

void generatePattern() {
  game.patternLength = INITIAL_PATTERN_LENGTH + (game.level - 1) * 2;
  if (game.patternLength > MAX_PATTERN_LENGTH) {
    game.patternLength = MAX_PATTERN_LENGTH;
  }

  uint8_t numColors = (game.level >= 4) ? 4 : 3;

  for (int i = 0; i < game.patternLength; i++) {
    game.pattern[i].color = (GameColor)(random(1, numColors + 1));
    game.pattern[i].position = NUM_LEDS - 5 - (i * 3); // Spread out
    game.pattern[i].active = true;
  }
}

void advancePattern() {
  for (int i = 0; i < game.patternLength; i++) {
    if (game.pattern[i].active && game.pattern[i].position > 0) {
      game.pattern[i].position--;
    }
  }
}

void fireShot(GameColor color) {
  game.currentShot.color = color;
  game.currentShot.position = 0;
  game.currentShot.active = true;
  playShotSound();
}

void updateShot() {
  game.currentShot.position += 1.0;

  // Check for collision with pattern elements
  for (int i = 0; i < game.patternLength; i++) {
    if (!game.pattern[i].active) continue;

    if ((uint8_t)game.currentShot.position >= game.pattern[i].position) {
      if (game.currentShot.color == game.pattern[i].color) {
        // Hit!
        game.pattern[i].active = false;
        game.score += 10 * game.level;
        game.currentShot.active = false;
        playHitSound();
        Serial.print("Hit! Score: ");
        Serial.println(game.score);
        return;
      }
    }
  }

  // Shot went off the end
  if (game.currentShot.position >= NUM_LEDS) {
    game.currentShot.active = false;
    playMissSound();
    Serial.println("Miss!");
  }
}

bool checkGameOver() {
  for (int i = 0; i < game.patternLength; i++) {
    if (game.pattern[i].active && game.pattern[i].position <= DANGER_ZONE) {
      return true;
    }
  }
  return false;
}

bool checkLevelComplete() {
  for (int i = 0; i < game.patternLength; i++) {
    if (game.pattern[i].active) {
      return false;
    }
  }
  return true;
}

// ===================
// Rendering
// ===================

void renderLEDs() {
  if (game.state != STATE_PLAYING) return;

  FastLED.clear();

  // Render danger zone (subtle pulse)
  uint8_t dangerBrightness = beatsin8(60, 20, 60);
  for (int i = 0; i <= DANGER_ZONE; i++) {
    leds[i] = CRGB(dangerBrightness, 0, 0);
  }

  // Render pattern
  for (int i = 0; i < game.patternLength; i++) {
    if (game.pattern[i].active) {
      uint8_t pos = game.pattern[i].position;
      if (pos < NUM_LEDS) {
        leds[pos] = gameColors[game.pattern[i].color];
      }
    }
  }

  // Render shot
  if (game.currentShot.active) {
    uint8_t shotPos = (uint8_t)game.currentShot.position;
    if (shotPos < NUM_LEDS) {
      // Bright white head with colored trail
      leds[shotPos] = CRGB::White;
      if (shotPos > 0) {
        leds[shotPos - 1] = gameColors[game.currentShot.color];
      }
    }
  }
}

// ===================
// Input Handling
// ===================

void readButtons() {
  lastButtonState[0] = buttonState[0];
  lastButtonState[1] = buttonState[1];
  lastButtonState[2] = buttonState[2];
  lastButtonState[3] = buttonState[3];

  // INPUT_PULLUP means LOW = pressed
  buttonState[0] = !digitalRead(BUTTON_RED_PIN);
  buttonState[1] = !digitalRead(BUTTON_GREEN_PIN);
  buttonState[2] = !digitalRead(BUTTON_BLUE_PIN);
  buttonState[3] = !digitalRead(BUTTON_YELLOW_PIN);
}

bool anyButtonJustPressed() {
  for (int i = 0; i < 4; i++) {
    if (buttonState[i] && !lastButtonState[i]) {
      return true;
    }
  }
  return false;
}

GameColor getJustPressedColor() {
  if (buttonState[0] && !lastButtonState[0]) return COLOR_RED;
  if (buttonState[1] && !lastButtonState[1]) return COLOR_GREEN;
  if (buttonState[2] && !lastButtonState[2]) return COLOR_BLUE;
  if (buttonState[3] && !lastButtonState[3]) return COLOR_YELLOW;
  return COLOR_NONE;
}

// ===================
// Sound Effects
// ===================

void playCountdownSound() {
  tone(BUZZER_PIN, 600, 100);
}

void playGoSound() {
  tone(BUZZER_PIN, 1000, 200);
}

void playShotSound() {
  for (int freq = 1200; freq >= 400; freq -= 100) {
    tone(BUZZER_PIN, freq, 8);
    delay(8);
  }
  noTone(BUZZER_PIN);
}

void playHitSound() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000, 40);
    delay(60);
  }
  noTone(BUZZER_PIN);
}

void playMissSound() {
  tone(BUZZER_PIN, 200, 300);
}

void playLevelCompleteSound() {
  int melody[] = {523, 659, 784, 1047}; // C5, E5, G5, C6
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], 120);
    delay(150);
  }
  noTone(BUZZER_PIN);
}

void playGameOverSound() {
  int melody[] = {392, 349, 330, 262}; // G4, F4, E4, C4
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], 200);
    delay(250);
  }
  noTone(BUZZER_PIN);
}
