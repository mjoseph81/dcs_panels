#include <SPI.h>             // SPI library for comm to LCD and CAN interface
#include <mcp2515.h>         // Library for MCP2515 CAN BUS
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

#define DEBUG       0
#define SOLID       0

//Pins for LCD and CAN
#define TFT_CS      9
#define TFT_RST     8 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC      7
#define CAN_CS      10

//messages and instance for CAN BUS
struct can_frame canMsgRx;
struct can_frame canMsgTx;
MCP2515 mcp2515(CAN_CS);  

//create instance of LCD
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define ON      1
#define OFF     0

//define Pins for input switches
#define btn_gear      3
#define btn_half_flap 4
#define btn_full_flap 5
#define btn_hook      6


const uint8_t selfTestNum = 2;

//variables for indicators from CAN msg
uint8_t noseGearValue = 1;
uint8_t leftGearValue = 1;
uint8_t rightGearValue = 1;
uint8_t halfFlapValue = 1;
uint8_t fullFlapValue = 1;
uint8_t hookValue = 1;
uint8_t activeIndicator = 1;

uint8_t noseGearLast;
uint8_t leftGearLast;
uint8_t rightGearLast;
uint8_t halfFlapLast;
uint8_t fullFlapLast;
uint8_t hookLast;
uint8_t activeIndicatorLast=0;

//variables for switch inputs
uint8_t gearCmdValue = 1;
uint8_t halfFlapCmdValue = 1;
uint8_t fullFlapCmdValue = 1;
uint8_t hookCmdValue = 1;

uint8_t gearCmdLast;
uint8_t halfFlapCmdLast;
uint8_t fullFlapCmdLast;
uint8_t hookCmdLast;

uint8_t canSwitchCmds = 0xFF;
uint8_t canIndicators = 0xFF;

//variables for CAN msg sending
long msgSendTime = 0;
const long msgSendInt = 100;
String myStr;

void setup() {
  //setup CAN interface
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();

  
  Serial.begin(115200);
  
  
  lcdInit();

  if(DEBUG){
    Serial.println("------- CAN Read ----------");
    Serial.println("ID  DLC   DATA");
  }

  //setup pins for switch inputs
  pinMode(btn_gear, INPUT_PULLUP);
  pinMode(btn_half_flap, INPUT_PULLUP);
  pinMode(btn_full_flap, INPUT_PULLUP);
  pinMode(btn_hook, INPUT_PULLUP); 
}

void loop() {
  //get input from switches
  readButtons();

  //send status message
  if(msgSendTime == 0){
    msgSendTime = millis();
    sendMsg();
  }else if(millis() - msgSendTime >= msgSendInt){
    msgSendTime = millis();
    sendMsg();
  }
  
  //check for messages on the CAN BUS
  if (mcp2515.readMessage(&canMsgRx) == MCP2515::ERROR_OK) {
    if((canMsgRx.can_id == 0x35) && (canMsgRx.data[0] == 0x80)){
      //in CAN message is "indicator state" message then process
      if(DEBUG){
        Serial.print("RX: "); 
        Serial.print(canMsgRx.can_id, HEX); // print ID
        Serial.print(" "); 
        Serial.print(canMsgRx.can_dlc, HEX); // print DLC
        Serial.print(" ");
        
        for (int i = 0; i<canMsgRx.can_dlc; i++)  {  // print the data
          Serial.print(canMsgRx.data[i],HEX);
          Serial.print(" ");
        }
        Serial.println();
      }

      //save previous values
      noseGearLast = noseGearValue;
      leftGearLast = leftGearValue;
      rightGearLast = rightGearValue;
      halfFlapLast = halfFlapValue;
      fullFlapLast = fullFlapValue;
      hookLast = hookValue;

      //read new values from CAN message
      noseGearValue = bitRead(canMsgRx.data[1], 0);
      leftGearValue = bitRead(canMsgRx.data[1], 1);
      rightGearValue = bitRead(canMsgRx.data[1], 2);
      halfFlapValue = bitRead(canMsgRx.data[1], 3);
      fullFlapValue = bitRead(canMsgRx.data[1], 4);
      hookValue = bitRead(canMsgRx.data[1], 5);

      evaluateIndicators();
    }
  }
}

void sendMsg(void){
  //bitpack for switch states
  bitWrite(canSwitchCmds, 0, gearCmdValue);
  bitWrite(canSwitchCmds, 1, halfFlapCmdValue);
  bitWrite(canSwitchCmds, 2, fullFlapCmdValue);
  bitWrite(canSwitchCmds, 3, hookCmdValue);
  
  //build CAN message
  canMsgTx.can_id  = 0x36;
  canMsgTx.can_dlc = 2;
  canMsgTx.data[0] = 0x01;
  canMsgTx.data[1] = canSwitchCmds;

  //send CAN message 
  mcp2515.sendMessage(&canMsgTx);

  if(DEBUG){
    Serial.print("TX: "); 
    Serial.print(canMsgTx.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsgTx.can_dlc, HEX); // print DLC
    Serial.print(" ");
    for (int i = 0; i<canMsgTx.can_dlc; i++)  {  // print the data
      Serial.print(canMsgTx.data[i],HEX);
      Serial.print(" ");
      myStr += (char)canMsgTx.data[i];
    }
    Serial.print(" - Data: " + myStr);
    Serial.println();     
    myStr = ""; 
  }
}

void lcdInit(void){
  // use this initializer if using a 2.0" 320x240 TFT:
  tft.init(240, 320);           // Init ST7789 320x240
  tft.setRotation(3);           // set rotation horizontal, connector on left side
  
  
  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  //tft.setSPISpeed(40000000);
  Serial.println("Display Initialized");

  tft.fillScreen(ST77XX_BLACK);
  delay(500);
 
  //run indicator self test
  indicatorSelfTest();

  Serial.println("done");
  delay(1000);

  tft.fillScreen(ST77XX_BLACK);
}

void noseGear(uint8_t state){
  if(state == ON){
     //Indicator for NOSE Gear on
    if(SOLID){     
      tft.fillRoundRect(90, 10, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(120, 25);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_BLACK);
      tft.println("NOSE");
    }else{
      tft.drawRoundRect(90, 10, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(120, 25);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_GREEN);
      tft.println("NOSE");
    }
  }else{
    //Indicator for NOSE Gear off
    tft.fillRoundRect(90, 10, 130, 50, 8, ST77XX_BLACK);
  }
}

void leftGear(uint8_t state){
  if(state == ON){
    if(SOLID){
      //Indicator for LEFT Gear on
      tft.fillRoundRect(20, 70, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(50, 85);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_BLACK);
      tft.println("LEFT");
    }else{
      tft.drawRoundRect(20, 70, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(50, 85);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_GREEN);
      tft.println("LEFT");
    }
  }else{
    //Indicator for LEFT Gear on
    tft.fillRoundRect(20, 70, 130, 50, 8, ST77XX_BLACK);
  }
}

void rightGear(uint8_t state){
  if(state == ON){
    if(SOLID){
      //Indicator for RIGHT Gear on
      tft.fillRoundRect(160, 70, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(185, 85);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_BLACK);
      tft.println("RIGHT");
    }else{
      tft.drawRoundRect(160, 70, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(185, 85);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_GREEN);
      tft.println("RIGHT");
    }
  }else{
    //Indicator for RIGHT Gear off
    tft.fillRoundRect(160, 70, 130, 50, 8, ST77XX_BLACK);
  }
}

void halfFlaps(uint8_t state){
  if(state == ON){
    if(SOLID){
      //Indicator for HALF Flaps on
      tft.fillRoundRect(20, 130, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(50, 145);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_BLACK);
      tft.println("HALF");
    }else{
      //Indicator for HALF Flaps on
      tft.drawRoundRect(20, 130, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(50, 145);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_GREEN);
      tft.println("HALF");
    }
  }else{
    //Indicator for HALF Flaps off
    tft.fillRoundRect(20, 130, 130, 50, 8, ST77XX_BLACK);
  }
}

void fullFlaps(uint8_t state){
  if(state == ON){
    if(SOLID){
      //Indicator for FULL Flaps on
      tft.fillRoundRect(160, 130, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(192, 145);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_BLACK);
      tft.println("FULL");
    }else{
      tft.drawRoundRect(160, 130, 130, 50, 8, ST77XX_GREEN);
      tft.setCursor(192, 145);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_GREEN);
      tft.println("FULL");
    }
  }else{
    //Indicator for FULL Flaps off
    tft.fillRoundRect(160, 130, 130, 50, 8, ST77XX_BLACK);
  }
}

void hookState(uint8_t state){
  if(state == ON){
    if(SOLID){
      //Indicator for HOOK on
      tft.fillRoundRect(90, 190, 130, 50, 8, ST77XX_YELLOW);
      tft.setCursor(120, 205);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_BLACK);
      tft.println("HOOK");
    }else{
      tft.drawRoundRect(90, 190, 130, 50, 8, ST77XX_YELLOW);
      tft.setCursor(120, 205);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_YELLOW);
      tft.println("HOOK");
    }
  }else{
    //Indicator for HOOK off
    tft.fillRoundRect(90, 190, 130, 50, 8, ST77XX_BLACK);
  }
}

void indicatorSelfTest(void){

  //display message that "self-test" is starting
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(90, 100);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Starting");
  tft.setCursor(80, 140);
  tft.println("Self-Test");
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);
  
  int i=0;
  for(i=0;i<selfTestNum;i++){
    Serial.println("Starting indicator self test....");
    delay(500);
    
    Serial.println("All gear indicators OFF");
    noseGear(OFF);
    leftGear(OFF);
    rightGear(OFF); 
    delay(500); 

    Serial.println("NOSE gear indicator ON");
    noseGear(ON);
    delay(500);

    Serial.println("LEFT and RIGHT gear indicators ON");
    leftGear(ON);
    rightGear(ON);
    delay(500);
  }

  for(i=0;i<selfTestNum;i++){
    Serial.println("HALF flaps indicator ON, FULL indicator OFF");
    halfFlaps(ON);
    fullFlaps(OFF);
    delay(500);

    Serial.println("FULL flaps indicator ON, HALF indicator OFF");
    halfFlaps(OFF);
    fullFlaps(ON);
    delay(500);
  }
  
  for(i=0;i<selfTestNum;i++){
    Serial.println("HOOK indicator ON");
    hookState(ON);
    delay(500);

    Serial.println("HOOK indicator OFF");
    hookState(OFF);
    delay(500);
  }
  

  Serial.println("Turn all indicators OFF");
  noseGear(OFF);
  leftGear(OFF);
  rightGear(OFF);
  halfFlaps(OFF);
  fullFlaps(OFF);
  hookState(OFF);

  //display message that "self-test" is complete
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(80, 100);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Self-Test");
  tft.setCursor(90, 140);
  tft.println("Complete");

  Serial.println("Indicator self test complete."); 
}

void evaluateIndicators(void){
  //check if any indicators are active  (active LOW)
  activeIndicatorLast = activeIndicator;
  if(noseGearValue == LOW ||  leftGearValue == LOW || rightGearValue == LOW || halfFlapValue == LOW || fullFlapValue == LOW || hookValue == LOW){
    activeIndicator = ON;
  }else{
    activeIndicator = OFF;
  }

  //if any indicator active the clear screen to remove message
  if(activeIndicator != activeIndicatorLast){
    if(activeIndicator == ON){
        tft.fillScreen(ST77XX_BLACK);
    }
  }
    
  //activate NOSE Gear indicator when switch is enabled
  if(noseGearLast != noseGearValue){
    if(noseGearValue == LOW){
      noseGear(ON);
    }else{
      noseGear(OFF);
    }
  }

  //activate LEFT Gear indicator when switch is enabled
  if(leftGearLast != leftGearValue){
    if(leftGearValue == LOW){
      leftGear(ON);
    }else{
      leftGear(OFF);
    }
  }

   //activate RIGHT Gear indicator when switch is enabled
  if(rightGearLast != rightGearValue){
    if(rightGearValue == LOW){
      rightGear(ON);
    }else{
      rightGear(OFF);
    }
  }
  
  //activate HALF flaps indicator when switch is enabled
  if(halfFlapLast != halfFlapValue){
    if(halfFlapValue == LOW){
      halfFlaps(ON);
    }else{
      halfFlaps(OFF);
    }
  }

  //activate FULL flaps indicator when switch is enabled
  if(fullFlapLast != fullFlapValue){
    if(fullFlapValue == LOW){
      fullFlaps(ON);
    }else{
      fullFlaps(OFF);
    }
  }

  //activate HOOK indicator when switch is enabled
  if(hookLast != hookValue){
    if(hookValue == LOW){
      hookState(ON);
    }else{
      hookState(OFF);
    }  
  }

  //display message when no indicators are active on state change
  if(activeIndicator != activeIndicatorLast){
    if(activeIndicator == OFF){
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(40, 120);
      tft.setTextSize(3);
      tft.setTextColor(ST77XX_WHITE);
      tft.println("No Indicators");
    }
  }
}

void readButtons(void){
  //remember previous value and read buttons 
  gearCmdLast = gearCmdValue;
  gearCmdValue = digitalRead(btn_gear);

  halfFlapCmdLast = halfFlapCmdValue;
  halfFlapCmdValue = digitalRead(btn_half_flap);

  fullFlapCmdLast = fullFlapCmdValue;
  fullFlapCmdValue = digitalRead(btn_full_flap);

  hookCmdLast = hookCmdValue;
  hookCmdValue = digitalRead(btn_hook);
}
