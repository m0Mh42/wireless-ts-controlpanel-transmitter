#include "radio.h"
#include "error.h"
#include "display.h"
#include <RF24.h>

static RF24 s_hardware_radio(CE_PIN, CSN_PIN);
static RF24 *s_active_radio = &s_hardware_radio;

static uint8_t current_channel = CHANNEL;
static uint8_t current_lqi = 100;
static uint16_t lqi_scaled = 10000;
static RadioTxDiagnostics diag = {0, 0, 0, 0, 0, 0, false, false, 1, 1};

void radio_setup(RF24 *_radio)
{
    if (_radio != NULL)
    {
        s_active_radio = _radio;
    }
    else
    {
        s_active_radio = &s_hardware_radio;
    }

    pinMode(CSN_PIN, OUTPUT);
    digitalWrite(CSN_PIN, HIGH);
    pinMode(CE_PIN, OUTPUT);
    digitalWrite(CE_PIN, LOW);

    delay(15); // Silicon Power-On Reset settling delay

    s_active_radio->begin();
    s_active_radio->setAddressWidth(5);
    s_active_radio->setPALevel(RF24_PA_LEVEL_DEFAULT);
    s_active_radio->setCRCLength(RF24_CRC_16);
    s_active_radio->setDataRate(RF24_DATARATE_DEFAULT);
    s_active_radio->setChannel(current_channel);

    // Hardware Auto-ACK and Dynamic Payloads for ACK payloads
    s_active_radio->enableDynamicPayloads();
    s_active_radio->enableAckPayload();
    s_active_radio->setRetries(5, 15); // 1500 us retry delay, up to 15 attempts

    s_active_radio->openWritingPipe(RF24_PIPE_CONTROL);
    s_active_radio->stopListening();

    if (!s_active_radio->isChipConnected())
    {
        diag.last_error_code = 1;
        error_set(ERROR_RADIO_NOT_CONNECTED);
        display_show_radio_error();
    }
    else
    {
        error_clear(ERROR_RADIO_NOT_CONNECTED);
    }
}

void radio_update_lqi(bool success)
{
    if (success)
    {
        lqi_scaled = (uint16_t)(((uint32_t)lqi_scaled * 95 + 50000UL + 99) / 100);
        if (lqi_scaled > 10000)
        {
            lqi_scaled = 10000;
        }
    }
    else
    {
        lqi_scaled = (uint16_t)(((uint32_t)lqi_scaled * 95) / 100);
    }
    current_lqi = (uint8_t)((lqi_scaled + 50) / 100);
}

uint8_t radio_get_lqi()
{
    return current_lqi;
}

bool radio_send_control(const ControlPacket &packet, TelemetryPacket *out_telemetry, bool *out_telemetry_received, RF24 *_radio)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    diag.tx_attempt_count++;
    if (out_telemetry_received != NULL)
    {
        *out_telemetry_received = false;
    }

    bool sent = r->write(&packet, sizeof(ControlPacket));
    if (!sent)
    {
        r->flush_tx();
        diag.tx_fail_count++;
        diag.last_tx_ok = false;
        diag.last_error_code = 2; // TX_FAILED
        radio_update_lqi(false);
        return false;
    }

    diag.tx_success_count++;
    diag.last_tx_ok = true;
    diag.last_error_code = 0;
    radio_update_lqi(true);

    if (r->isAckPayloadAvailable())
    {
        TelemetryPacket tel;
        r->read(&tel, sizeof(TelemetryPacket));
        diag.ack_payload_count++;
        diag.last_ack_payload_ok = true;

        if (out_telemetry != NULL)
        {
            *out_telemetry = tel;
        }
        if (out_telemetry_received != NULL)
        {
            *out_telemetry_received = true;
        }
    }

    return true;
}

bool radio_send_control(const ControlPacket &packet, TelemetryPacket *out_telemetry, RF24 *_radio)
{
    return radio_send_control(packet, out_telemetry, NULL, _radio);
}

bool radio_send_safety_frame_and_read_ack(RF24 *_radio, uint16_t buttons, uint16_t battery_mv, ts_status status, uint32_t seq, uint8_t *out_active_unit)
{
    ControlPacket pkt;
    pkt.buttons = buttons;
    pkt.buttons_inv = (uint16_t)(~buttons);
    pkt.battery_mv = battery_mv;
    pkt.status = (uint8_t)status;
    pkt.seq = (uint8_t)(seq & 0xFF);

    TelemetryPacket tel = {0, 0, 0, 0};
    bool tel_received = false;
    bool ok = radio_send_control(pkt, &tel, &tel_received, _radio);
    if (ok && tel_received && out_active_unit != NULL)
    {
        *out_active_unit = tel.active_unit;
    }
    return ok;
}

bool radio_set_channel(RF24 *_radio, uint8_t channel)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    current_channel = channel;
    r->setChannel(current_channel);
    return true;
}

uint8_t radio_get_channel()
{
    return current_channel;
}

void radio_hop_next_channel(RF24 *_radio)
{
    uint8_t next = (current_channel == 78) ? 88 : 78;
    radio_set_channel(_radio, next);
}

void radio_recover_channel(RF24 *_radio)
{
    radio_set_channel(_radio, CHANNEL);
}

bool radio_is_chip_connected(RF24 *_radio)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    return r->isChipConnected();
}

const RadioTxDiagnostics &radio_get_diagnostics()
{
    return diag;
}

void radio_reset_diagnostics()
{
    diag.tx_attempt_count = 0;
    diag.tx_success_count = 0;
    diag.tx_fail_count = 0;
    diag.ack_payload_count = 0;
    diag.ack_payload_err_count = 0;
    diag.last_error_code = 0;
    diag.last_tx_ok = false;
    diag.last_ack_payload_ok = false;
    current_lqi = 100;
    lqi_scaled = 10000;
}

void radio_set_mode_tx(RF24 *_radio)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    r->stopListening();
}

void radio_set_mode_rx(RF24 *_radio)
{
    (void)_radio;
}

bool radio_is_tx_mode()
{
    return true;
}
