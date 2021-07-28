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

#define SERIAL_DEBUG

//unit consts
#define U_DEG 3
#define DAMPER_PWM_PIN 6 

#include <ams_as5048b.h>
#include <Adafruit_MCP23017.h> 
// Note: download the latest from https://github.com/ppedro74/Arduino-SerialCommands and install in Documents/Arduino/libraries.
// The latest changes fix warnings in the version available through the Arduino Library Manager.
#include <SerialCommands.h>


AMS_AS5048B _collective_angle;
// IO expander
Adafruit_MCP23017 _io;

// Angle limits for collective arm.
const double _angle_min = 197.5;
double _angle_range = 30;
double _angle_max = _angle_min + _angle_range;

// Keep track of button press/releases by tracking the switch state.
uint16_t prev_switch_state = 0;

// Buffer for serial command library.
char _serial_command_buffer[32];

// Current setting for brake PWM.
uint8_t _brake_pwm = 192;

// PWM frequency for brake/damper control.
const float BRAKE_PWM_FREQ = 100;

// Tracks time since collective arm has moved.
elapsedMillis _brake_timeout;
// The collective position to compare to for brake timeout.
int16_t _brake_timeout_angle = 0;
// The amount the collective must move before we reset the brake timeout.
uint16_t _brake_timeout_tolerance = 10;
// Brake will disable if position does not change for this many millis.
uint32_t _brake_timeout_millis = 1000 * 15;
// Is the brake enabled?
bool _is_brake_enabled = false;

// Store the collective arm position.
int16_t _collective_arm_angle = 0;

// Debug flags
bool _debug_angle = true;
bool _debug_buttons = true;

SerialCommands _serial_commands(&Serial, _serial_command_buffer, sizeof(_serial_command_buffer), "\n", " ");

// This is the default handler, and gets called when no other command matches. 
// Note: It does not get called for one_key commands that do not match
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}


// Sets the resistance of the collective brake when the brake is set.
void cmd_resistance_brake_on(SerialCommands* sender)
{
  // Note: Every call to Next moves the pointer to next parameter
  char* pwm_str = sender->Next();
  if (pwm_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_PWM");
    return;
  }
  int brake_pwm = atoi(pwm_str);
  set_brake_pwm(brake_pwm);
  sender->GetSerial()->print("New brake PWM: ");
  sender->GetSerial()->println(brake_pwm);
}

// Sets the range of angles for the collective arm.
void cmd_angle_range(SerialCommands* sender)
{
  // Note: Every call to Next moves the pointer to next parameter
  char* pwm_str = sender->Next();
  if (pwm_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_PWM");
    return;
  }
  float angle_range = atof(pwm_str);
  set_angle_range(angle_range);
  sender->GetSerial()->print("New angle range: ");
  sender->GetSerial()->println(angle_range);
}

void _update_brake_pwm()
{
  if (_is_brake_enabled)
  {
    analogWrite(DAMPER_PWM_PIN, _brake_pwm);
  } else {
    analogWrite(DAMPER_PWM_PIN, 0);
  }
}

// Update applied brake PWM.
void set_brake_pwm(uint8_t brake_pwm)
{
  _brake_pwm = brake_pwm;
  _update_brake_pwm();
}

// Update angle range for collective arm.
void set_angle_range(double new_range)
{
  _angle_range = new_range;
  _angle_max = _angle_min + _angle_range;
}

void set_brake_enable(bool is_enabled)
{
  if (is_enabled != _is_brake_enabled)
  {
    _is_brake_enabled = is_enabled;
    Serial.print("Brake enable? ");
    Serial.print(_is_brake_enabled);
    Serial.println();
    _update_brake_pwm();
  }
}

SerialCommand _cmd_brake("BRAKE", cmd_resistance_brake_on);
SerialCommand _cmd_angle_range("RANGE", cmd_angle_range);

void setup() {

  //Start serial
  Serial.begin(9600);
  while (!Serial) ; //wait until Serial ready

  //Start Wire object. Unneeded here as this is done (optionally) by AMS_AS5048B object (see lib code - #define USE_WIREBEGIN_ENABLED)
  Wire.begin();

  setup_as5048b();

  setup_io();

  analogWriteFrequency(DAMPER_PWM_PIN, BRAKE_PWM_FREQ);
  set_brake_pwm(192);
  set_angle_range(30);

  _serial_commands.SetDefaultHandler(cmd_unrecognized);
  _serial_commands.AddCommand(&_cmd_brake);
  _serial_commands.AddCommand(&_cmd_angle_range);
}

void setup_as5048b()
{
  //init AMS_AS5048B object
  _collective_angle.begin();

  //set clock wise counting
  _collective_angle.setClockWise(false);
}

// set up the IO expander..
void setup_io()
{  
  _io.begin();
  for (int i = 0; i < 16; i++)
  {
    _io.pullUp(i, HIGH);
  }
}

uint16_t sample_as5048b()
{
  _collective_angle.updateMovingAvgExp();

  double angle = _collective_angle.angleR(U_DEG, false);
  double scaled = (angle - _angle_min) / (_angle_max - _angle_min);
  if (scaled > 1.0) scaled = 1.0;
  if (scaled < 0.0) scaled = 0.0;

  uint16_t ten_bit = (uint16_t)(scaled * 1024.0);
  if (_debug_angle)
  {
    static uint16_t count;
    count++;
    if (count % 100 == 0)
    {
      Serial.print(angle);
      Serial.print("\t");
      Serial.print(scaled);
      Serial.print("\t");
      Serial.print(ten_bit);
      Serial.print("\t");
      Serial.print(_collective_angle.getAutoGain());
      Serial.print("\t");
      Serial.print(_collective_angle.getDiagReg(),BIN);
      Serial.print("\t");
      Serial.print(_collective_angle.magnitudeR());
      Serial.println();
    }
  }

  return ten_bit;
}

void update_buttons()
{
  uint16_t switch_state = _io.readGPIOAB();
  uint16_t changes = switch_state ^ prev_switch_state;
  if (changes != 0)
  {
    for (int i = 0; i < 16; i++)
    {
      int mask = 1 << i;
      if ( ( changes & mask ) != 0 )
      {
        int new_state = (switch_state & mask) == 0;
        // Joystick buttons are indexed from 1, so add 1.
        Joystick.button(i + 1, new_state);
        if (_debug_buttons)
        {
          Serial.print("\t");
          Serial.print(i);
          Serial.print(" ");
          Serial.print(new_state ? "down" : "up");
          Serial.println();
        }
      }
    }   
  }
  prev_switch_state = switch_state;
}

void update_brake_timeout()
{
  if (abs(_collective_arm_angle - _brake_timeout_angle) > _brake_timeout_tolerance)
  {
    _brake_timeout_angle = _collective_arm_angle;
    _brake_timeout = 0;
    set_brake_enable(true);
  } 
  else if (_brake_timeout > _brake_timeout_millis)
  {
    set_brake_enable(false);
  }
}

void loop() 
{
  _serial_commands.ReadSerial();
  _collective_arm_angle = sample_as5048b();
  update_brake_timeout();
  Joystick.X(_collective_arm_angle);
  update_buttons();
  delay(10);
}
