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

//unit consts
#define U_DEG 3
#undef EXP_MOVAVG_N
#define EXP_MOVAVG_N 20

AMS_AS5048B mysensor;


const double angleMin = 162.0;
const double angleMax = 208.0;

int count = 0;


void setup() {

  //Start serial
  Serial.begin(9600);
  while (!Serial) ; //wait until Serial ready

  //Start Wire object. Unneeded here as this is done (optionnaly) by AMS_AS5048B object (see lib code - #define USE_WIREBEGIN_ENABLED)
  //Wire.begin();

  //init AMS_AS5048B object
  mysensor.begin();

  //set clock wise counting
  mysensor.setClockWise(false); 

  //set the 0 to the sensorr
  //mysensor.zeroRegW(0x0);

}


void loop() {
  
  //prints to serial the read angle in degree and its average every 2 seconds
  //prints 2 times the exact same angle - only one measurement
  mysensor.updateMovingAvgExp();


  //Serial.print("Angle degree : ");
  //Serial.println(mysensor.angleR(U_DEG, false), DEC);

  //Serial.print("Average ");
  //Serial.println(mysensor.getMovingAvgExp(U_DEG), DEC);

  //double angle = mysensor.angleR(U_DEG, false);
  double angle = mysensor.getMovingAvgExp(U_DEG);;
  double scaled = (angle - angleMin) / (angleMax - angleMin);

  if (scaled > 1.0) scaled = 1.0;
  if (scaled < 0.0) scaled = 0.0;

  uint16_t tenBit = (uint16_t)(scaled * 1024.0);
  
  Joystick.X(tenBit);

  count++;
  if (count % 15 == 0)
  {
    Serial.print(angle);
    Serial.print(" ");
    Serial.print(scaled);
    Serial.print(" ");
    Serial.print(tenBit);
    Serial.println();
  }

  Wire.beginTransmission(0x35);
  Wire.write((uint8_t)1);
  Wire.endTransmission();

  delay(1);

  uint8_t respVals[4];
  
  Wire.requestFrom(0x35, 4);
  for (byte r = 0; r < 4; r++)
  {
    if(Wire.available()){ 
      respVals[r] = (uint8_t)Wire.read();
    }
    else{
      // log or handle error: "missing read"; if you are not going to do so use r index instead of respIoIndex and delete respoIoIndex from this for loop
      break;
    }
  }

  for (int i = 0; i < 4; i ++)
  {
    Serial.print("switch bytes: ");  
    Serial.print(respVals[i], 2);
    Serial.print(" ");
  }
  Serial.println();
  
  delay(1);
}
