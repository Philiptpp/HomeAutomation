/*----------------------------------------------------------------------*
 *----------------------------------------------------------------------*/ 

#include <HomeAutomation.h>

#ifndef TYPE
	#define TYPE 0b0000
#endif
#ifndef ADDRESS
	#define ADDRESS 0b0000
#endif

/*----------------------------------------------------------------------*
 * Constructor.                                                         *
 *----------------------------------------------------------------------*/
HomeAutomation::HomeAutomation(uint8_t address, uint8_t Rx_Pin, uint8_t Tx_Pin)
    : driver(2000, Rx_Pin, Tx_Pin, PTT_PIN, PTT_INVERTED)
    , manager(driver, address) {
    this->clientAddress = address;
    this->Rx = Rx_Pin;
    this->Tx = Tx_Pin;
}

/*----------------------------------------------------------------------*
 * Getters.                                                         *
 *----------------------------------------------------------------------*/
RHReliableDatagram HomeAutomation::getManager() {
    return this->manager;
}
  
/*----------------------------------------------------------------------*
 * Reads the current time from the RTC and returns it as a time_t       *
 * value. Returns a zero value if an I2C error occurred (e.g. RTC       *
 * not present).                                                        *
 *----------------------------------------------------------------------*/
bool HomeAutomation::safeTransmit(byte address, byte command, char *data) {
	uint8_t *transmitData;
	memcpy(transmitData, data, sizeof(data));
    if (getManager().sendtoWait(transmitData, sizeof(data), MASTER))
        return true;
    else
        return false;
}

bool HomeAutomation::transmit(byte address, byte command, char *data) {
	uint8_t *transmitData;
	memcpy(transmitData, data, sizeof(data));
    if (getManager().sendto(transmitData, sizeof(data), MASTER))
        return true;
    else
        return false;
}

bool HomeAutomation::dataReceived() {
    uint8_t buffer[RH_ASK_MAX_MESSAGE_LEN];
    
    if (getManager().available())
    {
        uint8_t len = sizeof(buffer);
        uint8_t from;
        if (getManager().recvfromAck(buffer, &len, &from)) {
            this->command = buffer[0];
            strncpy((char*)this->message, (char*)buffer, RH_ASK_MAX_MESSAGE_LEN);
			this->sender = from;
            return true;
        }
        else
            return false;
    } else {
        return false;
    }
}

uint8_t HomeAutomation::getCommand() {
    return this->command;
}

void HomeAutomation::getData(time_t& time) {
    uint8_t *data = this->message;
    tmElements_t tm;
    tm.Year = (bcd2dec(data[1]) * (2^8)) + bcd2dec(data[2]);
    tm.Month = bcd2dec(data[3]);
    tm.Day = bcd2dec(data[4]);
    tm.Hour = bcd2dec(data[5]);
    tm.Minute = bcd2dec(data[6]);
    time = makeTime(tm);
}

void HomeAutomation::getData(uint16_t& value) {
    uint8_t *data = this->message;
    value = (bcd2dec(data[1]) * (2^8)) + bcd2dec(data[2]);
}

uint8_t HomeAutomation::client() {
	return TYPE << 4 | ADDRESS;
}

uint8_t HomeAutomation::getSender() {
	return this->sender;
}

/*    Utility functions   */
/*
 *  Converts Binary Coded Decimal to Decimal
 *  @PARAM n - BCD value
 *  @RETURN Decimal value of corresponding BCD
 */
uint8_t bcd2dec(uint8_t n) {
  return n - 6 * (n >> 4);
}