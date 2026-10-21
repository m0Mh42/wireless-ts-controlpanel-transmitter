#ifndef _H_RADIO
#define _H_RADIO 1

#include <Arduino.h>
#include "header.h"
#include "protocol.h"

class RF24;

struct RadioTxDiagnostics
{
    uint32_t tx_attempt_count;
    uint32_t tx_success_count;
    uint32_t tx_fail_count;
    uint32_t ack_payload_count;
    uint32_t ack_payload_err_count;
    uint8_t last_error_code; // 0=NONE, 1=RADIO_DISCONNECTED, 2=TX_FAILED_NO_ACK, 3=BAD_LEN, 4=OTHER
    bool last_tx_ok;
    bool last_ack_payload_ok;
    uint8_t datarate_mode;
    uint8_t datarate_1mbps;
};

void radio_setup(RF24 *_radio = NULL);
bool radio_send_control(const ControlPacket &packet, TelemetryPacket *out_telemetry, bool *out_telemetry_received, RF24 *_radio = NULL);
bool radio_send_control(const ControlPacket &packet, TelemetryPacket *out_telemetry = NULL, RF24 *_radio = NULL);

bool radio_send_safety_frame_and_read_ack(RF24 *_radio, uint16_t buttons, uint16_t battery_mv, ts_status status, uint32_t seq, uint8_t *out_active_unit);
inline bool radio_send_safety_frame_and_read_ack(uint16_t buttons, uint16_t battery_mv, ts_status status, uint32_t seq, uint8_t *out_active_unit)
{
    return radio_send_safety_frame_and_read_ack(NULL, buttons, battery_mv, status, seq, out_active_unit);
}

bool radio_set_channel(RF24 *_radio, uint8_t channel);
inline bool radio_set_channel(uint8_t channel) { return radio_set_channel(NULL, channel); }
uint8_t radio_get_channel();

void radio_hop_next_channel(RF24 *_radio = NULL);
void radio_recover_channel(RF24 *_radio = NULL);

uint8_t radio_get_lqi();
void radio_update_lqi(bool success);
bool radio_is_chip_connected(RF24 *_radio = NULL);

void radio_set_mode_tx(RF24 *_radio = NULL);
void radio_set_mode_rx(RF24 *_radio = NULL);
bool radio_is_tx_mode();

const RadioTxDiagnostics &radio_get_diagnostics();
void radio_reset_diagnostics();

#endif // _H_RADIO
