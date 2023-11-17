#include <TFT_eSPI.h>
#include "g_meter_gauge.h"
#include "g_meter_needles.h"

//Pins for LCD
#define TFT_CS      15
#define TFT_RST     4 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC      2
#define TFT_DIN     13
#define TFT_SCLK    14
#define NIGHT_MODE  32
#define G_RST       33
#define POT         34

//create instance of LCD
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

TFT_eSprite gaugeBack = TFT_eSprite(&tft);
TFT_eSprite needle_curr = TFT_eSprite(&tft);
TFT_eSprite needle_pos = TFT_eSprite(&tft);
TFT_eSprite needle_neg = TFT_eSprite(&tft);

TFT_eSprite gaugeBack_night = TFT_eSprite(&tft);
TFT_eSprite needle_curr_night = TFT_eSprite(&tft);
TFT_eSprite needle_pos_night = TFT_eSprite(&tft);
TFT_eSprite needle_neg_night = TFT_eSprite(&tft);

bool nightMode=1;
bool g_reset_btn=1;
uint16_t pot_value=0;
uint16_t g_max_angle=1548;
uint16_t g_min_angle=1548;

uint16_t angle_curr=0;
uint16_t angle_pos=0;
uint16_t angle_neg=0;


void setup() {
                                // use this initializer if using a 2.0" 320x240 TFT:
  tft.init();                   // Init ST7789 320x240
  tft.setRotation(1);           // set rotation horizontal, connector on left side
  tft.fillScreen(TFT_BLACK);
  
  gaugeBack.setSwapBytes(true);
  gaugeBack_night.setSwapBytes(true);
  needle_curr.setSwapBytes(true);
  needle_pos.setSwapBytes(true);
  needle_neg.setSwapBytes(true);
  needle_curr_night.setSwapBytes(true);
  needle_pos_night.setSwapBytes(true);
  needle_neg_night.setSwapBytes(true);

  
  createNeedle();
  createNeedle_night();

  pinMode(NIGHT_MODE, INPUT_PULLUP);
  pinMode(G_RST, INPUT_PULLUP);
  pinMode(POT, INPUT);

}

void loop() {  

    g_reset_btn = digitalRead(G_RST);
    nightMode = digitalRead(NIGHT_MODE);

    if(g_reset_btn==0){
      g_max_angle=1548;
      g_min_angle=1548;
    }

    //Get G input value from POT  (0-4096)
    pot_value = analogRead(POT);
   
    //use POT value to map rotation degree for needle
    if(pot_value < 1548 && pot_value > 0){
      angle_curr = map(pot_value, 1547, 0, 0, -140);
    }else if(pot_value >=1548 && pot_value < 4096){
      angle_curr = map(pot_value, 1548, 3595, 0, 180);
    }else if(pot_value >=3596 && pot_value < 4096){
      angle_curr = map(pot_value, 3596, 4096, -180, -140);
    }

    //check for new max pos G value
    if(pot_value > g_max_angle){
      g_max_angle = pot_value;
    }

    //use g_max_angle value to map rotation degree for MAX Pos G needle
    if(g_max_angle < 1548 && g_max_angle > 0){
      angle_pos = map(g_max_angle, 1547, 0, 0, -140);
    }else if(g_max_angle >=1548 && g_max_angle < 4096){
      angle_pos = map(g_max_angle, 1548, 3595, 0, 180);
    }else if(g_max_angle >=3596 && g_max_angle < 4096){
      angle_pos = map(g_max_angle, 3596, 4096, -180, -140);
    }

    //check for new min neg G value
    if(pot_value < g_min_angle){
      g_min_angle = pot_value;
    }

    //use g_min_angle value to map rotation degree for MAX Neg G needle
    if(g_min_angle < 1548 && g_min_angle > 0){
      angle_neg = map(g_min_angle, 1547, 0, 0, -140);
    }else if(g_min_angle >=1548 && g_min_angle < 4096){
      angle_neg = map(g_min_angle, 1548, 3595, 0, 180);
    }else if(g_min_angle >=3596 && g_min_angle < 4096){
      angle_neg = map(g_min_angle, 3596, 4096, -180, -140);
    }
    

    if(nightMode==1){
      plotGauge(angle_curr, angle_pos, angle_neg);
    }else{
      plotGauge_night(angle_curr, angle_pos, angle_neg);
    }


}


void plotGauge(uint16_t angle_curr, uint16_t angle_pos, uint16_t angle_neg){
  createBackground();

  needle_pos.pushRotated(&gaugeBack, angle_pos, TFT_TRANSPARENT);
  needle_neg.pushRotated(&gaugeBack, angle_neg, TFT_TRANSPARENT);
  needle_curr.pushRotated(&gaugeBack, angle_curr, TFT_TRANSPARENT);
  gaugeBack.pushSprite(40,0,TFT_TRANSPARENT);

}

void createBackground(void){
  gaugeBack.setColorDepth(8);
  gaugeBack.createSprite(240,240);
  gaugeBack.setPivot(120,120);
  tft.setPivot(160,120);
  gaugeBack.fillSprite(TFT_TRANSPARENT);
  gaugeBack.pushImage(0,0,240,240,g_meter);
}

void createNeedle(void){
  needle_curr.setColorDepth(8);
  needle_curr.createSprite(115,21);
  needle_curr.pushImage(0,0,115,21,g_meter_needle_1);
  needle_curr.setPivot(75,10); 

  needle_pos.setColorDepth(8);
  needle_pos.createSprite(115,21);
  needle_pos.pushImage(0,0,115,21,g_meter_needle_2);
  needle_pos.setPivot(75,10); 

  needle_neg.setColorDepth(8);
  needle_neg.createSprite(115,21);
  needle_neg.pushImage(0,0,115,21,g_meter_needle_2);
  needle_neg.setPivot(75,10); 
}

//functions for night mode
void plotGauge_night(uint16_t angle_curr, uint16_t angle_pos, uint16_t angle_neg){
  createBackground_night();

  needle_pos_night.pushRotated(&gaugeBack_night, angle_pos, TFT_TRANSPARENT);
  needle_neg_night.pushRotated(&gaugeBack_night, angle_neg, TFT_TRANSPARENT);
  needle_curr_night.pushRotated(&gaugeBack_night, angle_curr, TFT_TRANSPARENT);
  gaugeBack_night.pushSprite(40,0,TFT_TRANSPARENT);

}

void createBackground_night(void){
  gaugeBack_night.setColorDepth(8);
  gaugeBack_night.createSprite(240,240);
  gaugeBack_night.setPivot(120,120);
  tft.setPivot(160,120);
  gaugeBack_night.fillSprite(TFT_TRANSPARENT);
  gaugeBack_night.pushImage(0,0,240,240,g_meter_night);
}

void createNeedle_night(void){
  needle_curr_night.setColorDepth(8);
  needle_curr_night.createSprite(115,21);
  needle_curr_night.pushImage(0,0,115,21,g_meter_needle_1_night);
  needle_curr_night.setPivot(75,10); 

  needle_pos_night.setColorDepth(8);
  needle_pos_night.createSprite(115,21);
  needle_pos_night.pushImage(0,0,115,21,g_meter_needle_2_night);
  needle_pos_night.setPivot(75,10); 

  needle_neg_night.setColorDepth(8);
  needle_neg_night.createSprite(115,21);
  needle_neg_night.pushImage(0,0,115,21,g_meter_needle_2_night);
  needle_neg_night.setPivot(75,10); 
}
