#pragma once

#include <Wire.h>

#define SLAVE_ADDRESS   0x35


// I2C protocol example based on https://stackoverflow.com/questions/21073085
namespace Commands
{
  enum I2CCommand {
    I2C_CMD_GET_BUTTONS = 1
  };
}

enum { 
  I2C_MSG_ARGS_MAX = 32,
  I2C_RESP_LEN_MAX = 32
};

extern const byte supportedI2Ccmd[] = { 
  Commands::I2C_CMD_GET_BUTTONS
};
