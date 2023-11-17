#include <SPI.h>             // SPI library for comm to LCD and CAN interface
#include <mcp2515.h>         // Library for MCP2515 CAN BUS

#define DEBUG   0
#define SPI_CS  10

//pins for switches
#define btn_canopy_open         9
#define btn_canopy_close        8
#define btn_launch_bar_retract  7
#define btn_launch_bar_extend   6
#define btn_ldgtaxi_lights      5
#define btn_anti_skid           4
#define btn_hook_bypass         3
#define btn_parking_brake       1

//variables for switch inputs from LANDING Panel
uint8_t canopyOpenCmd = 1;
uint8_t canopyCloseCmd = 1;
uint8_t launchBarExtendCmd = 1;
uint8_t launchBarRetractCmd = 1;
uint8_t ldgTaxiLightCmd = 1;
uint8_t antiSkidCmd = 1;
uint8_t hookBypassCmd = 1;
uint8_t parkingBrakeCmd = 1;

uint8_t canopyOpenLast;
uint8_t canopyCloseLast;
uint8_t launchBarExtendLast;
uint8_t launchBarRetractLast;
uint8_t ldgTaxiLightLast;
uint8_t antiSkidLast;
uint8_t hookBypassLast;
uint8_t parkingBrakeLast;

//CAN messages and interface instance
struct can_frame canMsgRx;
struct can_frame canMsgTx;
MCP2515 mcp2515(SPI_CS);  

//variables for managing when to send CAN msg
long msgSendTime = 0;
const long msgSendInt = 100;
String myStr;

//variables for indicators to GEAR Panel
uint8_t noseGearIndicator = 1;
uint8_t leftGearIndicator = 1;
uint8_t rightGearIndicator = 1;
uint8_t halfFlapsIndicator = 1;
uint8_t fullFlapsIndicator = 1;
uint8_t hookIndicator = 1;
uint8_t canIndicators = 0xFF;

//variables for switch inputs from GEAR Panel
uint8_t noseGearCmd = 1;
uint8_t leftGearCmd = 1;
uint8_t rightGearCmd = 1;
uint8_t halfFlapsCmd = 1;
uint8_t fullFlapsCmd = 1;
uint8_t hookCmd = 1;

//variables for the switch inputs from the APU Batt Panel
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

void setup() { 
  //setup pins for switch inputs
  pinMode(btn_canopy_open, INPUT_PULLUP);
  pinMode(btn_canopy_close, INPUT_PULLUP);
  pinMode(btn_launch_bar_retract, INPUT_PULLUP);
  pinMode(btn_launch_bar_extend, INPUT_PULLUP);
  pinMode(btn_ldgtaxi_lights, INPUT_PULLUP);
  pinMode(btn_anti_skid, INPUT_PULLUP);
  pinMode(btn_hook_bypass, INPUT_PULLUP);
  pinMode(btn_parking_brake, INPUT_PULLUP);

  //setup CAN interface
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS);
  mcp2515.setNormalMode();

  if(DEBUG){
    Serial.begin(115200);
    Serial.println("------- CAN Read ----------");
    Serial.println("ID  DLC   DATA");    
  }

}

void loop() {
  //Receive CAN messages
  if (mcp2515.readMessage(&canMsgRx) == MCP2515::ERROR_OK) {
    if(DEBUG){
      Serial.print("RX: "); 
      Serial.print(canMsgRx.can_id, HEX); // print ID
      Serial.print(" "); 
      Serial.print(canMsgRx.can_dlc, HEX); // print DLC
      Serial.print(" ");
      
      for (int i = 0; i<canMsgRx.can_dlc; i++)  {  // print the data
        Serial.print(canMsgRx.data[i],HEX);
        Serial.print(" ");
        myStr += (char)canMsgRx.data[i];
      }
      Serial.print(" - Data: " + myStr);
      Serial.println();     
      myStr = ""; 
    }

    if(canMsgRx.can_id == 0x36){
      //parse message from GEAR_PANEL
      parseGearPanel();
    }else if( canMsgRx.can_id == 0x38){
      //parse message from APU_BATT_PANEL
      parseApuBattPanel();
    }
  }

  //check if its time to send message for indicators
  if(msgSendTime == 0){
    msgSendTime = millis();
    sendIndicatorMsg();
  }else if(millis() - msgSendTime >= msgSendInt){
    msgSendTime = millis();
    sendIndicatorMsg();
  }

}

void sendIndicatorMsg(void){
  //bitpack for indicator states into CAN byte
  bitWrite(canIndicators, 0, noseGearIndicator);
  bitWrite(canIndicators, 1, leftGearIndicator);
  bitWrite(canIndicators, 2, rightGearIndicator);
  bitWrite(canIndicators, 3, halfFlapsIndicator);
  bitWrite(canIndicators, 4, fullFlapsIndicator);
  bitWrite(canIndicators, 5, hookIndicator);

  //define CAN message
  canMsgTx.can_id  = 0x35;
  canMsgTx.can_dlc = 2;
  canMsgTx.data[0] = 0x80;
  canMsgTx.data[1] = canIndicators;   

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
  canopyOpenLast = canopyOpenCmd;
  canopyOpenCmd = digitalRead(btn_canopy_open);

  canopyCloseLast = canopyCloseCmd;
  canopyCloseCmd = digitalRead(btn_canopy_close);

  launchBarExtendLast = launchBarExtendCmd;
  launchBarExtendCmd = digitalRead(btn_launch_bar_retract);

  launchBarRetractLast = launchBarRetractCmd;
  launchBarRetractCmd = digitalRead(btn_launch_bar_extend);

  ldgTaxiLightLast = ldgTaxiLightCmd;
  ldgTaxiLightCmd = digitalRead(btn_ldgtaxi_lights);
  
  antiSkidLast = antiSkidCmd;
  antiSkidCmd = digitalRead(btn_anti_skid);

  hookBypassLast = hookBypassCmd;
  hookBypassCmd = digitalRead(btn_hook_bypass);

  parkingBrakeLast = parkingBrakeCmd;
  parkingBrakeCmd = digitalRead(btn_parking_brake);

}

//Function that parses the CAN message from GEAR Panel for switch states
void parseGearPanel(void){
  //un-bitpack message from Gear Panel
  noseGearCmd = bitRead(canMsgRx.data[1], 0);
  leftGearCmd = bitRead(canMsgRx.data[1], 0);
  rightGearCmd = bitRead(canMsgRx.data[1], 0);
  halfFlapsCmd = bitRead(canMsgRx.data[1], 1);
  fullFlapsCmd = bitRead(canMsgRx.data[1], 2);
  hookCmd = bitRead(canMsgRx.data[1], 3);
  
  //temporarily pass switch command state back to GEAR panel as indicator status
  //This feedback will be replace with feedback from DCS
  noseGearIndicator = noseGearCmd;
  leftGearIndicator = leftGearCmd;
  rightGearIndicator = rightGearCmd;
  halfFlapsIndicator = halfFlapsCmd;
  fullFlapsIndicator = fullFlapsCmd;
  hookIndicator = hookCmd;
}

//Function that parses the CAN message from APU Batt Panel for switch states
void parseApuBattPanel(void){
  //un-bitpack byte 1 from APU Batt Panel
  battOnCmd = bitRead(canMsgRx.data[1], 0);
  battOrideCmd = bitRead(canMsgRx.data[1], 1);
  leftGenNormCmd = bitRead(canMsgRx.data[1], 2);
  rightGenNormCmd = bitRead(canMsgRx.data[1], 3);
  apuOnCmd = bitRead(canMsgRx.data[1], 4);

  //un-bitpack byte 2 from APU Batt Panel
  leftEngCrankCmd = bitRead(canMsgRx.data[2], 0);
  rightEngCrankCmd = bitRead(canMsgRx.data[2], 1);
  obogsOnCmd = bitRead(canMsgRx.data[2], 2);
  wingFoldCmd = bitRead(canMsgRx.data[2], 3);
  wingSpreadCmd = bitRead(canMsgRx.data[2], 4);
}
