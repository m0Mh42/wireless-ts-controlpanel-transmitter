#ifndef _H_RADIO
#define _H_RADIO

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "header.h"

bool _radio_sendpacket(RF24 *_radio, transaction_unit *_transaction_unit);
void radio_setup(RF24 *_radio);
void radio_transact(RF24 *_radio, transaction_unit *_transaction_unit);
void radio_read(RF24 *_radio, transaction_unit *_transaction_unit);

#endif
