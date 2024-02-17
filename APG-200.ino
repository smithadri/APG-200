#include <ResponsiveAnalogRead.h>

//Buttons
const int N_BUTTONS = 1;
byte buttonPin[N_BUTTONS] = { 6 };
int buttonCC[N_BUTTONS] = {130};
int buttonState[N_BUTTONS] = {HIGH};
int buttonPState[N_BUTTONS] = {HIGH}; //previous

//Debounce buttons
unsigned long lastDebounceTime[N_BUTTONS] = {0};
unsigned long debounceTimer[N_BUTTONS] = {0};
int debounceDelay = 4;


//Potentiometers
const int N_POTS = 2;
byte potPin[N_POTS] = {A0, A1};
int potCC[N_POTS] = {23, 17};
int potReading[N_POTS] = {0};
int potState[N_POTS] = {0};
int potPState[N_POTS] = {0}; //previous
int dataState[N_POTS] = {0};
int dataPState[N_POTS] = {0}; //previous

//Smoothing pot
byte potThreshold = 20; 

unsigned long lastPotTime[N_POTS] = {0};
unsigned long potTimer[N_POTS] = {0};
const int POT_TIMEOUT = 300; //in milliseconds

float snapMultiplier = 0.01; // (0.0 - 1.0) - Increase for faster, but less smooth reading
ResponsiveAnalogRead responsivePot[N_POTS] = {};

/* =================================================== */

void setup ()
{
  pinMode(8, OUTPUT);

  //buttons
  for (int i = 0; i < N_BUTTONS; i++) {
      pinMode(buttonPin[i], INPUT);
  }


  //ResponsiveAnalogRead (for pots smooting)
  for (int i = 0; i < N_POTS; i++) {
    responsivePot[i] = ResponsiveAnalogRead(0, true, snapMultiplier);
    responsivePot[i].setAnalogResolution(1023);
  }

  for (int i = 0; i < N_POTS; i++) {
    potRead(i);
    potPState[i] = potState[i];
  }

  Serial.begin (31250, SERIAL_8N1, true); // 9 bit mode
  delay(2500);
  Serial.write9bit(to9Bits(128, 1)); // send initial 128 ping + 0 value
  delay(20);
  Serial.write9bit(to9Bits(0, 0)); // send initial 128 ping + 0 value

}  // end of setup


/* =================================================== */


void loop ()
{
  momentaryButtons();

  potentiometers();
}  // end of loop


/* =================================================== */


void momentaryButtons() {
  for (int i = 0; i < N_BUTTONS; i++) {
    buttonState[i] = digitalRead(buttonPin[i]);
    debounceTimer[i] = millis() - lastDebounceTime[i];

    if (buttonState[i] != buttonPState[i]) {

      lastDebounceTime[i] = millis();
      
      if (debounceTimer[i] > debounceDelay) {
        if (buttonState[i] == HIGH) {
          digitalWrite(8, HIGH);
          delay(20);
          Serial.write9bit(to9Bits(buttonCC[i], 1)); // send initial 128 ping + 0 value
          delay(20);
          Serial.write9bit(to9Bits(255, 0)); // send initial 128 ping + 0 value
          if ( buttonCC[i] == 130) {
            delay(20);
            Serial.write9bit(to9Bits(0, 0)); // send initial 128 ping + 0 value
          }
        } else {
          digitalWrite(8, LOW);
        }
        buttonPState[i] = buttonState[i];
      }

    }

  }
}
void potRead(int i) {
  potReading[i] = analogRead(potPin[i]);
  responsivePot[i].update(potReading[i]);
  potState[i] = responsivePot[i].getValue();
}

void potentiometers() {
  for (int i = 0; i < N_POTS; i++) {
    potRead(i);
    dataState[i] = map(potState[i], 0, 1023, 0, 255);

    int potVar = abs(potState[i] - potPState[i]);

    if (potVar > potThreshold) {
      lastPotTime[i] = millis(); //reset
    }

    potTimer[i] = millis() - lastPotTime[i];

    if (potTimer[i] < POT_TIMEOUT && millis() >= POT_TIMEOUT) {
      if (dataState[i] != dataPState[i]) {
        delay(20);
        Serial.write9bit(to9Bits(potCC[i], 1)); // send initial 128 ping + 0 value
        delay(20);
        Serial.write9bit(to9Bits(dataState[i], 0)); // send initial 128 ping + 0 value


        dataPState[i] = dataState[i];
      }
      potPState[i] = potState[i];

    }
  }
}

int to9Bits(int address, int value)
{
  if (value == 1) {
    address += 256;
  }
  return address;
}


/* =================================================== */


//8 value or address bits, 9th (left most one) bit indicates if byte is a value/mask or an address:
//0: Value or mask
//1: Address

//POTENTIOMETERS (addresses above 15 only)

//1: Address
//2: Value (0-255)

//Potmeter     Address
//Fine tune       16
//Tune            17
//Env amount      18
//LFO amount      19
//Source mix      20
//HPF             21
//Resonance       22
//Cut off freq    23
//Env mod         24

//Potmeter     Address
//LFO mod         25
//Pitch follow    26
//VCA level       27
//LFO rate        28
//LFO delay time  29
//Attack          30
//Decay           31
//Sustain         32
//Release         33

//SPECIAL COMMANDS

//Command                Address       Value
//Manual                  130           all address/value bytes (?)
//Write                   129           0
//Ping (sent on startup)  128           0

