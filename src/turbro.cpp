#include <Arduino.h>
#include <PicoMQTT.h>

#include "turbro.h"

enum ACTIONS
{
  ACTION_OFF,
  ACTION_COOLING,
  ACTION_DRYING,
  ACTION_HEATING,
  ACTION_FAN
};

extern PicoMQTT::Client *mqtt;

const char *TURBRO::mode_lookup[] = {"off", "heat_cool", "cool", "dry", "fan_only", "heat"};
const char *TURBRO::fan_lookup[] = {"", "low", "medium", "high", "auto"};
const char *TURBRO::action_lookup[] = {"off", "cooling", "drying", "heating", "fan"};

int TURBRO::init()
{
  _port.begin(4800, SERIAL_8E1, _rx_pin, _tx_pin);
  while (_port.available())
  {
    _port.read(); // clear buffer
  }

  return 0;
}

int TURBRO::update()
{
  static uint32_t comm_millis = millis();
  static int evap_offs = 0;

  if (millis() - comm_millis < 1000)
  {
    return 0;
  }

  if (millis() - comm_millis > 2000)
  {
    comm_millis = millis();
  }
  else
  {
    comm_millis += 1000;
  }

  struct
  {
    uint8_t preamble[2] = {0xAA, 0x01};
    uint8_t dummy1 = 0;
    union
    {
      struct
      {
        uint8_t power : 1;
        uint8_t led : 1;
        uint8_t dummy : 5;
        uint8_t swing : 1;
      } b;
      uint8_t u = 0;
    } ctrl;
    uint8_t auto5 = 0;
    uint8_t dummy2[1] = {0};
    enum TURBRO_modes mode = MODE_OFF;
    enum TURBRO_fans fan = FAN_OFF;
    int8_t setpoint;
    uint8_t dummy3[9] = {0};
    uint8_t chksum;
    uint8_t postamble = 0x77;
  } tx_packet = {};

  struct
  {
    uint8_t preamble[2] = {0, 0};
    uint8_t dummy1 = 0;
    uint8_t comp_cmd;
    uint8_t comp_act;
    uint8_t cond_fan;
    uint8_t dummy2[2];
    uint8_t temp_cond_in;
    uint8_t temp_cond_out;
    uint8_t dummy3[2];
    uint8_t evap_fan;
    uint8_t rev_valve; //??
    uint8_t temp_evap_out;
    uint8_t temp_evap_in;
    uint8_t dummy4[2];
    uint8_t temp_y;
    uint8_t dummy5[5];
    uint8_t chksum = 0;
    uint8_t postamble = 0;
  } rx_packet = {};

  tx_packet.ctrl.b.power = (_mode != 0) ? true : false;
  tx_packet.auto5 = (_mode == MODE_AUTO) ? 0x05 : 0x00;
  tx_packet.mode = (typeof tx_packet.mode)_mode; // tx_packet.MODE_HEAT;
  tx_packet.fan = (typeof tx_packet.fan)_fan;    // tx_packet.FAN_AUTO;
  tx_packet.setpoint = _setpoint;

  uint8_t chksum = 0;
  for (int i = 2; i < sizeof(tx_packet) - 4; i++)
  {
    chksum += ((uint8_t *)&tx_packet)[i];
  }
  tx_packet.chksum = 0xFF - chksum;

  for (int i = 0; i < sizeof(rx_packet); i++)
  {
    ((uint8_t *)&rx_packet)[i] = _port.read();
  }
  while (_port.available())
  {
    _port.read(); // clear rest of buffer
  }

  chksum = 0;
  for (int i = 2; i < sizeof(rx_packet) - 4; i++)
  {
    chksum += ((uint8_t *)&rx_packet)[i];
  }
  chksum = 0xFF - chksum;

  _port.write((uint8_t *)&tx_packet, sizeof(tx_packet));

  //  auto rx_diag = mqtt->begin_publish(rx_topic.c_str(), sizeof(tx_packet) * 3);
  //  for (int i = 0; i < sizeof(tx_packet); i++)
  //  {
  // rx_diag.printf("%02x ", ((char *)&tx_packet)[i]);
  auto rx_diag = mqtt->begin_publish(rx_topic.c_str(), sizeof(rx_packet) * 3);
  for (int i = 0; i < sizeof(rx_packet); i++)
  {
    rx_diag.printf("%02x ", ((char *)&rx_packet)[i]);
  }
  rx_diag.send();

  _action = ACTION_OFF;
  _ac_temp = -100;
  _comp_rps = -1;

  // check if packet is valid
  if (rx_packet.preamble[0] == 0xAA && rx_packet.preamble[1] == 0x01 && chksum == rx_packet.chksum && rx_packet.postamble == 0x77 && rx_packet.temp_evap_in < 0x40 + 80)
  {
    _ac_temp = (float)rx_packet.temp_evap_in - 0x40;
    _comp_rps = rx_packet.comp_act;

    if (_mode == MODE_OFF || _mode == MODE_FAN) // calibrate evap in/out offset when off
    {
      evap_offs = rx_packet.temp_evap_in - rx_packet.temp_evap_out;
    }

    if (_mode == MODE_FAN)
    {
      _action = ACTION_FAN;
    }
    else if (_mode == MODE_AUTO)
    {
      static int rev_cnt = 0;
      static uint8_t last_rev_valve = 0;
      if (rx_packet.comp_cmd != 0)
      {
        if (rev_cnt == 30) // wait for condenser to reach temp
        {
          if (rx_packet.temp_evap_in < rx_packet.temp_evap_out - evap_offs)
          {
            _action = ACTION_HEATING;
          }
          else
          {
            _action = ACTION_COOLING;
          }
        }
        else
        {
          rev_cnt++;
        }
      }
      else
      {
        rev_cnt = 0;
        last_rev_valve = 0;
      }
    }
    else if (_mode == MODE_COOL && rx_packet.comp_cmd != 0)
    {
      _action = ACTION_COOLING;
    }
    else if (_mode == MODE_DRY && rx_packet.comp_cmd != 0)
    {
      _action = ACTION_DRYING;
    }
    else if (_mode == MODE_HEAT && rx_packet.comp_cmd != 0)
    {
      _action = ACTION_HEATING;
    }
    return TURBRO_OK;
  }
  return TURBRO_ERR;
}
