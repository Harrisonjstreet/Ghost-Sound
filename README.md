# Ghost Drawer Device

A low-power ATtiny1616-based sound device designed to hide inside a drawer and randomly play a ghost-like sound when the drawer is opened.

The project originally started as a Raspberry Pi Pico prototype and was later migrated to an ATtiny1616 to reduce idle current and improve battery life.

---

## Current Hardware

- ATtiny1616 breakout
- Photoresistor
- 10kΩ resistor
- Passive buzzer
- Momentary push button
- Battery power
- UPDI programmer

---

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

---

## Current Functionality

The device is designed to sit inside a closed drawer and remain in a low-power sleep state until activity is detected.

### Drawer Trigger

The ATtiny1616 wakes approximately every 250 ms and briefly powers the photoresistor circuit to check the current light level.

A significant increase in light is treated as the drawer being opened.

Current light detection settings:

```cpp
const int CHANGE_THRESHOLD = 75;
const int REARM_DROP = 75;
```

After an opening is detected, the device waits for a significant drop in light before re-arming for the next opening.

---

## Random Sound Behavior

Each valid drawer opening has a chance of playing a sound.

Current setting:

```cpp
const int SOUND_CHANCE = 4;
```

This gives the device a **25% chance of making a sound** when a valid drawer opening is detected.

If a sound is selected:

- **49/50 chance** — regular ghost sound
- **1/50 chance** — rare crazy ghost sound

Approximate probability per drawer opening:

- **75%** — no sound
- **24.5%** — regular ghost sound
- **0.5%** — crazy ghost sound

---

## Regular Ghost Sound

The regular ghost sound lasts approximately 12 seconds.

It uses hardware PWM and includes:

- Smooth pitch transitions
- Alternating high and low tones
- 7 Hz vibrato
- Smooth pitch glide

---

## Crazy Ghost Sound

The rare alternate sound is intentionally much more dramatic.

It:

- Starts near the regular ghost frequency
- Continuously rises in pitch
- Accelerates upward near the end
- Uses increasingly intense vibrato
- Abruptly drops to a very low tone after approximately 10 seconds
- Holds the low tone for the final 2 seconds

The crazy ghost sound is intentionally rare so that it acts as a hidden surprise.

---

## Push Button

The push button provides a manual trigger that works independently of the photoresistor.

A normal button press immediately plays the regular ghost sound.

The button also includes a hidden multi-press feature.

Pressing the button several times quickly while the regular ghost sound is playing interrupts the regular sound and triggers the crazy ghost sound.

Current secret press setting:

```cpp
const int SECRET_PRESS_COUNT = 5;
```

This makes it possible to manually demonstrate the rare sound without waiting for the random 1-in-50 selection.

---

## Photoresistor Circuit

The photoresistor and 10kΩ resistor form a voltage divider.

```text
Pin 3
  |
Photoresistor
  |
  +---- Pin 0 / A0
  |
10kΩ
  |
 GND
```

Pin 3 powers the sensor only while a reading is being taken.

This reduces idle current compared with powering the divider continuously from the 3V rail.

---

## Low-Power Operation

The firmware includes several power-saving features:

- ATtiny1616 power-down sleep mode
- RTC wake approximately every 250 ms
- Photoresistor powered only during measurements
- First ADC conversion discarded after sensor power-up
- Sensor immediately powered back off after the reading
- Hardware PWM used for buzzer output

Idle current during testing has been approximately:

```text
0.3 mA
```

---

## Sensor Reading Process

Each light measurement follows this sequence:

```text
Sleep
  ↓
RTC wake
  ↓
Pin 3 HIGH
  ↓
Wait 5 ms
  ↓
Discard first ADC reading
  ↓
Take second ADC reading
  ↓
Pin 3 LOW
  ↓
Return to sleep
```

---

## Programming

The ATtiny1616 is programmed using an Adafruit UPDI Friend.

### UPDI Connections

| UPDI Friend | ATtiny1616 |
|---|---|
| White | UPDI |
| Black | GND |
| Red | VIN |

The project uses the Arduino IDE with megaTinyCore.

Current configuration:

- ATtiny1616
- 10 MHz internal clock
- SerialUPDI
- 57600 baud

---

## Project Goals

The main goals of this project are:

- Long battery life
- Small physical size
- Low idle current
- Reliable drawer-opening detection
- Randomized sound behavior
- Simple standalone operation

The final device is intended to remain hidden inside a drawer for long periods and occasionally surprise whoever opens it.
