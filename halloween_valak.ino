// One dual linear actuator, two relays (active-LOW)
// LOW = relay ON, HIGH = relay OFF
//
// Audio: DY-HL50F in trigger mode. Pulling the trigger pin to GND plays a track.
// Wire the board's track-1 trigger pin (IO0 / K1) to audioTriggerPin, and
// connect Arduino GND to the DY board's GND.
//
// Sequence (runs once per power-up):
//   boot -> wait STARTUP_DELAY_MS -> start audio
//        -> wait AUDIO_TO_EXTEND_DELAY_MS -> extend for EXTEND_TIME_MS
//        -> hold for HOLD_TIME_MS -> retract for RETRACT_TIME_MS -> done

#include <Arduino.h>

// ---------------- Configuration: tweak these ----------------

// Pins (8-10 sit together on the same header)
const uint8_t extendRelayPin  = 7;
const uint8_t retractRelayPin = 8;
const uint8_t audioTriggerPin = 9;

// Timing (all in milliseconds)
const uint32_t STARTUP_DELAY_MS         = 1000;  // boot -> audio starts (lets the DY player power up)
const uint32_t AUDIO_TO_EXTEND_DELAY_MS = 15000;  // audio starts -> actuator begins extending
const uint32_t EXTEND_TIME_MS           = 3000;  // how long the extend relay stays on
const uint32_t HOLD_TIME_MS             = 3000;  // pause while fully extended
const uint32_t RETRACT_TIME_MS          = 3000;  // how long the retract relay stays on

// How long to hold the audio trigger low (like a button press)
const uint32_t AUDIO_TRIGGER_PULSE_MS = 200;

// Brief pause with both relays off before switching direction
const uint32_t RELAY_DEADTIME_MS = 100;

// ------------------------------------------------------------

#define RELAY_ON  LOW
#define RELAY_OFF HIGH

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

// "Press" the DY board's trigger: pull the pin to GND, then let go.
// Releasing uses INPUT (not HIGH) so we never push 5V into the
// DY board's 3.3V input; its own pull-up brings the pin back high.
void triggerAudio() {
  digitalWrite(audioTriggerPin, LOW);
  pinMode(audioTriggerPin, OUTPUT);
  delay(AUDIO_TRIGGER_PULSE_MS);
  pinMode(audioTriggerPin, INPUT);
}

void setup() {
  // Safe init for relay pins first, so they're off while everything else starts
  safeInitPin(extendRelayPin);
  safeInitPin(retractRelayPin);

  // Audio trigger starts "released" (not pulled to GND)
  pinMode(audioTriggerPin, INPUT);

  // Startup delay to prevent animation from starting immediately on power-up
  delay(STARTUP_DELAY_MS);

  // Start the track
  triggerAudio();

  // Wait for the right moment in the audio, then animate.
  // The trigger pulse already used AUDIO_TRIGGER_PULSE_MS of this gap.
  delay(AUDIO_TO_EXTEND_DELAY_MS - AUDIO_TRIGGER_PULSE_MS);
  extendFor(EXTEND_TIME_MS);

  delay(HOLD_TIME_MS);

  retractFor(RETRACT_TIME_MS);
}

void loop() {
  // Nothing to do; the sequence runs once per power-up.
  // Press the Arduino's reset button to run it again.
}
