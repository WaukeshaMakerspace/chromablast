# Bill of Materials - Chromablast

## Required Components

| Component | Description | Qty | Est. Cost | Source |
|-----------|-------------|-----|-----------|--------|
| ESP32 DevKit | ESP32-WROOM-32 dev board | 1 | $8 | Amazon/AliExpress |
| WS2812B Strip | 60 LED/m, 1 meter, IP30 | 1 | $12 | Amazon/AliExpress |
| Arcade Buttons | 30mm LED illuminated, momentary | 4 | $10 | Amazon/Arcade shops |
| Piezo Buzzer | 5V passive buzzer | 1 | $1 | Amazon |
| Power Supply | 5V 3A DC adapter | 1 | $8 | Amazon |
| DC Barrel Jack | 5.5mm x 2.1mm panel mount | 1 | $2 | Amazon |
| Capacitor | 1000µF 6.3V electrolytic | 1 | $1 | Amazon |
| Resistor | 470Ω 1/4W | 1 | $0.10 | Amazon |
| Hookup Wire | 22 AWG solid core, various colors | 1 | $8 | Amazon |
| Enclosure | Project box or 3D printed | 1 | $5-15 | Various |

**Estimated Total: $55-65**

## Optional Upgrades

| Component | Description | Qty | Est. Cost | Notes |
|-----------|-------------|-----|-----------|-------|
| Longer LED Strip | 2m (120 LEDs) | 1 | $20 | More play area |
| 5V 5A Power Supply | Higher capacity | 1 | $12 | For longer strips |
| Speaker | 8Ω 0.5W small speaker | 1 | $3 | Better sound than piezo |
| PAM8403 Amp | Mini audio amplifier | 1 | $2 | For speaker |
| 7-Segment Display | 4-digit LED display | 1 | $5 | Score display |

## Tools Required

- Soldering iron and solder
- Wire strippers
- Screwdriver set
- Hot glue gun (optional, for mounting)
- Multimeter (recommended)

## Wiring Notes

1. **LED Strip Power**: Run power directly from supply to strip, not through Arduino
2. **Common Ground**: All grounds must be connected (Arduino, LEDs, buttons, buzzer)
3. **Data Resistor**: 470Ω resistor between Arduino pin and LED data input reduces noise
4. **Capacitor**: Place across power supply terminals to prevent power spikes

## Suggested Vendors

- **Amazon**: Fast shipping, slightly higher prices
- **AliExpress**: Lower prices, 2-4 week shipping
- **Adafruit**: Quality components, good documentation
- **SparkFun**: Quality components, good tutorials
- **LCSC/JLCPCB**: Bulk components, PCB fabrication
