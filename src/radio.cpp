#include "radio.h"

bool _radio_sendpacket(RF24 *_radio, transaction_unit *_transaction_unit)
{
    // TODO Radio Transmition Error Handling
    const int max_retries = 10;
    for (int retry = 0; retry < max_retries; retry++)
    {
        bool radio_sent = _radio->write(_transaction_unit, sizeof(transaction_unit));
        if (radio_sent)
        {
            return true;
        }
        delay(10); // Small delay between retries
    }
    return false; // Failed after retries
}

void radio_setup(RF24 *_radio)
{
    _radio->begin();
    _radio->openWritingPipe(addresses[DEVICE_TS_SLAVE]);    // TS 1
    _radio->openReadingPipe(1, addresses[DEVICE_TS_SLAVE]); // Master Reciever
    _radio->setPALevel(RF24_PA_MAX);
    _radio->setChannel(CHANNEL);

    // Defected Chip Error
    if (!_radio->isChipConnected())
    {
        while (true)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(250);
            digitalWrite(LED_BUILTIN, LOW);
            delay(250);
        }
    }
}

void radio_transact(RF24 *_radio, transaction_unit *_transaction_unit)
{
    _radio_sendpacket(_radio, _transaction_unit);
}

void radio_read(RF24 *_radio, transaction_unit *_transaction_unit)
{
}