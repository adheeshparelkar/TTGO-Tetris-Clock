#include <TFT_eSPI.h>   // https://github.com/Bodmer/TFT_eSPI with TTGO-T-Display header https://github.com/Xinyuan-LilyGO/TTGO-T-Display/blob/master/TTGO_T_Display.h
#include <SPI.h>    // ESP32 library
#include <Ticker.h>   // ESP32 library
#include <TetrisTFTDraw.h>
#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include "Button2.h"
#include <esp_adc_cal.h>


#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23
#define TFT_BL              4  // Display backlight control pin
#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            35
#define BUTTON_2            0

#define DISPLAY_LIBRARY TFT_ESPI

#define TETRIS_SCALE_CLOCK  8
#define TETRIS_SCALE_LETTERS  5

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke TFT_eSPI library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

char buff[512];
int vref = 1100;
int btnCick = false;

const char* ssid     = "<WIFI-SSID";
const char* password = "WIFI-PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -25200;
const int   daylightOffset_sec = 0;

Ticker animationTicker;
boolean animate;
boolean pulseOn;
boolean updateNTP;
double animationTick = 0.15;    // default interval between drops in seconds
int tftBacklight = 205;      // backlight PWM 0-255
TFT_eSprite img = TFT_eSprite(&tft);    // using a Sprite to avoid flickering (double buffering technique)
TetrisTFTDraw tetris(img);

void animationHandlerClock()
{
    if (animate) 
    {
      img.fillSprite(TFT_BLACK);
      animate = !tetris.drawNumbers(0, 125, true);
      img.pushSprite(0, 0);
    }
}


void animationHandlerLetters()
{
    if (animate) 
    {
      img.fillSprite(TFT_BLACK);
      animate = !tetris.drawText(0, 125);
      img.pushSprite(0, 0);
    }
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println("Time variables");
  char timeHour[6];
  strftime(timeHour,6,"%I:%M", &timeinfo);
  Serial.println(timeHour);
    // Tetris blocks visual effects
  tetris.scale = TETRIS_SCALE_CLOCK;
  tetris.drawOutline = false;
  tetris.drawBlockOutline = false;
  tetris.draw3DOutline = true;      //3D outline effect
  tetris.draw3DDblOutline = false;
  animationTicker.attach(animationTick, animationHandlerClock);   // Set animation handler for numbers, a clock in this case
  tetris.setRandomDropStart();    // Set random start point for each tetromino in each digit
  tetris.setTime(timeHour, false);     // Draw time, refresh only changed digit. Change to true to redraw all digits.
  animate = true;
  do {
      delay(1);
    } while (animate);    // wait until animation completes

}
void setup()
{
    Serial.begin(115200);
    Serial.println("Start");
    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected.");
  
    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    tft.init();
    tft.setRotation(1);

    // Set backlight intensity using PWM output
    //ledcSetup(1, 1000, 8);   // PWM channel 1, 1000Hz, 8 bit
    //ledcAttachPin(TFT_BL, 1);   // attach to TFT backlight
    //ledcWrite(1, tftBacklight); // set backlight

    // use a sprite as double buffer to animate screen with no flickering
    // frame is drawn on the sprite, then the sprite (with black backround) is 
    // overimposed on the TFT screen, erasing previous content at the same time
    img.createSprite(240, 135);
    img.fillSprite(TFT_BLACK);

    yield();
    tft.fillScreen(TFT_BLACK);  // Clear TFT
}
void loop()
{
    delay(1000);
    printLocalTime();
    
}


