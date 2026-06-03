#include "radio.h"

void radio_setup(RF24 *_radio)
{
    _radio->begin();

    // Write to master receiver address (RX0), listen on our own address (TS1)
    _radio->openWritingPipe(addresses[DEVICE_MASTER]);
    _radio->openReadingPipe(1, addresses[DEVICE_TS_SLAVE]);

    _radio->setPALevel(RF24_PA_MAX);
    _radio->setChannel(CHANNEL);
    _radio->setAutoAck(true);
    _radio->setRetries(5, 15);
    _radio->setDataRate(RF24_250KBPS);
    _radio->setPayloadSize(sizeof(transaction_unit));

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

bool radio_transact(RF24 *_radio, transaction_unit *_transaction_unit)
{
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

bool radio_read(RF24 *_radio, transaction_unit *_transaction_unit)
{
    if (_radio->available())
    {
        _radio->read(_transaction_unit, sizeof(transaction_unit));
        return true;
    }
    return false;
}