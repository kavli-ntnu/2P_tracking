/**************************************************************************
Tracking bar LED controller code

Uses a OLED mini display with integrated buttons (https://www.adafruit.com/product/3045), 
mounted on a Feather M0 express (Adafruit) to display / control status of a pair of 
nano neopixel LEDs (https://www.adafruit.com/product/3484).

 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define down          5
#define next          6
#define up            9

#define NEOPIN        11
#define NUMPIXELS     2
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

int up_state , next_state, down_state = (0,0,0);

int led1_r = 150;
int led1_g = 0;
int led1_b = 0;
int led2_r = 0;
int led2_g = 0;
int led2_b = 150;

int pos = 0;

void setup() {
  // buttons
  pinMode(up, INPUT_PULLUP);
  pinMode(next, INPUT_PULLUP);  
  pinMode(down, INPUT_PULLUP);
  
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Clear the buffer
  display.clearDisplay();
  
  // Invert and restore display, pausing in-between
  display.invertDisplay(true);
  delay(10);
  display.invertDisplay(false);
  delay(10);
  pixels.begin();
  
}

void loop() {
  
  up_state   = digitalRead(up);
  next_state = digitalRead(next);
  down_state = digitalRead(down);

  if (next_state == LOW){
    pos += 1;
    if(pos >= 7){
      pos = 0; 
    }
  }

  if (up_state == LOW){
    if (pos == 0) { led1_r += 5;};
    if (pos == 1) { led1_g += 5;};
    if (pos == 2) { led1_b += 5;};
    if (pos == 3) { led2_r += 5;};
    if (pos == 4) { led2_g += 5;};
    if (pos == 5) { led2_b += 5;};
  }
  if (down_state == LOW){
    if (pos == 0) { led1_r -= 5;};
    if (pos == 1) { led1_g -= 5;};
    if (pos == 2) { led1_b -= 5;};
    if (pos == 3) { led2_r -= 5;};
    if (pos == 4) { led2_g -= 5;};
    if (pos == 5) { led2_b -= 5;};
  }

  led1_r = constrain(led1_r,0,255);
  led1_g = constrain(led1_g,0,255);
  led1_b = constrain(led1_b,0,255);
  led2_r = constrain(led2_r,0,255);
  led2_g = constrain(led2_g,0,255);
  led2_b = constrain(led2_b,0,255);
  
  led_control(pos);

  delay(90);
}

void led_control(int pos) {

  pixels.setPixelColor(0, pixels.Color(led1_r,led1_g,led1_b)); 
  pixels.setPixelColor(1, pixels.Color(led2_r,led2_g,led2_b)); 
  pixels.show();
  
  display.clearDisplay();

  // FIRST LINE
  display.setTextSize(2);             
  display.setTextColor(BLACK, WHITE); 
  display.setCursor(0,0);             
  display.print(F("LED1"));
  display.setTextColor(WHITE);
  display.print(F(" "));
  display.setTextSize(1);
  if (pos == 0) {
    display.setTextColor(BLACK, WHITE); 
    display.print(led1_r);
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(WHITE);
    display.print(led1_r);
  }
  display.print(F(" "));
  if (pos == 1) {
    display.setTextColor(BLACK, WHITE); 
    display.print(led1_g);
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(WHITE);
    display.print(led1_g);
  }
  display.print(F(" "));
  if (pos == 2) {
    display.setTextColor(BLACK, WHITE); 
    display.print(led1_b);
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(WHITE);
    display.print(led1_b);
  }
  display.setTextSize(2);  
  display.println(F(""));
  
  // NEXT LINE
  display.setTextColor(BLACK, WHITE); 
  display.print(F("LED2"));
  display.setTextColor(WHITE);
  display.print(F(" "));
  display.setTextSize(1);
  if (pos == 3) {
    display.setTextColor(BLACK, WHITE); 
    display.print(led2_r);
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(WHITE);
    display.print(led2_r);
  }
  display.print(F(" "));
  if (pos == 4) {
    display.setTextColor(BLACK, WHITE); 
    display.print(led2_g);
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(WHITE);
    display.print(led2_g);
  }  display.print(F(" "));
    if (pos == 5) {
    display.setTextColor(BLACK, WHITE); 
    display.print(led2_b);
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(WHITE);
    display.print(led2_b);
  }

  display.display();
}




