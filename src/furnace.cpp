#include <Arduino.h>
// #include <BMP280_DEV.h>
#include <SensirionI2cSht4x.h>
#include <Unit_4RELAY.h>

#include "furnace.h"

#define RLY_FURNACE 0

SensirionI2cSht4x sht40; // address 0x44
// BMP280_DEV bmp280(SDA_GPIO, SCL_GPIO, Wire); // address 0x77
UNIT_4RELAY relay; // address 0x26

int Furnace::init()
{
  Wire.begin(_sda_pin, _scl_pin, 100000);
  relay.begin(&Wire);
  relay.Init(true); // LEDs synchronous with relays

  sht40.begin(Wire, SHT40_I2C_ADDR_44);
  sht40.softReset();
  /*bmp280.begin(BMP280_I2C_ALT_ADDR);
  //bmp280.setPresOversampling(OVERSAMPLING_X4);    // Set the pressure oversampling to X4
  //bmp280.setTempOversampling(OVERSAMPLING_X1);    // Set the temperature oversampling to X1
  //bmp280.setIIRFilter(IIR_FILTER_4);              // Set the IIR filter to setting 4
  bmp280.setTimeStandby(TIME_STANDBY_2000MS);     // Set the standby time to 2 seconds
  bmp280.startNormalConversion();                 // Start BMP280 continuous conversion in NORMAL_MODE
  */
  return ERR_OK;
}

int Furnace::update()
{
  static uint32_t update_millis = millis();
  if (millis() - update_millis < 1000)
  {
    return ERR_OK;
  }

  if (millis() - update_millis > 2000)
  {
    update_millis = millis();
  }
  else
  {
    update_millis += 1000;
  }
  sht40.measureLowestPrecision(_temp, _hum);


  if (_mode == 0)
  {
    _relay_on = false;
  }
  else if (!_relay_on && _temp < _setpoint - _deadband)
  {
    _relay_on = true;
  }
  else if (_relay_on && _temp > _setpoint)
  {
    _relay_on = false;
  }
  relay.relayWrite(RLY_FURNACE, _relay_on);
  return ERR_OK;
}