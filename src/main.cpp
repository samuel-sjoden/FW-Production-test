#include <Arduino.h>

// pin assignments
const int PIN_CHARGE = 26; // CHARGE
const int PIN_DONE = 25; // DONE
const int PIN_CHIP = 32; // CHIP
const int PIN_KICK = 33; // KICK

const int MAX_PULSE_WIDTH = 10000; // Max Kick and Chip Pulse in [us] 
const int BAUD_RATE = 115200;
const unsigned long TIMEOUT = 5000;
const int KICK_CHIP_DELAY = 5000;

enum Mode { KICK, CHIP };
volatile Mode current_mode;

void IRAM_ATTR capsCharged();
void IRAM_ATTR stopPulse();
void sendPulse(const int pulse_length, const String& action);
void chargeCaps();
volatile bool caps_charged = false;
volatile bool charging = false;
volatile bool finished_charing = false;
hw_timer_t *timer = NULL;
SemaphoreHandle_t timer_lock = xSemaphoreCreateBinary();
unsigned long t0;

void setup() {

  pinMode(PIN_CHARGE, OUTPUT);
  pinMode(PIN_DONE, INPUT_PULLUP);
  pinMode(PIN_CHIP, OUTPUT);
  pinMode(PIN_KICK, OUTPUT);

 // attachInterrupt(PIN_DONE, capsCharged , RISING);
  digitalWrite(PIN_KICK, LOW);
  digitalWrite(PIN_CHIP, LOW);
  digitalWrite(PIN_CHARGE, LOW);
  Serial.begin(BAUD_RATE);
  
  // Set timer frequency to 1MHz and attach to stop pulse
  // Assuming clock speed is 80MHz
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &stopPulse, true);
  xSemaphoreGive(timer_lock);
}

void loop() {

  while (Serial.available()){
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.equalsIgnoreCase("charge")){
      Serial.println("Charging the capacitors...");
      t0 = millis();
      chargeCaps();
      charging = true;
    } else if (input.equalsIgnoreCase("kick") || input.equalsIgnoreCase("chip")){
      if (caps_charged){
        int pulse_width = MAX_PULSE_WIDTH + 1;
        String pulse = "";
        Serial.println("Enter your pulse length");
        while (pulse_width > MAX_PULSE_WIDTH || pulse_width < 1) {
          pulse = Serial.readStringUntil('\n');
          pulse.trim();
          pulse_width = pulse.toInt();
        }
        Serial.printf("Sending Pulse of length %d us in %d seconds\n", pulse_width, KICK_CHIP_DELAY/1000);
        delay(KICK_CHIP_DELAY);
        caps_charged = false;
        sendPulse(pulse_width, input);
      } else {
        Serial.println("You need to charge the capacitors first or let them finish charging");
      }
    } else {
      Serial.println("INVALID INPUT");
    }
  }
  if (finished_charing) {
    finished_charing = false;
    Serial.println("Capacitors are charged");
  }
  if (millis() - t0 > TIMEOUT && charging) {
    digitalWrite(PIN_CHARGE, LOW);
    Serial.println("Charging timed out...");
    charging = false;
  }
}

void chargeCaps() {
  digitalWrite(PIN_CHARGE,HIGH);
  // TODO: the referenced code uses when the done pin is low (falling edge). Check the datasheet for this chip
  attachInterrupt(PIN_DONE, capsCharged, FALLING);
}

void IRAM_ATTR capsCharged() {
  digitalWrite(PIN_CHARGE, LOW);
  caps_charged = true;
  charging = false;
  finished_charing = true;
  // Detach to prevent boucing of signal affecting state
  detachInterrupt(PIN_DONE);
}

void IRAM_ATTR stopPulse() {
  if (current_mode == KICK) {
    digitalWrite(PIN_KICK, LOW);
  }
  else if (current_mode == CHIP) {
    digitalWrite(PIN_CHIP, LOW); 
  }
  xSemaphoreGiveFromISR(timer_lock, NULL); 
}


void sendPulse(const int pulse_length, const String& action) {
  if(xSemaphoreTake(timer_lock, portMAX_DELAY)) {
    timerWrite(timer, 0);
    timerAlarmWrite(timer, pulse_length, false); 

    if (action.equalsIgnoreCase("Chip")){
      digitalWrite(PIN_CHIP, HIGH);
      current_mode = CHIP;

    } else if (action.equalsIgnoreCase("kick")){
      digitalWrite(PIN_KICK, HIGH);
      current_mode = KICK;
    }
    // Start the timer to last for the pulse length starting at 0. Do not repeat.
    timerAlarmEnable(timer);
  } else {
    Serial.println("Pulse failed. Existing pulse");
  }
}

