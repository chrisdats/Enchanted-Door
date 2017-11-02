/* Enchanted Door DPI 
Oct 25 2017
Chris Datsikas
*/

// Include libraries for NeoPixel
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// Connect Neopixel data line to this pin
#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// Include libraries for Capacitive Sensor, MPR121
#include <Wire.h>
#include "Adafruit_MPR121.h"

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// keep track of touches on which pin
boolean touch = false;
const byte capSensePin = 4;

// keeps track of previous IR sensor value
int previousValue = 0;
int onThreshold = 50;

byte state = 0;   // 0 - OFF (checking IR sensor)
                  // 1 - Bootup (when doors is activated Neopixels bootup)
                  // 2 - Standby (steady white, checking cap sensor and IR sensor) 
                  // 3 - Notify (when person approaches, show green light if 

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  // Neopixel setup
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(30); // Decrease brightness of Neopixels
  
  // Start Serial Communication
  Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
   
  // MPR121 Capacitive Sensor Setup
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  
  // Complete Setup
  state = 0;
  previousValue = analogRead(A0);
  Serial.println("Setup is Complete."); 
  
}

void loop() {
  // Some example procedures showing how to display to the pixels:
  switch (state)
  {
    case 0: // ** OFF
    {
      int currentValue = analogRead(A0); // Read IR Sensor value
      Serial.print("IR reading is: ");
      Serial.println(currentValue);      // Print IR Sensor value
      
      // If the difference is greater than threshold, change to bootup state
      if (currentValue - previousValue > onThreshold) {  
        state = 1;
        Serial.println("State set to 1");
      }
      previousValue = currentValue;
      Serial.print("Previous value set to: ");
      Serial.println(previousValue);
    } 
      break;
    case 1: // ** Bootup      
      // colorWipe
      Serial.println("colorWipe White");
      colorWipe(strip.Color(127, 127, 127), 100); // White
      
      // delay to avoid completing case 2 too early. TODO: find better way to do this
      delay(1000);
      
      // After bootup is complete, change to state 2
      state = 2;
      Serial.println("State set to 2");
      
      break;
    case 2: // ** Standby
    {
      // keep checking cap sensor 
      if (touch == false) {
        touch = checkCapSensor(capSensePin);
      }
      
      // keep checking IR sensor
      int currentValue = analogRead(A0);
      Serial.print("IR Sensor Value: ");
      Serial.println(currentValue);
      if (currentValue > 200) {
        state = 3;
        Serial.println("State set to 3");
      }
    }
      break;
    case 3: // ** Notify
      if (touch == true) {
        Serial.println("Neopixel color set to Green");
        colorWipe(strip.Color(0, 127, 0), 0); // Green
      }
      else {
        Serial.println("Neopixel color set to Red");
        colorWipe(strip.Color(127, 0, 0), 0); // Red
      }
  }
}

// checks the cap sensor when called
boolean checkCapSensor(int i) {
    // Get the currently touched pads
  currtouched = cap.touched();
    
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
      return true;
    }
    
  // reset our state
  lasttouched = currtouched;
  return false;
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i< 16; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}



