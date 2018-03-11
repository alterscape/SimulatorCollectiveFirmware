#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


/*
   SN74HC165N_shift_reg

   Program to shift in the bit values from a SN74HC165N 8-bit
   parallel-in/serial-out shift register.

   This sketch demonstrates reading in 16 digital states from a
   pair of daisy-chained SN74HC165N shift registers while using
   only 4 digital pins on the Arduino.

   You can daisy-chain these chips by connecting the serial-out
   (Q7 pin) on one shift register to the serial-in (Ds pin) of
   the other.

   Of course you can daisy chain as many as you like while still
   using only 4 Arduino pins (though you would have to process
   them 4 at a time into separate unsigned long variables).

*/

/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   2

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5


/* You will need to change the "int" to "long" If the
   NUMBER_OF_SHIFT_CHIPS is higher than 2.
*/
#define BYTES_VAL_T unsigned int

class ShiftButtons
{
  public:
    ShiftButtons();

    void setup(void);
    
    /* This function is essentially a "shift-in" routine reading the
       serial Data from the shift register chips and representing
       the state of those pins in an unsigned integer (or long).
    */
    BYTES_VAL_T read_shift_regs();

    /* Dump the list of zones along with their current status.
    */
    void display_pin_values();


    BYTES_VAL_T pinValues;
    BYTES_VAL_T oldPinValues;

  private:
    int ploadPin        = 10;  //8;  // Connects to Parallel load pin the 165
    int clockEnablePin  = 11; //9;  // Connects to Clock Enable pin the 165
    int dataPin         = 7;   //11; // Connects to the Q7 pin the 165
    int clockPin        = 8;  //12; // Connects to the Clock pin the 165

};
