#include <TFT_eSPI.h>
#include "efei_gauge.h"
#include "efei_needles.h"
#include "Seven_Segment12pt7b.h"

//Pins for LCD
#define TFT_CS      15
#define TFT_RST     4 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC      2
#define TFT_DIN     13
#define TFT_SCLK    14

//create instance of LCD
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

TFT_eSprite gaugeBack = TFT_eSprite(&tft);
TFT_eSprite needle_L = TFT_eSprite(&tft);
TFT_eSprite needle_R = TFT_eSprite(&tft);

TFT_eSprite gaugeBackNight = TFT_eSprite(&tft);
TFT_eSprite needle_L_night = TFT_eSprite(&tft);
TFT_eSprite needle_R_night = TFT_eSprite(&tft);


char buffer_L[4];
char buffer_R[4];
bool nightMode=1;


void setup() {
                                // use this initializer if using a 2.0" 320x240 TFT:
  tft.init();                   // Init ST7789 320x240
  tft.setRotation(1);           // set rotation horizontal, connector on left side

  gaugeBack.setSwapBytes(true);
  gaugeBackNight.setSwapBytes(true);
  needle_L.setSwapBytes(true);
  needle_R.setSwapBytes(true);
  needle_L_night.setSwapBytes(true);
  needle_R_night.setSwapBytes(true);

  
  createNeedle();
  createNeedleNight();

  pinMode(18, INPUT_PULLUP);

}

void loop() {
  
  uint16_t angle_L=0;
  uint16_t angle_R=0;
  uint16_t oil_L=0;
  uint16_t oil_R=0;
  
  for(int i=0; i<=100;i+=2){
    //Left needle moves from 0 to 90 degrees, values tweaked based to align with gauge marks
    //Right needles moves from 0 to -90 degrees, values tweaked based to align with gauge marks
    angle_L = map(i, 0, 100, 2, 83);
    angle_R = map(i, 0, 100, -4, -85);
    oil_L = map(i, 0, 100, 30, 999);
    oil_R = map(i, 0, 100, 26, 994);

    //pad OIL values to 3 digits
    snprintf(buffer_L,sizeof(buffer_L), "%03d", oil_L);
    snprintf(buffer_R,sizeof(buffer_R), "%03d", oil_R);

    nightMode = digitalRead(18);

    if(nightMode==1){
      plotGauge(angle_L, angle_R, String(buffer_L), String(buffer_R));
    }else{
      plotGaugeNight(angle_L, angle_R, String(buffer_L), String(buffer_R));
    }

  }

  for(int i=100; i>=0;i-=2){

    angle_L = map(i, 0, 100, 2, 83);
    angle_R = map(i, 0, 100, -4, -85);
    oil_L = map(i, 0, 100, 30, 999);
    oil_R = map(i, 0, 100, 26, 994);
    
    snprintf(buffer_L,sizeof(buffer_L), "%03d", oil_L);
    snprintf(buffer_R,sizeof(buffer_R), "%03d", oil_R);
    
    nightMode = digitalRead(18);

    if(nightMode==1){
      plotGauge(angle_L, angle_R, String(buffer_L), String(buffer_R));
    }else{
      plotGaugeNight(angle_L, angle_R, String(buffer_L), String(buffer_R));
    }
  }
}


void plotGauge(uint16_t angle_L, uint16_t angle_R, String oil_L, String oil_R){
  createBackground();
  //set text and update Oil values
  gaugeBack.setTextSize(2);
  gaugeBack.setTextColor(TFT_WHITE);
  gaugeBack.setFreeFont(&Seven_Segment12pt7b);
  gaugeBack.drawString(oil_L, 33, 180);
  gaugeBack.drawString(oil_R, 213, 180);

  //set pivot to upper left corner and update left needle rotation
  gaugeBack.setPivot(0,0);
  needle_L.pushRotated(&gaugeBack, angle_L, TFT_TRANSPARENT);

  //set pivot to upper right corner and update right needle rotation
  gaugeBack.setPivot(320,0);
  needle_R.pushRotated(&gaugeBack, angle_R, TFT_TRANSPARENT);
  
  gaugeBack.pushSprite(0,0,TFT_TRANSPARENT);

}

void createBackground(void){
  gaugeBack.setColorDepth(8);
  gaugeBack.createSprite(320,240);
  gaugeBack.fillSprite(TFT_TRANSPARENT);
  gaugeBack.pushImage(0,0,320,240,efei_gauge);
}

void createNeedle(void){
  needle_L.setColorDepth(8);
  needle_L.createSprite(113,9);
  needle_L.pushImage(0,0,113,9,efei_needle_L);
  needle_L.setPivot(5,0); 

  needle_R.setColorDepth(8);
  needle_R.createSprite(113,9);
  needle_R.pushImage(0,0,113,9,efei_needle_R);
  needle_R.setPivot(113,4); 
}

//functions for night mode
void plotGaugeNight(uint16_t angle_L, uint16_t angle_R, String oil_L, String oil_R){
  createBackgroundNight();
  //set text and update Oil values
  gaugeBackNight.setTextSize(2);
  gaugeBackNight.setTextColor(TFT_GREEN);
  gaugeBackNight.setFreeFont(&Seven_Segment12pt7b);
  gaugeBackNight.drawString(oil_L, 33, 180);
  gaugeBackNight.drawString(oil_R, 213, 180);

  //set pivot to upper left corner and update left needle rotation
  gaugeBackNight.setPivot(0,0);
  needle_L_night.pushRotated(&gaugeBackNight, angle_L, TFT_TRANSPARENT);

  //set pivot to upper right corner and update right needle rotation
  gaugeBackNight.setPivot(320,0);
  needle_R_night.pushRotated(&gaugeBackNight, angle_R, TFT_TRANSPARENT);
  
  gaugeBackNight.pushSprite(0,0,TFT_TRANSPARENT);

}

void createBackgroundNight(void){
  gaugeBackNight.setColorDepth(8);
  gaugeBackNight.createSprite(320,240);
  gaugeBackNight.fillSprite(TFT_TRANSPARENT);
  gaugeBackNight.pushImage(0,0,320,240,eng_gauge_night);
}

void createNeedleNight(void){
  needle_L_night.setColorDepth(8);
  needle_L_night.createSprite(113,9);
  needle_L_night.pushImage(0,0,113,9,efei_needle_L_night);
  needle_L_night.setPivot(5,0); 

  needle_R_night.setColorDepth(8);
  needle_R_night.createSprite(113,9);
  needle_R_night.pushImage(0,0,113,9,efei_needle_R_night);
  needle_R_night.setPivot(113,4); 
}
