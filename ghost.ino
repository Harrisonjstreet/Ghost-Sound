#include <Arduino.h>
#include <math.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// ==================================================
// PIN SETUP
// ==================================================

const uint8_t LIGHT_PIN        = 0;
const uint8_t BUZZER_PIN       = 7;
const uint8_t BUTTON_PIN       = 2;
const uint8_t SENSOR_POWER_PIN = 3;


// ==================================================
// LIGHT CHANGE SETTINGS
// ==================================================

// TEST SETTINGS
const int CHANGE_THRESHOLD = 75;
const int REARM_DROP = 75;


// ==================================================
// RANDOM TRIGGER SETTINGS
// ==================================================

// TEST MODE:
// 1 = 100% chance of sound
const int SOUND_CHANCE = 5;

// If a sound happens:
// 1 in 50 = funny sound
const int FUNNY_SOUND_CHANCE = 50;


// ==================================================
// SECRET BUTTON SETTINGS
// ==================================================

const unsigned long SECRET_WINDOW_MS = 2000;
const unsigned long BUTTON_DEBOUNCE_MS = 80;

const int SECRET_PRESS_COUNT = 5;


// ==================================================
// SOUND SETTINGS
// ==================================================

const uint32_t CPU_HZ = 10000000UL;
const uint16_t PRESCALER = 8;

const float BASE_FREQ = 523.0;
const float LOW_FREQ  = 466.0;

const float NORMAL_DURATION = 12.0;

const float PITCH_INTERVAL = 2.5;

const float VIBRATO_FREQ = 7.0;
const float VIBRATO_DEPTH = 0.03;

const float GLIDE_TAU = 0.3;

const int UPDATE_MS = 5;


// ==================================================
// FORWARD DECLARATION
// ==================================================

void playRisingSound();


// ==================================================
// RTC WAKE INTERRUPT
// ==================================================

ISR(RTC_PIT_vect) {

  RTC.PITINTFLAGS = RTC_PI_bm;
}


// ==================================================
// BUTTON WAKE INTERRUPT
// ==================================================

void buttonWake() {

}


// ==================================================
// SENSOR READING
// ==================================================

int readLightSensor() {

  // Turn sensor divider ON
  digitalWrite(
    SENSOR_POWER_PIN,
    HIGH
  );

  // Give divider time to settle
  delay(5);


  // Throw away first ADC reading
  analogRead(LIGHT_PIN);


  // Use second ADC reading
  int reading =
    analogRead(LIGHT_PIN);


  // Turn sensor divider OFF
  digitalWrite(
    SENSOR_POWER_PIN,
    LOW
  );


  return reading;
}


// ==================================================
// BUZZER FREQUENCY
// ==================================================

void setBuzzerFrequency(float frequency) {

  uint16_t period =
    (uint16_t)(
      (CPU_HZ / (PRESCALER * frequency)) - 1
    );


  TCA0.SINGLE.PERBUF =
    period;


  TCA0.SINGLE.CMP2BUF =
    period / 2;
}


// ==================================================
// BUZZER CONTROL
// ==================================================

void buzzerOn() {

  TCA0.SINGLE.CTRLB |=
    TCA_SINGLE_CMP2EN_bm;
}


void buzzerOff() {

  TCA0.SINGLE.CTRLB &=
    ~TCA_SINGLE_CMP2EN_bm;


  digitalWrite(
    BUZZER_PIN,
    LOW
  );
}


// ==================================================
// REGULAR GHOST SOUND
// ==================================================

void playGhost(bool allowSecret) {

  float pitchFactor =
    1.0;


  const float LOW_PITCH_FACTOR =
    LOW_FREQ / BASE_FREQ;


  const float dt =
    UPDATE_MS / 1000.0;


  const float glideAmount =
    dt / (GLIDE_TAU + dt);


  unsigned long elapsedMs =
    0;


  const unsigned long durationMs =
    (unsigned long)(
      NORMAL_DURATION * 1000.0
    );


  uint8_t tapCount =
    allowSecret ? 1 : 0;


  bool lastButtonPressed =
    digitalRead(BUTTON_PIN) == LOW;


  unsigned long lastTapTime =
    0;


  setBuzzerFrequency(
    BASE_FREQ
  );


  buzzerOn();


  while (
    elapsedMs < durationMs
  ) {

    float t =
      elapsedMs / 1000.0;


    // ==================================================
    // SECRET MULTI-TAP CHECK
    // ==================================================

    if (
      allowSecret &&
      elapsedMs <= SECRET_WINDOW_MS
    ) {

      bool buttonPressed =
        digitalRead(BUTTON_PIN) == LOW;


      if (
        buttonPressed &&
        !lastButtonPressed
      ) {

        if (
          elapsedMs - lastTapTime >=
          BUTTON_DEBOUNCE_MS
        ) {

          tapCount++;


          lastTapTime =
            elapsedMs;


          if (
            tapCount >=
            SECRET_PRESS_COUNT
          ) {

            buzzerOff();


            delay(30);


            playRisingSound();


            return;
          }
        }
      }


      lastButtonPressed =
        buttonPressed;
    }


    // ==================================================
    // NORMAL GHOST SOUND
    // ==================================================

    float cyclePosition =
      fmod(
        t,
        2.0 * PITCH_INTERVAL
      );


    float targetPitch;


    if (
      cyclePosition <
      PITCH_INTERVAL
    ) {

      targetPitch =
        1.0;
    }

    else {

      targetPitch =
        LOW_PITCH_FACTOR;
    }


    pitchFactor +=
      (
        targetPitch -
        pitchFactor
      )
      * glideAmount;


    float vibrato =
      sin(
        2.0 *
        PI *
        VIBRATO_FREQ *
        t
      );


    float frequency =
      BASE_FREQ *
      pitchFactor *
      (
        1.0 +
        VIBRATO_DEPTH *
        vibrato
      );


    setBuzzerFrequency(
      frequency
    );


    delay(
      UPDATE_MS
    );


    elapsedMs +=
      UPDATE_MS;
  }


  buzzerOff();
}


// ==================================================
// CURSED / FUNNY SOUND
// ==================================================

void playRisingSound() {

  const float FUNNY_DURATION =
    12.0;


  unsigned long elapsedMs =
    0;


  const unsigned long durationMs =
    (unsigned long)(
      FUNNY_DURATION * 1000.0
    );


  setBuzzerFrequency(
    BASE_FREQ
  );


  buzzerOn();


  while (
    elapsedMs < durationMs
  ) {

    float t =
      elapsedMs / 1000.0;


    // ==================================================
    // FIRST 10 SECONDS
    // ==================================================

    if (
      t < 10.0
    ) {

      float pitchProgress;


      // 0–8 seconds
      if (
        t < 8.0
      ) {

        pitchProgress =
          (t / 8.0) * 0.55;
      }


      // 8–10 seconds
      else {

        float x =
          (t - 8.0) / 2.0;


        pitchProgress =
          0.55 +
          0.25 *
          pow(
            x,
            1.7
          );
      }


      float risingFrequency =
        BASE_FREQ *
        pow(
          4.0,
          pitchProgress
        );


      float currentVibratoDepth =
        VIBRATO_DEPTH;


      // Vibrato gets stronger after 4 seconds
      if (
        t >= 4.0
      ) {

        float x =
          (t - 4.0) / 6.0;


        if (
          x > 1.0
        ) {

          x =
            1.0;
        }


        currentVibratoDepth =
          VIBRATO_DEPTH +
          (
            0.15 *
            pow(
              x,
              1.3
            )
          );
      }


      float vibrato =
        sin(
          2.0 *
          PI *
          VIBRATO_FREQ *
          t
        );


      float frequency =
        risingFrequency *
        (
          1.0 +
          currentVibratoDepth *
          vibrato
        );


      setBuzzerFrequency(
        frequency
      );
    }


    // ==================================================
    // FINAL 2 SECONDS
    // ==================================================

    else {

      float lowFrequency =
        BASE_FREQ / 4.0;


      float vibrato =
        sin(
          2.0 *
          PI *
          VIBRATO_FREQ *
          t
        );


      float frequency =
        lowFrequency *
        (
          1.0 +
          0.06 *
          vibrato
        );


      setBuzzerFrequency(
        frequency
      );
    }


    delay(
      UPDATE_MS
    );


    elapsedMs +=
      UPDATE_MS;
  }


  buzzerOff();
}


// ==================================================
// RANDOM DRAWER SOUND
// ==================================================

void attemptRandomSound() {

  // TEST MODE:
  // SOUND_CHANCE = 1 means 100%

  if (
    random(SOUND_CHANCE) != 0
  ) {

    return;
  }


  if (
    random(FUNNY_SOUND_CHANCE) == 0
  ) {

    playRisingSound();
  }

  else {

    playGhost(false);
  }
}


// ==================================================
// RTC SLEEP TIMER
// ==================================================

void setupSleepTimer() {

  RTC.CLKSEL =
    RTC_CLKSEL_INT32K_gc;


  RTC.PITINTCTRL =
    RTC_PI_bm;


  // Wake every ~250 ms
  RTC.PITCTRLA =
    RTC_PERIOD_CYC8192_gc |
    RTC_PITEN_bm;
}


// ==================================================
// SLEEP
// ==================================================

void goToSleep() {

  set_sleep_mode(
    SLEEP_MODE_PWR_DOWN
  );


  sleep_enable();


  sleep_cpu();


  sleep_disable();
}


// ==================================================
// SETUP
// ==================================================

void setup() {

  pinMode(
    LIGHT_PIN,
    INPUT
  );


  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );


  pinMode(
    SENSOR_POWER_PIN,
    OUTPUT
  );


  // Sensor OFF by default
  digitalWrite(
    SENSOR_POWER_PIN,
    LOW
  );


  attachInterrupt(
    digitalPinToInterrupt(
      BUTTON_PIN
    ),
    buttonWake,
    FALLING
  );


  pinMode(
    BUZZER_PIN,
    OUTPUT
  );


  digitalWrite(
    BUZZER_PIN,
    LOW
  );


  // ==================================================
  // TCA0 HARDWARE PWM
  // ==================================================

  takeOverTCA0();


  TCA0.SINGLE.CTRLA =
    0;


  TCA0.SINGLE.CTRLB =
    TCA_SINGLE_WGMODE_SINGLESLOPE_gc;


  uint16_t initialPeriod =
    (uint16_t)(
      (
        CPU_HZ /
        (
          PRESCALER *
          BASE_FREQ
        )
      )
      - 1
    );


  TCA0.SINGLE.PER =
    initialPeriod;


  TCA0.SINGLE.CMP2 =
    initialPeriod / 2;


  TCA0.SINGLE.CTRLA =
    TCA_SINGLE_CLKSEL_DIV8_gc |
    TCA_SINGLE_ENABLE_bm;


  setupSleepTimer();


  randomSeed(
    readLightSensor() +
    micros()
  );
}


// ==================================================
// MAIN LOOP
// ==================================================

void loop() {

  static bool initialized =
    false;


  static bool lightArmed =
    true;


  static bool buttonWasPressed =
    false;


  static int baseline =
    0;


  static int peakLight =
    0;


  // ==================================================
  // READ LIGHT
  // ==================================================

  int lightValue =
    readLightSensor();


  // ==================================================
  // INITIALIZE BASELINE
  // ==================================================

  if (
    !initialized
  ) {

    baseline =
      lightValue;


    peakLight =
      lightValue;


    initialized =
      true;
  }


  // ==================================================
  // BUTTON
  // ==================================================

  bool buttonPressed =
    digitalRead(BUTTON_PIN) == LOW;


  if (
    buttonPressed &&
    !buttonWasPressed
  ) {

    playGhost(true);
  }


  buttonWasPressed =
    buttonPressed;


  // ==================================================
  // PHOTORESISTOR
  // ==================================================

  if (
    lightArmed
  ) {

    int lightChange =
      lightValue -
      baseline;


    // ==================================================
    // LIGHT INCREASE DETECTED
    // ==================================================

    if (
      lightChange >=
      CHANGE_THRESHOLD
    ) {

      lightArmed =
        false;


      peakLight =
        lightValue;


      // 100% during testing
      attemptRandomSound();
    }


    // ==================================================
    // OTHERWISE UPDATE BASELINE
    // ==================================================

    else {

      baseline =
        (
          baseline * 7L +
          lightValue
        )
        / 8;
    }
  }


  // ==================================================
  // WAIT FOR LIGHT TO DROP
  // ==================================================

  else {

    if (
      lightValue >
      peakLight
    ) {

      peakLight =
        lightValue;
    }


    int lightDrop =
      peakLight -
      lightValue;


    if (
      lightDrop >=
      REARM_DROP
    ) {

      lightArmed =
        true;


      baseline =
        lightValue;


      peakLight =
        lightValue;
    }
  }


  // ==================================================
  // SLEEP
  // ==================================================

  goToSleep();
}
