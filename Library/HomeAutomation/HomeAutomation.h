/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/ 

#ifndef HomeAutomation_h
    #define HomeAutomation_h
#endif

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WConstants.h"
#endif
#ifndef RH_ASK_h
    #include <RH_ASK.h>             //http://www.airspayce.com/mikem/arduino/RadioHead/index.html
#endif
#ifndef RHReliableDatagram_h
    #include <RHReliableDatagram.h>
#endif
#ifndef Time_h
    #include <Time.h>
#endif

/* ------------------------------------------------ *
 * Define device types                              *
 * Address byte consists of high & low nibble       *
 * High = Device type                               *
 * Low  = Unique address                            *
 *                                                  *
 * 0xxx for input devices like temperature sensor   *
 * 1xxx for output devices like switches            *
 * ------------------------------------------------ */
#define SENSOR              0b0000 << 4
#define SENSOR_TEMP         0b0001 << 4
#define SENSOR_HUMIDITY     0b0010 << 4
#define SENSOR_MOTION       0b0011 << 4
#define SENSOR_WATER_LEVEL  0b0100 << 4
#define SENSOR_LIGHT        0b0101 << 4

#define SWITCH              0b1000 << 4
#define SWITCH_TIMED        0b1001 << 4
#define SWITCH_TRIGGERED    0b1010 << 4
#define SWITCH_RF           0b1011 << 4

#define MAIN_HUB            0b1111 << 4

/* ------------------------------------------------ *
 * Define commands                                  *
 * Command consists of 8 bits (1 byte)              *
 * ==MSB==                                          *
 * bit 1   => Additional data present (1=yes,0=no)  *
 * bit 2   => 1 = Master->slave, 0 = Slave->master  *
 * bit 3-8 => Command code                          *
 * ==LSB==                                          *
 * ------------------------------------------------ */
#define NO_DATA             0b0 << 7
#define DATA                0b1 << 7
#define M2S                 0b1 << 6
#define S2M                 0b0 << 6

#define SWITCH_OFF          0b000000
#define SWITCH_ON           0b000001
#define SWITCH_TOGGLE       0b000010
#define SWITCH_STATUS       0b000011

#define TEMPERATURE         0b000100
#define HUMIDITY            0b000101

#define MOTION_STATUS       0b001000
#define MOTION_STARTED      0b001001
#define MOTION_ENDED        0b001011

#define GET_TIME            0b010000
#define SET_TIME            0b010001
#define ALARM1_SET          0b010010
#define ALARM1_GET          0b010011
#define ALARM2_SET          0b010110
#define ALARM2_GET          0b010111
#define WATER_LEVEL         0b011000

#define ACKNOWLEDGE         0b110110
#define COMMAND_FAILED      0b100100

/* ------------------------------------------------ *
 * Common definitions                               *
 * ------------------------------------------------ */
#define SPEED 2000
#define PTT_PIN -1
#define PTT_INVERTED true
#define MASTER 0b11110000  // Address = 11110000
#define RH_ASK_MAX_MESSAGE_LEN 10

/* ------------------------------------------------ */
class HomeAutomation
{
    public:
        // Constructor
        HomeAutomation(uint8_t address, uint8_t Rx_Pin = -1, uint8_t Tx_Pin = -1);
        // Custom methods
        bool    safeTransmit(byte address, byte command, char *data = "");
        bool    transmit(byte address, byte command, char *data = "");
        bool    dataReceived();
        uint8_t getCommand();
        void    getData(time_t& time);
        void    getData(uint16_t& value);
        uint8_t client();
		uint8_t getSender();
		
    private:
        uint8_t Rx;
        uint8_t Tx;
        uint8_t clientAddress;
        uint8_t command;
        uint8_t message[RH_ASK_MAX_MESSAGE_LEN];
		uint8_t sender;
        RH_ASK driver;
        RHReliableDatagram manager;
        RHReliableDatagram getManager();
};

// Utility functions
uint8_t bcd2dec(uint8_t n);