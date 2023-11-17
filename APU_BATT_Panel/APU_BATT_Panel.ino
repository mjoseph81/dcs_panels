#include <SPI.h>             // SPI library for comm to LCD and CAN interface
#include <mcp2515.h>         // Library for MCP2515 CAN BUS

#define DEBUG   0
#define SPI_CS  10

//PINS 10-13 used for SPI to CAN

//pins for switches
#define btn_batt_on             9
#define btn_batt_oride          8
#define btn_left_gen_norm       7
#define btn_right_gen_norm      6
#define btn_apu_on              5
#define led_apu_ready           4
#define btn_l_eng_crank         3
#define btn_r_eng_crank         1
#define btn_obogs_on            2
#define btn_wing_fold           A0
#define btn_wing_spread         A1

//variables for switch inputs
uint8_t battOnCmd = 1;
uint8_t battOrideCmd = 1;
uint8_t leftGenNormCmd = 1;
uint8_t rightGenNormCmd = 1;
uint8_t apuOnCmd = 1;
uint8_t leftEngCrankCmd = 1;
uint8_t rightEngCrankCmd = 1;
uint8_t obogsOnCmd = 1;
uint8_t wingFoldCmd = 1;
uint8_t wingSpreadCmd = 1;

uint8_t battOnLast;
uint8_t battOrideLast;
uint8_t leftGenNormLast;
uint8_t rightGenNormLast;
uint8_t apuOnLast;
uint8_t leftEngCrankLast;
uint8_t rightEngCrankLast;
uint8_t obogsOnLast;
uint8_t wingFoldLast;
uint8_t wingSpreadLast;

//variables for CAN message bitpacks
uint8_t canBattGenApu = 0xFF;
uint8_t canEngO2wings = 0xFF;

//message and instance for CAN interface
struct can_frame canMsgTx;
MCP2515 mcp2515(SPI_CS);  

//variables for CAN message sending
long msgSendTime = 0;
const long msgSendInt = 100;


void setup() { 
  //Set pins for input switches
  pinMode(btn_batt_on, INPUT_PULLUP);
  pinMode(btn_batt_oride, INPUT_PULLUP);
  pinMode(btn_left_gen_norm, INPUT_PULLUP);
  pinMode(btn_right_gen_norm, INPUT_PULLUP);
  pinMode(btn_apu_on, INPUT_PULLUP);
  pinMode(btn_l_eng_crank, INPUT_PULLUP);
  pinMode(btn_r_eng_crank, INPUT_PULLUP);
  pinMode(btn_obogs_on, INPUT_PULLUP);
  pinMode(btn_wing_fold, INPUT_PULLUP);
  pinMode(btn_wing_spread, INPUT_PULLUP);

  //set pin for output LED
  pinMode(led_apu_ready, OUTPUT);

  //setup CAN interface
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();

  if(DEBUG){
    Serial.begin(115200);
    Serial.println("------- CAN Write ----------");
    Serial.println("ID  DLC   DATA");    
  }

}

void loop() {

  readSwitches();
  
  //check if its time to send message for indicators
  if(msgSendTime == 0){
    msgSendTime = millis();
    sendMsg();
  }else if(millis() - msgSendTime >= msgSendInt){
    msgSendTime = millis();
    sendMsg();
  }

  delay(20);
}

void sendMsg(void){
  createCANmsg();
  
  //build CAN message
  canMsgTx.can_id  = 0x38;
  canMsgTx.can_dlc = 3;
  canMsgTx.data[0] = 0x01;
  canMsgTx.data[1] = canBattGenApu;
  canMsgTx.data[2] = canEngO2wings;

  //send message
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
    }
    Serial.println(); 
  }
}

void readSwitches(void){

  //remember previous value and read buttons 
  battOnLast = battOnCmd;
  battOnCmd = digitalRead(btn_batt_on);

  battOrideLast = battOrideCmd;
  battOrideCmd = digitalRead(btn_batt_oride);

  leftGenNormLast = leftGenNormCmd;
  leftGenNormCmd = digitalRead(btn_left_gen_norm);

  rightGenNormLast = rightGenNormCmd;
  rightGenNormCmd = digitalRead(btn_right_gen_norm);

  apuOnLast = apuOnCmd;
  apuOnCmd = digitalRead(btn_apu_on);
  
  leftEngCrankLast = leftEngCrankCmd;
  leftEngCrankCmd = digitalRead(btn_l_eng_crank);

  rightEngCrankLast = rightEngCrankCmd;
  rightEngCrankCmd = digitalRead(btn_r_eng_crank);

  obogsOnLast = obogsOnCmd;
  obogsOnCmd = digitalRead(btn_obogs_on);

  wingFoldLast = wingFoldCmd;
  wingFoldCmd = digitalRead(btn_wing_fold);

  wingSpreadLast = wingSpreadCmd;
  wingSpreadCmd = digitalRead(btn_wing_spread);
}

void createCANmsg(void){
  //bitpack canBattGenApu byte for CAN message
  bitWrite(canBattGenApu, 0, battOnCmd);
  bitWrite(canBattGenApu, 1, battOrideCmd);
  bitWrite(canBattGenApu, 2, leftGenNormCmd);
  bitWrite(canBattGenApu, 3, rightGenNormCmd);
  bitWrite(canBattGenApu, 4, apuOnCmd);

  //bitpack canEngO2wings byte for CAN message
  bitWrite(canEngO2wings, 0, leftEngCrankCmd);
  bitWrite(canEngO2wings, 1, rightEngCrankCmd);
  bitWrite(canEngO2wings, 2, obogsOnCmd);
  bitWrite(canEngO2wings, 3, wingFoldCmd);
  bitWrite(canEngO2wings, 4, wingSpreadCmd);
}
