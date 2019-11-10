
#include <NewPing.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

//-----------
// Pin setups
//-----------
// ultrasonic sensor setup
#define TRIGGER_PIN  11
#define ECHO_PIN     12

// initialize the LED shield
#define LED_DRIVER_PIN 13

// distance measurement limits
const int MAX_DISTANCE = 200;
const int GREEN_MIN    = 46;
const int YELLOW_MAX   = 45;
const int YELLOW_MIN   = 40;
const int RED_MAX      = 39;
const int RED_MIN      = 36;

// set color shortcuts for LED lighting
const int OFF    = 0;
const int GREEN  = 1;
const int YELLOW = 2;
const int RED    = 3;

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

//--------------
// LIBRARY CALLS
//--------------
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

int distance = 0;

int pixleCount = pixelShield.numPixels();

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif

  pixelShield.begin();
  pixelShield.show(); // Initialize all pixels to 'off'
}

void loop() {
  delay(50);

  distance = (sonar.ping_in());

  #ifdef DEBUG
  Serial.println(distance);
  #endif

  if (distance >= GREEN_MIN) {
    light_led(GREEN,30);
  }
  else if (distance <= YELLOW_MAX && distance >= YELLOW_MIN) {
    light_led(YELLOW,50);
  }
  else if (distance <= RED_MAX && distance >= RED_MIN) {
    light_led(RED,50);
  }
  else if (distance < RED_MIN) {
    stopp();
  }
  else {
    off();
  }
}

//----------
// FUNCTIONS
//----------
void light_led(int color, int brightness) {
  /* color shortcuts
    OFF    0
    GREEN  1
    YELLOW 2
    RED    3
  */
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
  for(int i = 0; i < pixleCount; i++) {
    pixelShield.setPixelColor(i, pixelShield.Color(red,green,blue));
  }
  pixelShield.show();
}

void stopp() {
  light_led(RED,127);
  delay(50);
  off();
}

void off() {
  light_led(OFF,0);
}
