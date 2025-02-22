#include <Arduino.h>
#include <HardwareSerial.h>

class Furnace {
  public:
    Furnace(int8_t sda_pin, int8_t scl_pin) : _sda_pin(sda_pin), _scl_pin(scl_pin), _relay_on(false) {};

    int init();
    int update();
    float get_temp() { return _temp; }
    float get_hum() { return _hum; }
    int get_action() { return _relay_on ? 3 : 0; }
    void set_parms(int mode, float setpoint, float deadband) { _mode = mode; _setpoint = setpoint; _deadband = deadband;}

  private:
    int8_t _sda_pin;
    int8_t _scl_pin;
    int _mode;
    float _temp, _hum, _setpoint, _deadband;
    bool _relay_on;
};
