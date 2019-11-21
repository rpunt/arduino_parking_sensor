#include <NewPing.h>

#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

// enable debug logging? uncomment this
//#define DEBUG

/***********
* pin setups
***********/
// ultrasonic sensor setup
#define TRIGGER_PIN    11
#define ECHO_PIN       12
// initialize the LED shield
#define LED_DRIVER_PIN 13

// LED shield specs
const int LCD_COLUMNS = 8;
const int LCD_ROWS    = 5;

// delay between pings
const int PING_DELAY = 200;

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
const int OFF    = 0;
const int GREEN  = 1;
const int YELLOW = 2;
const int RED    = 3;

// the max number of duplicate measurements we'll accept before turning the NeoPixel off
const int MAX_DUPLICATES = 120;

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

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// time-of-flight measurement from the rangefinder
int duration = 0;
// distance in CM as calculated from the rangefinder
int distance = 0;
// number of pings to measure for the input smoother
int iterations = 7;
// an accumulator to turn the NeoPixel off after X identical readings
int duplicate_accumulator = 0;
// capture the previous measurement for the accumulator
int last_reading = 0;
// the number of pixels available in the NeoPixel
int pixleCount = pixelShield.numPixels();
// the number of pixels per row to fill when presented as a bar graph
int columnHeight = 0;

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

  duration = sonar.ping_median(iterations); // get time-of-flight measurements from the sensor
  distance = sonar.convert_cm(duration);    // convert duration measurement to CM

  DEBUG_PRINT(distance); DEBUG_PRINT(" cm"); DEBUG_PRINTLN();

  if (last_reading == distance) {
    duplicate_accumulator++;
  } else {
    duplicate_accumulator = 0;
  }

  if (duplicate_accumulator > MAX_DUPLICATES) {
    DEBUG_PRINT("HIT MAX DUPLICATES WITH "); DEBUG_PRINT(duplicate_accumulator); DEBUG_PRINTLN();
    led_off();
  }
  else if (distance > MAX_DISTANCE || distance < SHUTOFF_DISTANCE) {
    led_off();
  }
  else if (distance >= GREEN_DISTANCE) {
    columnHeight = columnFill(distance, GREEN_RANGE, GREEN_DISTANCE);
    DEBUG_PRINT(columnHeight); DEBUG_PRINT(" columnHeight"); DEBUG_PRINTLN();
    light_led(GREEN, columnHeight, 5);
  }
  else if (distance >= YELLOW_DISTANCE) {
    columnHeight = columnFill(distance, YELLOW_RANGE, YELLOW_DISTANCE);
    DEBUG_PRINT(columnHeight); DEBUG_PRINT(" columnHeight"); DEBUG_PRINTLN();
    light_led(YELLOW, columnHeight, 10);
  }
  else if (distance >= STOP_DISTANCE) {
    columnHeight = columnFill(distance, RED_RANGE, STOP_DISTANCE);
    DEBUG_PRINT(columnHeight); DEBUG_PRINT(" columnHeight"); DEBUG_PRINTLN();
    light_led(RED, columnHeight, 15);
  }
  else if (distance >= SHUTOFF_DISTANCE) {
    stopp();
  }
  else {
    led_off();
  }
  last_reading = distance;
  DEBUG_PRINT(duplicate_accumulator); DEBUG_PRINT(" duplicates"); DEBUG_PRINTLN(); DEBUG_PRINTLN();
}

/* Set the color for the pixelshield
  Colors:
    OFF   : 0
    GREEN : 1
    YELLOW: 2
    RED   : 3
  Brightness: 1-255
*/
void light_led(int color, int columnHeight, int brightness) {
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

  for (int i = 0; i < LCD_ROWS; i++) {
    for (int j = 0; j < LCD_COLUMNS; j++) {
      int pixel = (i * LCD_COLUMNS) + j;
      if (j < columnHeight) {
        pixelShield.setPixelColor(pixel, pixelShield.Color(red, green, blue));
      } else {
        pixelShield.setPixelColor(pixel, pixelShield.Color(0, 0, 0));
      }

    }
  }
  pixelShield.show();
}

/* Flash red, rapidly */
void stopp() {
  light_led(RED, 8, 30);
  delay(PING_DELAY);
  led_off();
}

/* Turn the pixelshield off */
void led_off() {
  light_led(OFF, 0, 0);
}

/*
  calculate the number of pixels to fill per column
  Object distance from rangefinder, Size of range for color, Minimum distance for color
*/
int columnFill(int objectDistance, int range, int colorDistance) {
  // TODO: there's gotta be a use for map() in here...
  // figure out how far into the color's range we have traveled
  int rangeConsumed = objectDistance - (colorDistance + range);
  /* 
  apprarently the arduino abs() function cannot be used as part of any other calculation
  the results will be incorrect
  isolate it
  */
  float usableRangeConsumed = abs(rangeConsumed);
  //                         percentage of range consumed     percent to fill per pixel
  float columnFillHeight = ((usableRangeConsumed/range)*100)/(100/LCD_COLUMNS);

  DEBUG_PRINT(range); DEBUG_PRINT(" color range"); DEBUG_PRINTLN();
  DEBUG_PRINT(colorDistance); DEBUG_PRINT(" min colorDistance"); DEBUG_PRINTLN();
  DEBUG_PRINT(rangeConsumed); DEBUG_PRINT(" range consumed"); DEBUG_PRINTLN();
  DEBUG_PRINT(usableRangeConsumed); DEBUG_PRINT(" abs rangeConsumed"); DEBUG_PRINTLN();
  DEBUG_PRINT(columnFillHeight); DEBUG_PRINT(" columnFillHeight from columnFill"); DEBUG_PRINTLN();

  if (columnFillHeight < 1) {
    return 1;
  }
  else if (columnFillHeight > LCD_COLUMNS) {
    return LCD_COLUMNS;
  }
  else {
    return columnFillHeight;
  }
}
