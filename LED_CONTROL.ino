// ============================================================
// Traffic Light + Streetlight Fault Simulation - Arduino Mega
//
// CIRCUIT (no transistors needed):
//   Arduino PWM pin -> 220 ohm resistor -> LED Anode
//   LED Cathode -> GND (shared ground rail near pin 53)
//
// Two phases repeat forever:
//   WORKING phase (30 seconds): both traffic lights cycle
//     Red -> Green -> Yellow -> Red together, and both
//     streetlights are at full brightness.
//   FAULTY phase (30 seconds): Traffic Light 1's red LED
//     flickers, Traffic Light 2's green LED is stuck dim,
//     and both streetlights dim down as well.
//
// Every phase change and colour change is logged to the Serial
// Monitor with a timestamp -- this is the "ground truth" record
// used later to check whether the AI correctly detected each
// condition.
// ============================================================

// ---- Pin assignments (matches your latest wiring) ----
int SL2        = 2;
int SL1        = 3;
int TL1_RED    = 4;
int TL1_YELLOW = 5;
int TL1_GREEN  = 6;
int TL2_RED    = 7;
int TL2_YELLOW = 8;
int TL2_GREEN  = 9;

// ---- Brightness levels (PWM values range from 0 to 255) ----
int FULL = 255; // Fully lit
int DIM  = 77;   // About 30% brightness - used for dim faults
int OFF  = 0;     // Fully off

// ---- How long each phase lasts, in milliseconds ----
unsigned long WORKING_DURATION = 30000; // 30 seconds
unsigned long FAULTY_DURATION  = 30000; // 30 seconds

// ---- How long each colour stays on during the WORKING phase ----
unsigned long RED_DURATION    = 5000; // 5 seconds
unsigned long GREEN_DURATION  = 5000; // 5 seconds
unsigned long YELLOW_DURATION = 2000; // 2 seconds

// ---- How fast Traffic Light 1's red LED flickers during a fault ----
unsigned long FLICKER_INTERVAL = 200; // toggles on/off every 200ms

// ---- Variables that keep track of "what time it is" for each timer ----
bool inFaultyPhase = false;       // false = Working phase, true = Faulty phase
unsigned long phaseStartTime = 0;  // when the current phase began

int currentColor = 0;              // 0 = Red, 1 = Green, 2 = Yellow (Working phase only)
unsigned long colorStartTime = 0;  // when the current colour started showing

bool flickerOn = false;            // current on/off state of TL1's flickering red LED
unsigned long lastFlickerTime = 0; // when the flicker last toggled

void setup() {
  Serial.begin(9600); // Start the Serial Monitor so we can log everything

  // Tell the Arduino all eight LED pins will send OUTPUT signals
  pinMode(TL1_RED, OUTPUT);
  pinMode(TL1_YELLOW, OUTPUT);
  pinMode(TL1_GREEN, OUTPUT);
  pinMode(TL2_RED, OUTPUT);
  pinMode(TL2_YELLOW, OUTPUT);
  pinMode(TL2_GREEN, OUTPUT);
  pinMode(SL1, OUTPUT);
  pinMode(SL2, OUTPUT);

  // Start the very first phase: WORKING, beginning on Red
  phaseStartTime = millis();   // record "now" as the phase start time
  colorStartTime = millis();   // record "now" as the colour start time
  currentColor = 0;             // 0 means Red
  setBothLights(currentColor);  // turn both traffic lights Red
  analogWrite(SL1, FULL);       // streetlights start fully on
  analogWrite(SL2, FULL);
  Serial.println("Entering WORKING phase: Red, Streetlights Full");
}

void loop() {
  unsigned long now = millis(); // "now" - checked once per loop for consistency

  // ---- STEP 1: Check if it's time to switch phase (Working <-> Faulty) ----
  unsigned long currentPhaseDuration = inFaultyPhase ? FAULTY_DURATION : WORKING_DURATION;
  if (now - phaseStartTime >= currentPhaseDuration) {
    inFaultyPhase = !inFaultyPhase; // flip to the other phase
    phaseStartTime = now;            // reset the phase timer

    if (inFaultyPhase) {
      // Entering the FAULTY phase: turn traffic lights off first, then
      // apply the specific faults to everything.
      turnAllOff();
      analogWrite(TL2_GREEN, DIM); // TL2's fault: stuck dim green
      analogWrite(SL1, DIM);        // streetlights dim during a fault
      analogWrite(SL2, DIM);
      flickerOn = true;
      lastFlickerTime = now;
      analogWrite(TL1_RED, FULL);  // TL1's fault starts with red ON
      Serial.println("Entering FAULTY phase: TL1 flickering red, TL2 dim green, Streetlights Dim");
    } else {
      // Entering the WORKING phase: reset back to a normal Red start
      currentColor = 0;
      colorStartTime = now;
      setBothLights(currentColor);
      analogWrite(SL1, FULL);       // streetlights back to full brightness
      analogWrite(SL2, FULL);
      Serial.println("Entering WORKING phase: Red, Streetlights Full");
    }
  }

  // ---- STEP 2: Run whichever phase we're currently in ----
  if (inFaultyPhase) {
    runFaultyPhase(now);
  } else {
    runWorkingPhase(now);
  }
}

// Handles the normal Red -> Green -> Yellow -> Red cycle for both lights
void runWorkingPhase(unsigned long now) {
  unsigned long durationForCurrentColor;

  if (currentColor == 0) durationForCurrentColor = RED_DURATION;
  else if (currentColor == 1) durationForCurrentColor = GREEN_DURATION;
  else durationForCurrentColor = YELLOW_DURATION;

  // Has this colour been showing long enough? If so, move to the next one.
  if (now - colorStartTime >= durationForCurrentColor) {
    currentColor = currentColor + 1;
    if (currentColor > 2) {
      currentColor = 0; // after Yellow, go back to Red
    }
    colorStartTime = now;
    setBothLights(currentColor);

    if (currentColor == 0) Serial.println("Working: Red");
    else if (currentColor == 1) Serial.println("Working: Green");
    else Serial.println("Working: Yellow");
  }
}

// Handles TL1's flicker (TL2's dim and the streetlights need no repeated
// action - they were already set once, right when the Faulty phase began)
void runFaultyPhase(unsigned long now) {
  if (now - lastFlickerTime >= FLICKER_INTERVAL) {
    flickerOn = !flickerOn; // switch true<->false
    analogWrite(TL1_RED, flickerOn ? FULL : OFF);
    lastFlickerTime = now;
  }
}

// Sets BOTH traffic lights to the same colour at once (used during Working)
// colorIndex: 0 = Red, 1 = Green, 2 = Yellow
void setBothLights(int colorIndex) {
  analogWrite(TL1_RED,    (colorIndex == 0) ? FULL : OFF);
  analogWrite(TL1_YELLOW, (colorIndex == 2) ? FULL : OFF);
  analogWrite(TL1_GREEN,  (colorIndex == 1) ? FULL : OFF);

  analogWrite(TL2_RED,    (colorIndex == 0) ? FULL : OFF);
  analogWrite(TL2_YELLOW, (colorIndex == 2) ? FULL : OFF);
  analogWrite(TL2_GREEN,  (colorIndex == 1) ? FULL : OFF);
}

// Turns every traffic light LED off - used right before applying
// the specific faulty-phase conditions
void turnAllOff() {
  analogWrite(TL1_RED, OFF);
  analogWrite(TL1_YELLOW, OFF);
  analogWrite(TL1_GREEN, OFF);
  analogWrite(TL2_RED, OFF);
  analogWrite(TL2_YELLOW, OFF);
  analogWrite(TL2_GREEN, OFF);
}