
#include <NewPing.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

//#define DEBUG

// debugger output
#ifdef DEBUG
  #define DEBUG_PRINT(x)   Serial.print (x)
  #define DEBUG_PRINTDEC(x)Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x) Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
#endif

//-----------
// Pin setups
//-----------
// ultrasonic sensor setup
#define TRIGGER_PIN  11
#define ECHO_PIN     12

// initialize the LED shield
#define LED_DRIVER_PIN 13

// delay between pings
const int PING_DELAY = 100;

// distance measurement limits in CM
const int MAX_DISTANCE    = 200;
const int GREEN_DISTANCE  = 115;
const int YELLOW_DISTANCE = 100;
const int RED_DISTANCE    = 90;
const int MIN_DISTANCE    = 10;  // you'll never be this close; ignore distances of 0 when there's nothing inside of the rangefinder's effective range to measure against

// set color shortcuts for LED lighting
const int OFF    = 0;
const int GREEN  = 1;
const int YELLOW = 2;
const int RED    = 3;

// the max number of duplicate measurements we'll accept before turning the NeoPixel off
const int MAX_DUPLICATES = 60;

/* setup params for the pixelshield
  Parameter 1 = number of pixels in strip
  Parameter 2 = Arduino pin number (most are valid)
  Parameter 3 = pixel type flags, add together as needed:
    NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
    NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

  Example syntax:
    Adafruit_NeoPixel pixelShield = Adafruit_NeoPixel(40, LED_DRIVER_PIN, NEO_GRB + NEO_KHZ800);
*/
Adafruit_NeoPixel pixelShield = Adafruit_NeoPixel(40, LED_DRIVER_PIN, NEO_GRB + NEO_KHZ800);

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// time-of-flight measurement from the rangefinder
int duration = 0;
// distance in CM as calculated from the rangefinder
int distance = 0;
// number of pings to measure for the input smoother
int iterations = 5;
// an accumulator to turn the NeoPixel off after X identical readings
int duplicate_accumulator = 0;
// capture the previous measurement for the accumulator
int last_reading = 0;
// the number of pixels available in the NeoPixel
int pixleCount = pixelShield.numPixels();

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

  pixelShield.begin();
  pixelShield.show(); // Initialize all pixels to 'off'

  delay(100); // let the arduino settle before issuing first ping
}

void loop() {
  delay(PING_DELAY);

  duration = sonar.ping_median(iterations);
  // convert duration measurement to CM
  distance = sonar.convert_cm(duration);

  DEBUG_PRINT(distance);
  DEBUG_PRINT(" cm");
  DEBUG_PRINTLN();

  if (last_reading == distance) {
    duplicate_accumulator++;
  } else {
    duplicate_accumulator = 0;
  }

  if (duplicate_accumulator > MAX_DUPLICATES) {
    DEBUG_PRINT("HIT MAX DUPLICATES WITH "); DEBUG_PRINT(duplicate_accumulator); DEBUG_PRINTLN();
    off();
  }
  else if (distance > MAX_DISTANCE || distance < MIN_DISTANCE) {
    off();
  }
  else if (distance >= GREEN_DISTANCE) {
    light_led(GREEN, 5);
  }
  else if (distance >= YELLOW_DISTANCE) {
    light_led(YELLOW, 10);
  }
  else if (distance >= RED_DISTANCE) {
    light_led(RED, 15);
  }
  else if (distance >= MIN_DISTANCE) {
    stopp();
  }
  else {
    off();
  }
  last_reading = distance;
  DEBUG_PRINT(duplicate_accumulator); DEBUG_PRINT(" duplicates"); DEBUG_PRINTLN();
}

/* Set the color for the pixelshield
  Colors:
    OFF   : 0
    GREEN : 1
    YELLOW: 2
    RED   : 3
  Brightness: 1-255
*/
void light_led(int color, int brightness) {
  int red = 0; int green = 0; int blue = 0;

  switch (color) {
    case 1: // green
      green = brightness;
      break;
    case 2: // yellow
      red = brightness;
      green = brightness;
      break;
    case 3: // red
      red = brightness;
      break;
    default:
      break;
  }
  for (int i = 0; i < pixleCount; i++) {
    pixelShield.setPixelColor(i, pixelShield.Color(red, green, blue));
  }
  pixelShield.show();
}

/* Flash red, rapidly */
void stopp() {
  light_led(RED, 30);
  delay(100);
  off();
}

/* Turn the pixelshield off */
void off() {
  light_led(OFF, 0);
}
