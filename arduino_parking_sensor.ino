#include <NewPing.h>

#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

// enable debug logging? uncomment this
// #define DEBUG

// ultrasonic sensor setup
#define TRIGGER_PIN    11
#define ECHO_PIN       12

// initialize the LED shield
// Keyestudio 2812
// #define LED_DRIVER_PIN 13
// Adafruit_NeoPixel
#define LED_DRIVER_PIN 6

// LED shield specs
const int LCD_COLUMNS = 8;
const int LCD_ROWS    = 5;

// delay between pings in ms; 29ms should be the shortest delay between pings.
const int PING_DELAY = 100;

// distance measurement limits in CM
const int MAX_DISTANCE     = 200;
const int STOP_DISTANCE    = 90;
const int SHUTOFF_DISTANCE = 10;  // you'll never be this close; ignore distances of 0 when there's nothing inside of the rangefinder's effective range to measure against

/****************************************************
you're unlikely to have to modify anything below here
****************************************************/

const int COLOR_RANGE  = (MAX_DISTANCE - STOP_DISTANCE) / 3;
const int GREEN_DISTANCE = MAX_DISTANCE - COLOR_RANGE;
const int YELLOW_DISTANCE = MAX_DISTANCE - (COLOR_RANGE * 2);

// define the ranges of each color distance for percentage calculation
const int GREEN_RANGE = MAX_DISTANCE - GREEN_DISTANCE;
const int YELLOW_RANGE = GREEN_DISTANCE - YELLOW_DISTANCE;
const int RED_RANGE = YELLOW_DISTANCE - STOP_DISTANCE;

// set color shortcuts for LED lighting
enum color { OFF, GREEN, YELLOW, RED };

// easy debug output
#ifdef DEBUG
  #define DEBUG_PRINT(x)   Serial.print (x)
  #define DEBUG_PRINTDEC(x)Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x) Serial.println (x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
#endif

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

NewPing sonar = NewPing(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// time-of-flight measurement from the rangefinder
int duration = 0;
// distance in CM as calculated from the rangefinder
int distance = 0;
// number of pings to measure for the input smoother
int iterations = 5;
// the number of pixels per row to fill when presented as a bar graph
int columnHeight = 0;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

  pixelShield.begin();
  pixelShield.show(); // Initialize all pixels to 'off'

  DEBUG_PRINT("green min: "); DEBUG_PRINT(GREEN_DISTANCE);
  DEBUG_PRINT("; yellow min: "); DEBUG_PRINT(YELLOW_DISTANCE);
  DEBUG_PRINT("; red min: "); DEBUG_PRINT(STOP_DISTANCE); DEBUG_PRINTLN(); DEBUG_PRINTLN();

  delay(100); // let the arduino settle before issuing first ping
}

void loop() {
  delay(PING_DELAY);

  duration = sonar.ping_median(iterations); // get time-of-flight measurements from the sensor
  distance = sonar.convert_cm(duration);    // convert duration measurement to CM

  DEBUG_PRINT("distance: "); DEBUG_PRINT(distance); DEBUG_PRINT(" cm; "); DEBUG_PRINTLN();

  if (inRange(distance, SHUTOFF_DISTANCE, MAX_DISTANCE)) {
    if (inRange(distance, STOP_DISTANCE, MAX_DISTANCE)) {
      if (distance >= GREEN_DISTANCE) {
        columnHeight = columnFill(distance, GREEN_RANGE, GREEN_DISTANCE);
        light_led(GREEN, columnHeight);
      }
      else if (distance >= YELLOW_DISTANCE) {
        columnHeight = columnFill(distance, YELLOW_RANGE, YELLOW_DISTANCE);
        light_led(YELLOW, columnHeight);
      }
      else if (distance >= STOP_DISTANCE) {
        columnHeight = columnFill(distance, RED_RANGE, STOP_DISTANCE);
        light_led(RED, columnHeight);
      }
      DEBUG_PRINT("columnHeight: "); DEBUG_PRINT(columnHeight); DEBUG_PRINTLN();
    }
    else { // if (distance >= SHUTOFF_DISTANCE)
      stopp();
    }
  }
  else {
    led_off();
  }
}

/* Set the color for the pixelshield
  Colors    : from enum color
  Brightness: 1-255
*/
void light_led(int color, int columnHeight) {
  int r = 0; int g = 0; int b = 0;

  // increase brightness as we get closer to stopping
  int brightness = color * 5;

  switch (color) {
    case GREEN:
      g = brightness;
      break;
    case YELLOW:
      r = brightness;
      g = brightness;
      break;
    case RED:
      r = brightness;
      break;
    default:
      break;
  }

  for (int i = 0; i < LCD_ROWS; i++) {
    for (int j = 0; j < LCD_COLUMNS; j++) {
      int pixel = (i * LCD_COLUMNS) + j;
      if (j < columnHeight) {
        pixelShield.setPixelColor(pixel, pixelShield.Color(r, g, b));
      } else {
        pixelShield.setPixelColor(pixel, pixelShield.Color(0, 0, 0));
      }

    }
  }
  pixelShield.show();
}

/* Flash red, rapidly */
void stopp() {
  light_led(RED, 8);
  delay(PING_DELAY);
  led_off();
}

/* Turn the pixelshield off */
void led_off() {
  light_led(OFF, 0);
}

/*
  calculate the number of pixels to fill per column
  Object distance from rangefinder, Size of range for color, Minimum distance for color
*/
int columnFill(int objectDistance, int range, int colorDistance) {
  // figure out how far into the color's range we have traveled
  int rangeConsumed = objectDistance - (colorDistance + range);
  /*
  apprarently the arduino abs() function cannot be used as part of any other calculation
  the results will be incorrect
  isolate it
  */
  float usableRangeConsumed = abs(rangeConsumed);
  float percentConsumed = (usableRangeConsumed/range)*100;
  float columnFillHeight = map(percentConsumed, 1, 100, 1, 8);

  DEBUG_PRINT("min colorDistance: "); DEBUG_PRINT(colorDistance); DEBUG_PRINTLN();
  DEBUG_PRINT("color range: "); DEBUG_PRINT(range);  DEBUG_PRINT("; ");
  DEBUG_PRINT(rangeConsumed); DEBUG_PRINT(" (");
  DEBUG_PRINT(usableRangeConsumed); DEBUG_PRINT(") range consumed"); DEBUG_PRINTLN();
  DEBUG_PRINT("columnFillHeight from columnFill: "); DEBUG_PRINT(columnFillHeight); DEBUG_PRINT("; ");

  return constrain(columnFillHeight, 1, LCD_COLUMNS);
}

// Check if value is in range
bool inRange(int d, int rangeMin, int rangeMax) {
  return d >= rangeMin && d <= rangeMax;
}
