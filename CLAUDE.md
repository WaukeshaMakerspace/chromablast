# CLAUDE.md

This file provides guidance to Claude Code when working with this repository.

## Project Overview

Chromablast is an Arduino-based arcade game using addressable LEDs and physical buttons. It's a color-matching shooter where players eliminate advancing LED colors by pressing corresponding colored buttons.

## Technology Stack

- **Platform**: Arduino (ESP32, Arduino Nano, or similar)
- **Language**: C++ (Arduino framework)
- **LED Library**: FastLED (preferred) or Adafruit NeoPixel
- **IDE**: Arduino IDE or PlatformIO

## Directory Structure

```
src/Chromablast/     # Main Arduino sketch (folder name must match .ino file)
hardware/            # Schematics, wiring diagrams, BOM
docs/                # Additional documentation
```

## Key Concepts

### Game Loop
1. Update LED pattern position (advance toward player)
2. Check button inputs
3. If button pressed, animate "shot" traveling down strip
4. Check for collision with matching color
5. Update score/game state
6. Check win/lose conditions
7. Update display and play sounds

### LED Addressing
- LED 0 is the "player" end (where buttons are)
- Higher LED indices are "enemy" territory
- Pattern advances from high indices toward 0

### Color Mapping
```cpp
enum GameColor {
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW  // Optional 4th color
};
```

## Hardware Pins (Default)

| Component | Pin | Notes |
|-----------|-----|-------|
| LED Data | 6 | WS2812B data line |
| Red Button | 2 | INPUT_PULLUP |
| Green Button | 3 | INPUT_PULLUP |
| Blue Button | 4 | INPUT_PULLUP |
| Yellow Button | 7 | Optional |
| Buzzer | 5 | Piezo or speaker |

## Code Style

- Use `#define` for pin assignments and configuration
- Use `const` for game parameters that might be tuned
- Keep game logic separate from hardware abstraction where practical
- Comment timing-critical sections

## Common Tasks

### Adding a New Color
1. Add to `GameColor` enum
2. Add button pin definition
3. Add to button reading logic
4. Add CRGB color mapping
5. Update pattern generation to include new color

### Adjusting Difficulty
Key variables to tune:
- `ADVANCE_INTERVAL_MS` - How fast pattern moves (lower = harder)
- `SHOT_SPEED_MS` - How fast shots travel
- `PATTERN_LENGTH` - Initial number of colors to eliminate
- `MIN_ADVANCE_INTERVAL` - Fastest the game can get

### Sound Effects
Use tone() for simple sounds:
```cpp
tone(BUZZER_PIN, 1000, 100);  // 1kHz for 100ms
```

## Testing

Without full hardware:
1. Use Serial output to debug game state
2. Serial input can simulate button presses
3. Consider adding a `DEBUG_MODE` flag

## Dependencies

If using PlatformIO, add to `platformio.ini`:
```ini
lib_deps =
    fastled/FastLED@^3.6.0
```

If using Arduino IDE, install via Library Manager:
- FastLED by Daniel Garcia

## Related Documentation

- [FastLED Documentation](https://fastled.io/docs/)
- [WS2812B Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- Architecture docs: `../waukesha-makers-architecture/projects/chromablast.md`
