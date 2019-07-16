#include <NewPing.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

//--------
// DEFINES
//--------

#define GREEN_LED    7
#define YELLOW_LED   6
#define RED_LED      5
#define TRIGGER_PIN  11
#define ECHO_PIN     12
#define MAX_DISTANCE 200

#define GREEN_MIN  46
#define YELLOW_MAX 45
#define YELLOW_MIN 40
#define RED_MAX    39
#define RED_MIN    35

#define LED_DRIVER_PIN 13

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel pixelShield = Adafruit_NeoPixel(40, LED_DRIVER_PIN, NEO_GRB + NEO_KHZ800);

//--------------
// LIBRARY CALLS
//--------------
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

int distance = 0;

int pixleCount = pixelShield.numPixels();

void setup() {
  Serial.begin(9600);
  
  pixelShield.begin();
  pixelShield.show(); // Initialize all pixels to 'off'
}

void loop() {
  delay(50);
  
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  distance = (sonar.ping_in());
  Serial.println(distance);

  if (distance > GREEN_MIN) {
    green_led();
  }
  else if (distance <= YELLOW_MAX  && distance > YELLOW_MIN) {
    yellow_led();
  }
  else if (distance <= RED_MAX && distance > RED_MIN) {
    red_led();
  }
  else if (distance <= RED_MIN) {
    stopp();
  }
  else {
    off();
  }
}

//----------
// FUNCTIONS
//----------

void green_led() {
  for(int i=0; i < pixleCount; i++) {
    pixelShield.setPixelColor(i, pixelShield.Color(0,30,0));
  }
  pixelShield.show();
}

void yellow_led() {
  for(int i=0; i < pixleCount; i++) {
    pixelShield.setPixelColor(i, pixelShield.Color(127,127,0));
  }
  pixelShield.show();
}

void red_led() {
  for(int i=0; i < pixleCount; i++) {
    pixelShield.setPixelColor(i, pixelShield.Color(127,0,0));
  }
  pixelShield.show();
}

void stopp() {
  for(int i=0; i < pixleCount; i++) {
    pixelShield.setPixelColor(i, pixelShield.Color(127,0,0));
  }
  pixelShield.show();
  delay(50);
  off();
}

void off() {
  for(int i=0; i< pixelShield.numPixels(); i++) {
    pixelShield.setPixelColor(i, pixelShield.Color(0,0,0));
  }
  pixelShield.show();
}
