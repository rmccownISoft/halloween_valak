// One dual linear actuator, two relays (active-LOW)
// LOW = relay ON, HIGH = relay OFF
//
// Sequence (runs once per power-up):
//   boot -> wait STARTUP_DELAY_MS -> start audio
//        -> wait AUDIO_TO_EXTEND_DELAY_MS -> extend for EXTEND_TIME_MS
//        -> hold for HOLD_TIME_MS -> retract for RETRACT_TIME_MS -> done

#include <Arduino.h>
#include "DYPlayerArduino.h"

// ---------------- Configuration: tweak these ----------------

// Relay pins (set these to the pins your relay board is wired to)
const uint8_t extendRelayPin  = 7;
const uint8_t retractRelayPin = 8;

// Audio
const uint8_t  AUDIO_VOLUME = 20;   // 0-30 (15 is about 50%)
const uint16_t AUDIO_TRACK  = 1;    // track number on the DY player's SD card / flash

// Timing (all in milliseconds)
const uint32_t STARTUP_DELAY_MS         = 2000;  // boot -> audio starts (lets the DY player power up)
const uint32_t AUDIO_TO_EXTEND_DELAY_MS = 5000;  // audio starts -> actuator begins extending
const uint32_t EXTEND_TIME_MS           = 3000;  // how long the extend relay stays on
const uint32_t HOLD_TIME_MS             = 2000;  // pause while fully extended
const uint32_t RETRACT_TIME_MS          = 3000;  // how long the retract relay stays on

// Brief pause with both relays off before switching direction
const uint32_t RELAY_DEADTIME_MS = 100;

// ------------------------------------------------------------

#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// Initialize the DYPlayer (uses the hardware Serial port by default)
DY::Player player;

// Set the pin HIGH (relay off) *before* making it an output,
// so the relay never clicks on during boot
void safeInitPin(uint8_t pin) {
  digitalWrite(pin, RELAY_OFF);
  pinMode(pin, OUTPUT);
}

void allRelaysOff() {
  digitalWrite(extendRelayPin, RELAY_OFF);
  digitalWrite(retractRelayPin, RELAY_OFF);
}

// Turn the opposite relay off first so both are never on at the same time
void extendFor(uint32_t ms) {
  allRelaysOff();
  delay(RELAY_DEADTIME_MS);
  digitalWrite(extendRelayPin, RELAY_ON);
  delay(ms);
  allRelaysOff();
}

void retractFor(uint32_t ms) {
  allRelaysOff();
  delay(RELAY_DEADTIME_MS);
  digitalWrite(retractRelayPin, RELAY_ON);
  delay(ms);
  allRelaysOff();
}

void setup() {
  // Safe init for relay pins first, so they're off while everything else starts
  safeInitPin(extendRelayPin);
  safeInitPin(retractRelayPin);

  // Startup delay to prevent animation from starting immediately on power-up
  delay(STARTUP_DELAY_MS);

  // Initialize DYPlayer and start the track
  player.begin();
  player.setVolume(AUDIO_VOLUME);
  player.playSpecified(AUDIO_TRACK);

  // Wait for the right moment in the audio, then animate
  delay(AUDIO_TO_EXTEND_DELAY_MS);
  extendFor(EXTEND_TIME_MS);

  delay(HOLD_TIME_MS);

  retractFor(RETRACT_TIME_MS);
}

void loop() {
  // Nothing to do; the sequence runs once per power-up.
  // Press the Arduino's reset button to run it again.
}
