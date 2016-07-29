/*
 * Program to control porch light activated by
 * alarms 1 & 2 from a RTC or controlled manually
 * over a RF remote.
 * Major Components:
 *   ATTiny85                   x1
 *   DS3231 RTC                 x1
 *   RF Transmitter @433MHz     x1
 *   RF Receiver @433MHz        x1
 *   5V Relay                   x1
 */

#include <HomeAutomation.h>
#include <DS3232RTC.h>          //http://github.com/JChristensen/DS3232RTC
#include <Time.h>               //http://www.arduino.cc/playground/Code/Time
#include <TimeLib.h>

#define TYPE      SWITCH_TRIGGERED
#define ADDRESS   0b0001

/* GENERAL DEFINITIONS */
#define DEFAULT_ON_TIME 5   // minutes
#define ON 1
#define OFF 0

/* DEFINE PINS FOR PHYSICAL CONNECTION TO MCU */
// I2C PINS FOR RTC
#define I2C_SCL 2
#define I2C_SDA 0

// Rx & Tx PINS FOR RF TRANSMISSION @433MHz
#define Rx 3
#define Tx 4

// RELAY PIN TO CONTROL OUTPUT
#define RELAY 1

/* Global variables */
bool LIGHT_STATE;
String TRIGGER;
time_t ON_TIME;
time_t ALARM1;
time_t ALARM2;
HomeAutomation HAT = HomeAutomation(TYPE <<4 | ADDRESS, Rx, Tx);    // Setup HomeAutomation library for RF transmission

/* Custom functions declarations for standard C++ practise */
bool CheckRTCAlarm1();
bool CheckRTCAlarm2();
bool CheckTimer();
void LightOn(String);
void LightOff(String);
void GetAlarm(byte);


/* Main function */
int main(void) {
  // Set pins as input / output
  // Set Relay & Tx as output pins
  DDRB = 0x00 | 1 << RELAY | 1 << Tx;

  // Setup RTC
  setSyncInterval(60);      // Sync time every 1 minute (60 seconds)
  setSyncProvider(RTC.get); // Auto sync time from RTC
  GetAlarm(ALARM_1);
  GetAlarm(ALARM_2);

  // Loop activity
  while (1) {
    // Check if RF command received
    for (uint8_t i = 0; i < 10; i++) {
      if ( HAT.dataReceived() ) {
        switch ( HAT.getCommand() ) {
          case SWITCH_ON:
            LightOn("MASTER");
            break;
          case SWITCH_OFF:
            LightOff("MASTER");
            break;
          case SWITCH_TOGGLE:
            if (LIGHT_STATE == OFF)
              LightOn("MASTER");
            else
              LightOff("MASTER");
            break;
          case SWITCH_STATUS:
            if (LIGHT_STATE == ON)
              HAT.safeTransmit(MASTER, NO_DATA | S2M | SWITCH_ON);
            if (LIGHT_STATE == OFF)
              HAT.safeTransmit(MASTER, NO_DATA | S2M | SWITCH_OFF);
            break;
          case GET_TIME:
            //HAT.safeTransmit(MASTER, DATA | S2M | GET_TIME, (uint16_t)year() << 4 | (uint8_t)month() << 3 | (uint8_t)day() << 2 | (uint8_t)hour() << 1 | (uint8_t)minute()); 
            break;
          case SET_TIME:
            break;
          case ALARM1_GET:
            //HAT.safeTransmit(MASTER, DATA | S2M | ALARM1_GET, (uint8_t)hour(ALARM1) << 1 | (uint8_t)minute(ALARM1)); 
            break;
          case ALARM1_SET:
            //char *data = HAT.getData();
            //RTC.setAlarm(ALARM_1, data[5], data[4], 0x00);
            break;
          case ALARM2_GET:
            //HAT.safeTransmit(MASTER, DATA | S2M | ALARM2_GET, (uint8_t)hour(ALARM2) << 1 | (uint8_t)minute(ALARM2)); 
            break;
          case ALARM2_SET:
            break;
          default:
            HAT.transmit(MASTER, DATA | S2M | COMMAND_FAILED, (char*)HAT.getCommand());
        }
      }
    }
     
    // If light is OFF, check alarm 2 and timer to turn OFF
    if (LIGHT_STATE == ON) {
      if (CheckRTCAlarm2() == HIGH) {
        LightOff("ALARM");
      } else if (CheckTimer() == HIGH) {
        LightOff("TIMER");
      }
    }
    
    // If light is OFF, check alarm 1 to turn ON
    if (LIGHT_STATE == OFF) {
      if (CheckRTCAlarm1() == HIGH)
        LightOn("ALARM");
    }
  }
  return 1;
}

/*
 * Check if alarm 1 is triggered
 * @PARAM
 * @RETURN bool - Alarm1 status HIGH / LOW
 */
bool CheckRTCAlarm1() {
  if ( hour(ALARM1) > hour() || (hour(ALARM1) == hour() && minute(ALARM1) >= minute()) ) {
    return HIGH;
  } else {
    return LOW;
  }
}

/*
 * Check if alarm 2 is triggered
 * @PARAM
 * @RETURN bool - Alarm2 status HIGH / LOW
 */
bool CheckRTCAlarm2() {
  if ( hour(ALARM2) > hour() || (hour(ALARM2) == hour() && minute(ALARM2) >= minute()) ) {
    return HIGH;
  } else {
    return LOW;
  }
}

/*
 * Check if timer has run out for default ON time
 * @PARAM
 * @RETURN bool - Timer status HIGH (expired) / LOW (running)
 */
bool CheckTimer() {
  if ( now() - ON_TIME >= DEFAULT_ON_TIME * 60) {
    return HIGH;
  } else {
    return LOW;
  }
}

/*
 * Sets the relay pin to high in order to turn the porch light ON
 * @PARAM Trigger - Event that triggered the light ON
 * @RETURN
 */
void LightOn(char *trigger) {
  if (LIGHT_STATE == ON)
    return;
  ON_TIME = now();
  LIGHT_STATE == ON;
  HAT.safeTransmit(MASTER, DATA | S2M | SWITCH_ON, trigger);
}

/*
 * Sets the relay pin to low in order to turn the porch light OFF
 * @PARAM Trigger - Event that triggered the light OFF
 * @RETURN
 */
void LightOff(char *trigger) {
  if (LIGHT_STATE == OFF)
    return;
  LIGHT_STATE == OFF;
  HAT.safeTransmit(MASTER, DATA | S2M | SWITCH_OFF, trigger);
}

/*
   Reads the RTC registry for alarm's minute & hour values
   @PARAM alarmByte - ALARM_1 or ALARM_2 to read
   @RETURN
*/
void GetAlarm(byte alarmByte) {
  byte alarmTime[2];
  tmElements_t t;
  if (alarmByte == ALARM_1) {
    RTC.readRTC(0x08, alarmTime, 2);  // Read Minute & Hour bytes from RTC register for Alarm1
    t.Second = 00;
    t.Minute = bcd2dec(alarmTime[0]);
    t.Hour = bcd2dec(alarmTime[1] & ~_BV(HR1224));
    ALARM1 = makeTime(t);
  } else {
    RTC.readRTC(0x0B, alarmTime, 2);  // Read Minute & Hour bytes from RTC register for Alarm1
    t.Second = 00;
    t.Minute = bcd2dec(alarmTime[0]);
    t.Hour = bcd2dec(alarmTime[1] & ~_BV(HR1224));
    ALARM2 = makeTime(t);
  }
}


