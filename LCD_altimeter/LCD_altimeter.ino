#include <TFT_eSPI.h>
#include "altimeter.h"
#include "needle.h"
//#include "MS3355812pt7b.h"

//Pins for LCD
#define TFT_CS      15
#define TFT_RST     4 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC      2
#define TFT_DIN     13
#define TFT_SCLK    14

//create instance of LCD
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

TFT_eSprite gaugeBack = TFT_eSprite(&tft);
TFT_eSprite needle = TFT_eSprite(&tft);
TFT_eSprite gaugeBackNight = TFT_eSprite(&tft);
TFT_eSprite needleNight = TFT_eSprite(&tft);

uint16_t alt=0;
uint16_t fbt=0;
char buffer[6];
int altitude=0;
bool nightMode=0;

void setup() {
                                // use this initializer if using a 2.0" 320x240 TFT:
  tft.init();                   // Init ST7789 320x240
  tft.setRotation(1);           // set rotation horizontal, connector on left side
  gaugeBack.setSwapBytes(true);
  needle.setSwapBytes(true);
  gaugeBackNight.setSwapBytes(true);
  needleNight.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);
  //tft.pushImage(40,0,240,240,altimeter);
  //tft.pushImage(150,20,21,115,needle);
  
  createNeedle();
  createNeedleNight();

  pinMode(18, INPUT_PULLUP);

}

void loop() {
  int angle=0;

  
  
  for(int i=0; i<=30000;i+=10){
    fbt = i % 1000;
    if(fbt >= 0 && fbt <=500){
      angle = map(fbt, 0, 500, 0, 180);
    }else{
      angle = map(fbt, 501, 999, -179, -1);
    }

    nightMode=digitalRead(18);

    snprintf(buffer,sizeof(buffer), "%05d", i);

    if(nightMode==1){
      plotGauge(angle, String(buffer));
    }else{
      plotGaugeNight(angle, String(buffer));
    }

    

    //delay(10);
  }
  

   /*  //////Code for potentiometer control
   int alt_change = map(analogRead(32), 0, 4096, -100, 100);
    altitude += alt_change;
    
    fbt = altitude % 1000;
    if(fbt > 0 && fbt <=500){
      angle = map(fbt, 0, 500, 0, 180);
    }else{
      angle = map(fbt, 501, 999, -180, -1);
    }
  
    snprintf(buffer,sizeof(buffer), "%05d", altitude);
    plotGauge(angle, String(buffer));
    delay(10);
    */
  
}

void plotGauge(uint16_t angle, String altString){
  createBackground();
  
  gaugeBack.setTextSize(3);
  gaugeBack.setTextColor(TFT_WHITE);
  //gaugeBack.setFreeFont(&MS3355812pt7b);
  gaugeBack.drawString(altString, 77, 80);
  needle.pushRotated(&gaugeBack, angle, TFT_TRANSPARENT);
  gaugeBack.pushSprite(40,0,TFT_TRANSPARENT);

}

void createBackground(void){
  gaugeBack.setColorDepth(8);
  gaugeBack.createSprite(240,240);
  gaugeBack.setPivot(120,120);
  tft.setPivot(160,120);
  gaugeBack.fillSprite(TFT_TRANSPARENT);
  gaugeBack.pushImage(0,0,240,240,altimeter);
}

void createNeedle(void){
  needle.setColorDepth(8);
  needle.createSprite(21,115);
  needle.pushImage(0,0,21,115,needleIMG);
  needle.setPivot(10,92);
  //tft.setPivot(160,120);  
}

//Night mode functions

void plotGaugeNight(uint16_t angle, String altString){
  createBackgroundNight();
  
  gaugeBackNight.setTextSize(3);
  gaugeBackNight.setTextColor(TFT_GREEN);
  //gaugeBack.setFreeFont(&MS3355812pt7b);
  gaugeBackNight.drawString(altString, 77, 80);
  needleNight.pushRotated(&gaugeBackNight, angle, TFT_TRANSPARENT);
  gaugeBackNight.pushSprite(40,0,TFT_TRANSPARENT);

}

void createBackgroundNight(void){
  gaugeBackNight.setColorDepth(8);
  gaugeBackNight.createSprite(240,240);
  gaugeBackNight.setPivot(120,120);
  tft.setPivot(160,120);
  gaugeBackNight.fillSprite(TFT_TRANSPARENT);
  gaugeBackNight.pushImage(0,0,240,240,altimeter_night);
}

void createNeedleNight(void){
  needleNight.setColorDepth(8);
  needleNight.createSprite(21,115);
  needleNight.pushImage(0,0,21,115,needleIMG_night);
  needleNight.setPivot(10,92);
  //tft.setPivot(160,120);  
}
