/* CAN Ping Pong from Cory J Fowler's CAN Loopback Example
 *
 * This program displays all received messages and sends a message
 * once a second.
 * If a message is received, the next transmission is scheduled for 1/2
 * second later.  If used on 2 boards, they will then be in sync.
 *
 * This code is configured to run on the Feather CAN board
 *   
 *   Written By: Bob D'Avello 1/31/2019
 */

#include <mcp_can.h>
#include <SPI.h>

// CAN TX Variables
unsigned long prevTX = 0;                                        // Variable to store last execution time
const unsigned int invlTX = 1000;                                // One second interval constant
byte data[] = {0xAA, 0x55, 0x01, 0x10, 0xFF, 0x12, 0x34, 0x56};  // Generic CAN data to send

// CAN RX Variables
long unsigned int rxId;
unsigned char len;
unsigned char rxBuf[8];

// Serial Output String Buffer
char msgString[128];

// CAN0 INT and CS
#define CAN0_INT 30                              // Set INT to pin 30
MCP_CAN CAN0(8);                               // Set CS to pin 8

// MCP25625 RESET
#define TXD_RST 31

#define LED 13


void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 1);
  
  Serial.begin(115200);  // CAN is running at 500,000BPS; 115,200BPS is SLOW, not FAST, thus 9600 is crippling.
  
  // so we can see the startup messages
  while(!Serial) ;

  // CAN chip RESET line
  pinMode(TXD_RST, OUTPUT);
  
  // reset CAN chip
  digitalWrite(TXD_RST, 0);
  delay(100);
  digitalWrite(TXD_RST, 1);
  delay(500);  

  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  // Since we do not set NORMAL mode, we are in loopback mode by default.
  // Comment out next line for LOOPBACK MODE
  CAN0.setMode(MCP_NORMAL);

  pinMode(CAN0_INT, INPUT_PULLUP);                           // Configuring pin for /INT input
  
  Serial.println("MCP2515 Library Loopback Example...");

  digitalWrite(LED, 0);
}

void loop()
{
  if(!digitalRead(CAN0_INT))                          // If CAN0_INT pin is low, read receive buffer
  {
    // message received
    // flip LED
    // invert the LED
    int i = digitalRead(LED) ? 0 : 1;   // inline "~digitalRead(LED) returns -2!!!
    digitalWrite(LED, i);
    // and reset transmit time to 1/2
    prevTX = millis() - invlTX/2;
    
    CAN0.readMsgBuf(&rxId, &len, rxBuf);              // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)             // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){            // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
        
    Serial.println();
  }

  if(millis() - prevTX >= invlTX){                    // Send this at a one second interval. 
    prevTX = millis();
    byte sndStat = CAN0.sendMsgBuf(0x100, 8, data);
    // byte sndStat = CAN_OK;
    
    Serial.print("TEC: ");
    Serial.println(CAN0.errorCountTX());

    Serial.print("REC: ");
    Serial.println(CAN0.errorCountRX());
    
    if(sndStat == CAN_OK)
      Serial.println("Message Sent Successfully!");
    else
      Serial.println("Error Sending Message...");
    Serial.println();
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
