#include <Wire.h>
#include "I2CProtocol.h"
#include "ShiftToButtons.h"

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   1

int argsCnt = 0;                        // how many arguments were passed with given command
Commands::I2CCommand requestedCmd = 0;  // which command was requested (if any)

uint8_t i2cArgs[I2C_MSG_ARGS_MAX];      // array to store args received from master
int i2cArgsLen = 0;                     // how many args passed by master to given command

uint8_t i2cResponse[I2C_RESP_LEN_MAX];  // array to store response
int i2cResponseLen = 0;                 // response length

ShiftButtons buttons = ShiftButtons();

void loopI2C()
{
  switch(requestedCmd)
  {
    case Commands::I2C_CMD_GET_BUTTONS:
      i2cResponseLen = 0;
      i2cResponseLen++;
      i2cResponse[i2cResponseLen - 1] = buttons.pinValues >> 24 & 0xFF;
      i2cResponseLen++;
      i2cResponse[i2cResponseLen - 1] = buttons.pinValues >> 16 & 0xFF;
      i2cResponseLen++;
      i2cResponse[i2cResponseLen - 1] = buttons.pinValues >> 8 & 0xFF;
      i2cResponseLen++;
      i2cResponse[i2cResponseLen - 1] = buttons.pinValues & 0xFF;
      break;
    default:
      // do nothing.
      break;
  }
  requestedCmd = 0;
}

void requestEvent(){
  Wire.write(i2cResponse, i2cResponseLen);
}

void receiveEvent(int bytesReceived){
  int cmdRcvd = -1;
  int argIndex = -1; 
  argsCnt = 0;

  if (Wire.available()){
    cmdRcvd = Wire.read();                 // receive first byte - command assumed
    while(Wire.available()){               // receive rest of tramsmission from master assuming arguments to the command
      if (argIndex < I2C_MSG_ARGS_MAX){
        argIndex++;
        i2cArgs[argIndex] = Wire.read();
      }
      else{
        ; // implement logging error: "too many arguments"
      }
      argsCnt = argIndex+1;  
    }
  }
  else{
    // implement logging error: "empty request"
    return;
  }
  // validating command is supported by slave
  int fcnt = -1;
  for (int i = 0; i < sizeof(supportedI2Ccmd); i++) {
    if (supportedI2Ccmd[i] == cmdRcvd) {
      fcnt = i;
    }
  }

  if (fcnt<0){
    // implement logging error: "command not supported"
    return;
  }
  requestedCmd = cmdRcvd;
}  

void setup()
{
    Wire.begin(SLAVE_ADDRESS);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    Serial.begin(57600);

    Serial.println("Hello world");

    buttons.setup();
}

void loop()
{
    /* Read the state of all zones.
    */
    buttons.pinValues = buttons.read_shift_regs();

    /* If there was a chage in state, display which ones changed.
    */
    if(buttons.pinValues != buttons.oldPinValues)
    {
        Serial.print("*Pin value change detected*\r\n");
        buttons.display_pin_values();
        buttons.oldPinValues = buttons.pinValues;
    }
    loopI2C();
    delay(POLL_DELAY_MSEC);
}

