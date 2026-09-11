# Ghost Drawer Device

A low-power sound device designed to hide inside a drawer and randomly play a ghost-like sound when the drawer is opened.

The project began as a Raspberry Pi Pico prototype and was later migrated to an ATtiny1616 to reduce idle power consumption.

## Current Hardware

- ATtiny1616 breakout
- Photoresistor
- 10kΩ resistor
- Passive buzzer
- Momentary push button
- Battery power
- UPDI programmer

## Current Pinout

| ATtiny Pin | Function |
|---|---|
| Pin 0 / A0 | Photoresistor ADC input |
| Pin 2 | Push button |
| Pin 3 | Photoresistor power |
| Pin 7 | Passive buzzer |
| VIN | Battery input |
| G | Ground |
| UPDI | Programming |

## Current Functionality

The device is designed to sit inside a closed drawer and remain in a low-power sleep state until activity is detected.

### Drawer Trigger

The ATtiny1616 wakes approximately every 250 ms and briefly powers the photoresistor circuit to check the current light level.

A significant increase in light is treated as the drawer being opened.

Current light detection settings:

```cpp
const int CHANGE_THRESHOLD = 75;
const int REARM_DROP = 75;
