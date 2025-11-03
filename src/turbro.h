#include <Arduino.h>
#include <HardwareSerial.h>

#define MAX_TEMP 35
#define MIN_TEMP 5

class TURBRO {
  public:
    enum TURBRO_retval : int8_t
    {
      RET_RXERR = -1,
      RET_NOOP = 0,
      RET_RXOK = 1,
      RET_TXRETRY = 2,
      RET_TX = 3,
    };

    enum TURBRO_modes : uint8_t
    {
      MODE_OFF = 0,
      MODE_AUTO = 1,
      MODE_COOL = 2,
      MODE_DRY = 3,
      MODE_FAN = 4,
      MODE_HEAT = 5,
    };
    
    enum TURBRO_fans : uint8_t
    {
      FAN_OFF = 0,
      FAN_LOW = 1,
      FAN_MED = 2,
      FAN_HIGH = 3,
      FAN_AUTO = 4,
    };
    
    enum ACTIONS
    {
      ACTION_OFF = 0,
      ACTION_COOLING = 1,
      ACTION_DRYING = 2,
      ACTION_HEATING = 3,
      ACTION_FAN = 4,
      //ACTION_ERR = 5,
    };

    TURBRO(HardwareSerial & port, int8_t rx_pin, int8_t tx_pin) : _port(port), _rx_pin(rx_pin), _tx_pin(tx_pin) {};

    int init();
    int update();
    int get_temp() { return _ac_temp; }
    int get_action() { return _action; }
    int get_comp_rps() { return _comp_rps; }
    int get_evap_fan() { return _evap_fan; }
    void set_parms(int mode, int fan, int setpoint) { _mode = mode; _fan = fan, _setpoint = setpoint; }

    String rx_topic;
    static const char * mode_lookup[];
    static const char * fan_lookup[];
    static const char * action_lookup[];
  private:
    HardwareSerial & _port;
    int8_t _rx_pin;
    int8_t _tx_pin;
    int _mode, _action, _fan, _setpoint, _ac_temp, _comp_rps, _evap_fan;
};
