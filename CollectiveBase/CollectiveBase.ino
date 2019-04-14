/**************************************************************************/
/*!
    @file     angle_moving_avg.ino
    @author   SOSAndroid (E. Ha.)
    @license  BSD (see license.txt)

  read over I2C bus and averaging angle

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/

#include <ams_as5048b.h>
#include <Adafruit_MCP23017.h> 

//unit consts
#define U_DEG 3
#undef EXP_MOVAVG_N
#define EXP_MOVAVG_N 20

AMS_AS5048B mysensor;
Adafruit_MCP23017 io;

const double angleMin = 162.0;
const double angleMax = 208.0;

uint16_t prev_switch_state = 0;


void setup() {

  //Start serial
  Serial.begin(9600);
  while (!Serial) ; //wait until Serial ready

  //Start Wire object. Unneeded here as this is done (optionally) by AMS_AS5048B object (see lib code - #define USE_WIREBEGIN_ENABLED)
  Wire.begin();

  //SetupAMS5048B();

  SetupIO();
}

void SetupAMS5048B()
{
  //init AMS_AS5048B object
  mysensor.begin();

  //set clock wise counting
  mysensor.setClockWise(false);

  //set the 0 to the sensorr
  //mysensor.zeroRegW(0x0);
}

void SetupIO()
{  
  // set up the IO expander..
  io.begin();
  for (int i = 0; i < 16; i++)
  {
    io.pullUp(i, HIGH);
  }
}

uint16_t SampleAMS5048B()
{
  static uint16_t count;
    //prints to serial the read angle in degree and its average every 2 seconds
  //prints 2 times the exact same angle - only one measurement
  mysensor.updateMovingAvgExp();

  //double angle = mysensor.angleR(U_DEG, false);
  double angle = mysensor.getMovingAvgExp(U_DEG);;
  double scaled = (angle - angleMin) / (angleMax - angleMin);

  if (scaled > 1.0) scaled = 1.0;
  if (scaled < 0.0) scaled = 0.0;

  uint16_t ten_bit = (uint16_t)(scaled * 1024.0);
  count++;
  if (count % 50 == 0)
  {
    Serial.print(angle);
    Serial.print(" ");
    Serial.print(scaled);
    Serial.print(" ");
    Serial.print(ten_bit);
    Serial.println();
  }

  return ten_bit;
}

void UpdateButtons()
{
  uint16_t switch_state = io.readGPIOAB();
  uint16_t changes = switch_state ^ prev_switch_state;
  if (changes != 0)
  {
    Serial.print( switch_state, 2 );
    Serial.print("\t");
    Serial.print( changes,2 );
    Serial.println();
    for (int i = 0; i < 16; i++)
    {
      int mask = 1 << i;
      if ( ( changes & mask ) != 0 )
      {
        int new_state = (switch_state & mask) == 0;
        Joystick.button(i + 1, new_state);
        Serial.print("\t");
        Serial.print(i);
        Serial.print(" ");
        Serial.print(new_state ? "down" : "up");
        Serial.println();
      }
      
    }   
  }
  prev_switch_state = switch_state;
  //Serial.print(switchState, 2);
  //Serial.println();
}

void loop() 
{
  //Joystick.X(SampleAMS5048B());
  UpdateButtons();
  delay(100);
}
