# Chromablast

An arcade-style LED shooting game built with Arduino and addressable LED strips.

## The Game

Chromablast is a color-matching shooter inspired by Space Invaders. A wave of colored LEDs advances toward you along an LED strip. Armed with colored buttons, you must blast matching colors before they reach your position.

**How to Play:**
1. Watch the colored LED pattern approaching from the far end of the strip
2. Press the button matching the color of the nearest LED
3. Your "shot" travels down the strip and eliminates the matching color
4. Miss or hit the wrong color? You'll hear the failure sound
5. Eliminate all colors before they reach you to win
6. If the pattern reaches your end, game over!

## Hardware

- Arduino (ESP32/Arduino Nano/similar)
- WS2812B addressable LED strip (60+ LEDs recommended)
- 3-4 arcade buttons (one per color)
- Piezo buzzer or small speaker
- Power supply (5V, sized for LED count)

See [DESIGN.md](DESIGN.md) for complete hardware specifications and wiring diagrams.

## Directory Structure

```
chromablast/
├── src/
│   └── Chromablast/
│       └── Chromablast.ino    # Main Arduino sketch
├── hardware/
│   ├── schematic.png          # Wiring diagram
│   └── bom.md                  # Bill of materials
├── docs/
│   └── gameplay.md             # Detailed gameplay mechanics
├── CLAUDE.md                   # AI assistant guidance
├── DESIGN.md                   # Technical design document
└── README.md                   # This file
```

## Getting Started

1. Clone this repository
2. Open `src/Chromablast/Chromablast.ino` in Arduino IDE
3. Install required libraries:
   - FastLED or Adafruit NeoPixel
4. Configure your settings in the sketch (LED count, pin assignments)
5. Upload to your Arduino
6. Wire up the hardware per the schematic
7. Play!

## Configuration

Key settings in the sketch:

```cpp
#define NUM_LEDS 60           // Number of LEDs in your strip
#define LED_PIN 6             // Data pin for LED strip
#define BUTTON_RED_PIN 2      // Red button pin
#define BUTTON_GREEN_PIN 3    // Green button pin
#define BUTTON_BLUE_PIN 4     // Blue button pin
#define BUZZER_PIN 5          // Buzzer pin
```

## License

This project is licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).

You are free to use, modify, and distribute this project, including for commercial purposes, as long as you give credit to Waukesha Makerspace.

**Attribution example:**
> Based on Chromablast by [Waukesha Makerspace](https://waukeshamakers.com)

## Part of Waukesha Makerspace

This project is part of the [Waukesha Makerspace](https://waukeshamakers.com) arcade game collection.
