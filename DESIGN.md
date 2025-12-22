# Chromablast - Technical Design

## Overview

Chromablast is an arcade-style LED shooting game where players eliminate advancing colored LEDs by pressing matching colored buttons. Think Space Invaders meets Simon Says on an LED strip.

---

## Game Mechanics

### Core Gameplay

```
Player                                              Enemy Side
  |                                                      |
  v                                                      v
[BTN] [=====o=====o=====o=====o=====o=====] <- Pattern advances
  ^         ^     ^     ^     ^     ^
  |         R     G     B     R     G     <- Colors to eliminate
  |
  Press matching button to shoot
```

1. **Pattern Generation**: Random sequence of colored LEDs spawns at far end
2. **Advancement**: Pattern slowly moves toward player (LED 0)
3. **Shooting**: Player presses colored button to fire matching color
4. **Hit Detection**: Shot eliminates nearest LED of matching color
5. **Miss Penalty**: Wrong color = failure sound (no elimination)
6. **Win Condition**: All colors eliminated
7. **Lose Condition**: Any color reaches player position (LED 0-2)

### Difficulty Progression

| Level | Advance Interval | Pattern Length | Colors |
|-------|------------------|----------------|--------|
| 1 | 500ms | 5 | 3 (RGB) |
| 2 | 400ms | 8 | 3 |
| 3 | 350ms | 10 | 3 |
| 4 | 300ms | 12 | 4 (RGBY) |
| 5+ | 250ms | 15+ | 4 |

---

## Hardware Design

### Components

| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| Microcontroller | ESP32 or Arduino Nano | 1 | ESP32 preferred for future WiFi features |
| LED Strip | WS2812B, 60 LED/m | 1m (60 LEDs) | 5V, individually addressable |
| Arcade Buttons | 30mm illuminated | 4 | Red, Green, Blue, Yellow |
| Buzzer | Piezo 5V | 1 | Or small 8-ohm speaker |
| Power Supply | 5V 3A | 1 | Size for LED count (60mA per LED max) |
| Capacitor | 1000µF 6.3V | 1 | Across power supply |
| Resistor | 470Ω | 1 | On LED data line |
| Enclosure | Custom/3D printed | 1 | Button panel + electronics |

### Wiring Diagram

```
                                    +5V ──────┬─────────────────┐
                                              │                 │
                                         [1000µF]               │
                                              │                 │
                                    GND ──────┴──────┐          │
                                                     │          │
┌─────────────────────────────────────────────────────────────────────┐
│                           ARDUINO/ESP32                             │
│                                                                     │
│  5V ────────────────────────────────────────────────> LED Strip VCC │
│  GND ───────────────────────────────────────────────> LED Strip GND │
│  D6 ──────[470Ω]────────────────────────────────────> LED Strip DIN │
│                                                                     │
│  D2 ──────────────┬─────> Red Button ─────> GND                     │
│  D3 ──────────────┬─────> Green Button ───> GND                     │
│  D4 ──────────────┬─────> Blue Button ────> GND                     │
│  D7 ──────────────┬─────> Yellow Button ──> GND                     │
│                   │       (optional)                                │
│            (INPUT_PULLUP)                                           │
│                                                                     │
│  D5 ──────────────────────> Buzzer+ ──────> GND                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Power Considerations

- Each WS2812B LED draws up to 60mA at full white
- 60 LEDs × 60mA = 3.6A theoretical max
- Game typically uses ~30% brightness: ~1.2A average
- Add 500mA for Arduino and buttons
- **Recommended**: 5V 3A power supply minimum

---

## Software Architecture

### State Machine

```
                    ┌─────────────┐
                    │    IDLE     │ <- Power on / Game over
                    │  (Attract)  │
                    └──────┬──────┘
                           │ Any button press
                           ▼
                    ┌─────────────┐
                    │  COUNTDOWN  │ <- 3, 2, 1, GO!
                    │             │
                    └──────┬──────┘
                           │ Countdown complete
                           ▼
              ┌───────────────────────────┐
              │                           │
              │         PLAYING           │<─────────┐
              │                           │          │
              └─────┬──────────────┬──────┘          │
                    │              │                 │
         Pattern    │              │ All colors     │ Level up
         reaches    │              │ eliminated     │
         player     │              │                │
                    ▼              ▼                │
             ┌───────────┐  ┌───────────┐          │
             │  GAME     │  │   LEVEL   │──────────┘
             │  OVER     │  │ COMPLETE  │
             └─────┬─────┘  └───────────┘
                   │
                   │ Timeout / Button
                   ▼
             ┌───────────┐
             │   IDLE    │
             └───────────┘
```

### Core Data Structures

```cpp
// Game colors
enum GameColor : uint8_t {
  COLOR_NONE = 0,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW
};

// Game state
enum GameState : uint8_t {
  STATE_IDLE,
  STATE_COUNTDOWN,
  STATE_PLAYING,
  STATE_SHOOTING,
  STATE_LEVEL_COMPLETE,
  STATE_GAME_OVER
};

// Pattern element
struct PatternElement {
  GameColor color;
  uint8_t position;     // LED index
  bool active;          // Still in play?
};

// Shot in flight
struct Shot {
  GameColor color;
  float position;       // Current LED position (float for smooth animation)
  bool active;
};

// Game data
struct Game {
  GameState state;
  uint8_t level;
  uint16_t score;
  unsigned long lastAdvance;
  uint16_t advanceInterval;
  PatternElement pattern[MAX_PATTERN_LENGTH];
  uint8_t patternLength;
  Shot currentShot;
};
```

### Main Loop Structure

```cpp
void loop() {
  unsigned long now = millis();

  // Read inputs
  readButtons();

  // Update game state
  switch (game.state) {
    case STATE_IDLE:
      updateIdleAnimation();
      if (anyButtonPressed()) {
        startCountdown();
      }
      break;

    case STATE_COUNTDOWN:
      updateCountdown(now);
      break;

    case STATE_PLAYING:
      // Advance pattern
      if (now - game.lastAdvance >= game.advanceInterval) {
        advancePattern();
        game.lastAdvance = now;
      }

      // Handle shooting
      if (game.currentShot.active) {
        updateShot(now);
      } else if (buttonJustPressed()) {
        fireShot(getPressedColor());
      }

      // Check win/lose
      checkGameEnd();
      break;

    case STATE_SHOOTING:
      updateShot(now);
      break;

    case STATE_LEVEL_COMPLETE:
      playVictoryAnimation();
      break;

    case STATE_GAME_OVER:
      playGameOverAnimation();
      break;
  }

  // Render LEDs
  renderLEDs();
  FastLED.show();
}
```

### Key Functions

```cpp
// Fire a shot of given color
void fireShot(GameColor color) {
  game.currentShot.color = color;
  game.currentShot.position = 0;
  game.currentShot.active = true;
  playShootSound();
}

// Update shot position and check for hits
void updateShot(unsigned long now) {
  static unsigned long lastUpdate = 0;

  if (now - lastUpdate < SHOT_SPEED_MS) return;
  lastUpdate = now;

  game.currentShot.position += 1.0;

  // Check for collision with pattern
  for (int i = 0; i < game.patternLength; i++) {
    if (!game.pattern[i].active) continue;

    if ((uint8_t)game.currentShot.position >= game.pattern[i].position) {
      if (game.currentShot.color == game.pattern[i].color) {
        // Hit!
        game.pattern[i].active = false;
        game.score += 10;
        playHitSound();
        game.currentShot.active = false;
        return;
      }
    }
  }

  // Shot went past all targets
  if (game.currentShot.position >= NUM_LEDS) {
    playMissSound();
    game.currentShot.active = false;
  }
}

// Move pattern toward player
void advancePattern() {
  for (int i = 0; i < game.patternLength; i++) {
    if (game.pattern[i].active) {
      game.pattern[i].position--;

      // Check if reached player
      if (game.pattern[i].position <= DANGER_ZONE) {
        game.state = STATE_GAME_OVER;
        return;
      }
    }
  }
}
```

---

## Sound Design

### Sound Effects

| Event | Frequency | Duration | Pattern |
|-------|-----------|----------|---------|
| Button press | 800 Hz | 50ms | Single beep |
| Shot fired | 1200 Hz → 400 Hz | 100ms | Descending sweep |
| Hit (success) | 1000 Hz | 50ms × 3 | Triple beep |
| Miss (wrong color) | 200 Hz | 300ms | Low buzz |
| Level complete | Major arpeggio | 500ms | C-E-G-C |
| Game over | Minor arpeggio | 800ms | Descending |
| Countdown | 600 Hz | 100ms | Beep per second |
| GO! | 1000 Hz | 200ms | Higher beep |

### Sound Implementation

```cpp
void playHitSound() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000, 50);
    delay(80);
  }
}

void playMissSound() {
  tone(BUZZER_PIN, 200, 300);
}

void playShotSound() {
  for (int freq = 1200; freq >= 400; freq -= 100) {
    tone(BUZZER_PIN, freq, 10);
    delay(10);
  }
}
```

---

## LED Animations

### Idle/Attract Mode
- Rainbow wave cycling through strip
- Occasionally flash "PRESS START" pattern

### Countdown
- 3: Red LEDs blink
- 2: Yellow LEDs blink
- 1: Green LEDs blink
- GO: White flash, then game starts

### Active Game
- Pattern colors at their positions
- Shot as bright white/color traveling down strip
- Player zone (LED 0-2) pulses gently

### Hit Animation
- Explosion effect: hit LED flashes white, spreads outward briefly

### Level Complete
- Green wave washes across strip
- Score display (if using 7-segment or just celebration)

### Game Over
- Red wave washes across strip
- LEDs fade to black

---

## Configuration Constants

```cpp
// Hardware
#define NUM_LEDS 60
#define LED_PIN 6
#define BUTTON_RED_PIN 2
#define BUTTON_GREEN_PIN 3
#define BUTTON_BLUE_PIN 4
#define BUTTON_YELLOW_PIN 7
#define BUZZER_PIN 5

// Game tuning
#define DANGER_ZONE 3              // LEDs 0-2 are "you lose" zone
#define INITIAL_ADVANCE_MS 500     // Starting speed
#define MIN_ADVANCE_MS 150         // Fastest speed
#define SHOT_SPEED_MS 20           // Shot animation speed
#define MAX_PATTERN_LENGTH 20      // Maximum colors in pattern
#define INITIAL_PATTERN_LENGTH 5   // Starting pattern size
#define BRIGHTNESS 128             // LED brightness (0-255)

// Colors (CRGB values)
#define GAME_RED    CRGB(255, 0, 0)
#define GAME_GREEN  CRGB(0, 255, 0)
#define GAME_BLUE   CRGB(0, 0, 255)
#define GAME_YELLOW CRGB(255, 255, 0)
```

---

## Future Enhancements

### Phase 2
- [ ] High score storage (EEPROM)
- [ ] Multiple game modes (endless, timed, puzzle)
- [ ] Combo scoring (rapid hits = multiplier)
- [ ] Power-ups (slow motion, multi-shot, bomb)

### Phase 3
- [ ] WiFi high score leaderboard (ESP32)
- [ ] Bluetooth speaker support
- [ ] LED matrix display for score
- [ ] Two-player competitive mode

### Phase 4
- [ ] Integration with makerspace member system
- [ ] Usage tracking
- [ ] Tournament mode

---

## Bill of Materials

See `hardware/bom.md` for complete parts list with links and pricing.

---

## References

- [FastLED Library](https://github.com/FastLED/FastLED)
- [WS2812B Timing](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- [Arduino Tone Reference](https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/)
