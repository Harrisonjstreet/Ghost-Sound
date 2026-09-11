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

## Light Detection

The photoresistor divider is powered only during ADC measurements to reduce idle current.

The ATtiny wakes approximately every 250 ms, powers the sensor, takes a reading, turns the sensor back off, and returns to sleep.

A drawer opening is detected when the light level increases by a specified amount.

Current settings:

```cpp
const int CHANGE_THRESHOLD = 75;
const int REARM_DROP = 75;
const int SOUND_CHANCE = 4;
