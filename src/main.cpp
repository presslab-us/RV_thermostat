#include <Arduino.h>

#include <WiFi.h>
#include <ConfigAssist.h>
#include <ConfigAssistHelper.h>
#include <WebServer.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>

#include <esp_task_wdt.h>

#include "gui.h"
#include "turbro.h"
#include "furnace.h"
#include "variables_def.h"

#define RX_GPIO 1
#define TX_GPIO 2
#define SDA_GPIO 13
#define SCL_GPIO 15

float temperature, humidity;
float setpoint_low, setpoint_high;
volatile int mode, fan;
volatile bool send_update = false;
volatile int aux_heat;
String last_action;

Gui gui;
TURBRO turbro(Serial2, RX_GPIO, TX_GPIO);
Furnace furnace(SDA_GPIO, SCL_GPIO);

WebServer server(80);
ConfigAssist conf("/config.ini", VARIABLES_DEF_YAML);
PicoMQTT::Client *mqtt;

void set_low(float &low, float &high)
{
  float db = conf["therm_setpoint_deadband"].toFloat();
  if (low < MIN_TEMP)
  {
    low = MIN_TEMP;
  }
  else if (low > MAX_TEMP - db)
  {
    low = MAX_TEMP - db;
  }
  if (low > high - db)
  {
    high = low + db;
  }
}

void set_high(float &low, float &high)
{
  float db = conf["therm_setpoint_deadband"].toFloat();
  if (high < MIN_TEMP + db)
  {
    high = MIN_TEMP + db;
  }
  else if (high > MAX_TEMP)
  {
    high = MAX_TEMP;
  }
  if (high < low + db)
  {
    low = high - db;
  }
}

float get_actual(void)
{
  return furnace.get_temp();
}

int get_relative_setpoint(int setpoint)
{
  static float temp = 0;
  float actual = get_actual();

  if (actual - temp > 0.2 || actual - temp < -0.2) // give +/- 0.2 C of hysteresis for noise
  {
    temp = actual;
  }

  // Offset the TURBRO measured temp by the difference from the setpoint and actual temp, and multiplying by the gain
  return (setpoint - temp) * conf["therm_ac_gain"].toFloat() + turbro.get_temp() + 0.5;
}

// Handler function for Home page
void handleRoot()
{
  String out("<h2>Hello from {name}</h2>");
  out += "<h4>Device time: " + conf.getLocalTime() + "</h4>";
  out += "<h4>IPv4: " + WiFi.localIP().toString() + "</h4>";
  out += "<h4>IPv6 Local: " + WiFi.linkLocalIPv6().toString() + "</h4>";
  out += "<h4>IPv6 Global: " + WiFi.globalIPv6().toString() + "</h4>";
  out += "<a href='/cfg'>Edit config</a>";
  out.replace("{name}", "ESP32");
  server.send(200, "text/html", out);
}

void setup()
{
  sleep(5);
  log_i("Starting..\n");
  ESP_ERROR_CHECK(esp_task_wdt_deinit());
  esp_task_wdt_config_t twdt_config = {
    .timeout_ms = 120*1000,
    .idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,    // Bitmask of all cores
    .trigger_panic = false,
  };
  ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));
  enableLoopWDT();
  gui.init();
  furnace.init(); // do this soon so relay will shut off when reset (due to WDT/crash)
  
  turbro.init();
  turbro.rx_topic = conf("mqtt_topic") + "/rx";

  gui.brightness(conf["gui_brightness"].toInt());

  // conf.deleteConfig(); // Uncomment to remove ini file and re-build

  ConfigAssistHelper confHelper(conf);
  WiFi.setHostname(conf("host_name").c_str());
  
  // Connect to any available network
  bool bConn = confHelper.connectToNetwork(15000);
  if (!bConn)
  {
    log_e("WiFi client connect failed.\n");
  }

  WiFi.enableIPv6();
  WiFi.softAPenableIPv6();
  //confHelper.setReconnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setTxPower(WIFI_POWER_5dBm);
  //WiFi.setSleep(false);
  // Start AP to fix wifi credentials
  conf.setup(server, !bConn);

  // Register handlers for web server
  server.on("/", handleRoot);
  server.on("/d", []() { // Append dump handler
    conf.dump(&server);
  });

  // Start web server
  server.begin();

  conf.setRemotUpdateCallback([](String key)
                              {
    if (key == "therm_mode")
    {
      mode = conf["therm_mode"].toInt();
    }
    if (key == "therm_fan")
    {
      fan = conf["therm_fan"].toInt();
    }
    if (key == "therm_setpoint_low")
    {
      setpoint_low = conf["therm_setpoint_low"].toFloat();
      set_low(setpoint_low, setpoint_high);
    }
    if (key == "therm_setpoint_high")
    {
      setpoint_high = conf["therm_setpoint_high"].toFloat();
      set_high(setpoint_low, setpoint_high);
    }
    if (key == "therm_aux")
    {
      aux_heat = conf["therm_aux"].toInt();
    }
    if (key == "gui_brightness")
    {
      gui.brightness(conf["gui_brightness"].toInt());
    } });

  mode = conf["therm_mode"].toInt();
  fan = conf["therm_fan"].toInt();
  setpoint_low = conf["therm_setpoint_low"].toInt();
  setpoint_high = conf["therm_setpoint_high"].toInt();
  aux_heat = conf["therm_aux"].toInt();

  mqtt = new PicoMQTT::Client(conf["mqtt_broker"].c_str(), 1883, nullptr, conf["mqtt_user"].c_str(), conf["mqtt_pass"].c_str(), 5000, 60000, 300);

  // Subscribe to a topic pattern and attach a callback
  mqtt->subscribe(conf["mqtt_topic"] + "/mode/set", [](const char *payload)
                  {
    if (!strcasecmp(TURBRO::mode_lookup[TURBRO::MODE_OFF], payload)) { mode = TURBRO::MODE_OFF; }
    else if (!strcasecmp(TURBRO::mode_lookup[TURBRO::MODE_AUTO], payload)) { mode = TURBRO::MODE_AUTO; }
    else if (!strcasecmp(TURBRO::mode_lookup[TURBRO::MODE_COOL], payload)) { mode = TURBRO::MODE_COOL; }
    else if (!strcasecmp(TURBRO::mode_lookup[TURBRO::MODE_DRY], payload)) { mode = TURBRO::MODE_DRY; }
    else if (!strcasecmp(TURBRO::mode_lookup[TURBRO::MODE_FAN], payload)) { mode = TURBRO::MODE_FAN; }
    else if (!strcasecmp(TURBRO::mode_lookup[TURBRO::MODE_HEAT], payload)) { mode = TURBRO::MODE_HEAT; }
    send_update = true; }, 16);
  mqtt->subscribe(conf["mqtt_topic"] + "/fan/set", [](const char *payload)
                  {
    if (!strcasecmp(TURBRO::fan_lookup[TURBRO::FAN_AUTO], payload)) { fan = TURBRO::FAN_AUTO; }
    else if (!strcasecmp(TURBRO::fan_lookup[TURBRO::FAN_LOW], payload)) { fan = TURBRO::FAN_LOW; }
    else if (!strcasecmp(TURBRO::fan_lookup[TURBRO::FAN_MED], payload)) { fan = TURBRO::FAN_MED; }
    else if (!strcasecmp(TURBRO::fan_lookup[TURBRO::FAN_HIGH], payload)) { fan = TURBRO::FAN_HIGH; }
    send_update = true; }, 16);
  mqtt->subscribe(conf["mqtt_topic"] + "/setpoint_low/set", [](const char *payload)
                  {
                    setpoint_low = atof(payload);
                    set_low(setpoint_low, setpoint_high);
                    send_update = true; }, 16);
  mqtt->subscribe(conf["mqtt_topic"] + "/setpoint_high/set", [](const char *payload)
                  {
                    setpoint_high = atof(payload);
                    set_high(setpoint_low, setpoint_high);
                    send_update = true; }, 16);
  mqtt->subscribe(conf["mqtt_topic"] + "/aux/set", [](const char *payload)
                  {
    if (!strcasecmp(Furnace::aux_lookup[Furnace::AUX_NORMAL], payload)) { aux_heat = Furnace::AUX_NORMAL; }
    else if (!strcasecmp(Furnace::aux_lookup[Furnace::AUX_ONLY], payload)) { aux_heat = Furnace::AUX_ONLY; }
    else if (!strcasecmp(Furnace::aux_lookup[Furnace::AUX_QUICKHEAT], payload)) { aux_heat = Furnace::AUX_QUICKHEAT; }
    else if (!strcasecmp(Furnace::aux_lookup[Furnace::AUX_NEVER], payload)) { aux_heat = Furnace::AUX_NEVER; }
    send_update = true; }, 16);
  mqtt->subscribe(conf["mqtt_topic"] + "/brightness/set", [](const char *payload)
                  {
    conf["gui_brightness"] = payload;
    gui.brightness(conf["gui_brightness"].toInt()); }, 16);

  // Start the client
  mqtt->begin();

  xTaskCreatePinnedToCore([](void *)
                  {
                    while(1)
                    {
                      gui.update();
                      delay(10);
                    }
                  },
                  "gui_task", 10000, NULL, 0, NULL, 0);
}

void loop()
{
  static bool ac_comm_failed = false;
  static bool ac_temp_failed = false;
  int ac_mode = 0;
  static uint32_t save_millis = 0, ac_fail_millis = 0;

  int ac_setpoint;

  if (mode != conf["therm_mode"].toInt() || fan != conf["therm_fan"].toInt() || setpoint_low != conf["therm_setpoint_low"].toInt() ||
      setpoint_high != conf["therm_setpoint_high"].toInt() || aux_heat != conf["therm_setpoint"].toInt())
  {
    conf["therm_mode"] = mode;
    conf["therm_fan"] = fan;
    conf["therm_setpoint_low"] = setpoint_low;
    conf["therm_setpoint_high"] = setpoint_high;
    conf["therm_aux"] = aux_heat;
    save_millis = millis();
  }

  if (save_millis && millis() - save_millis > 10000)
  {
    save_millis = 0;
    conf.saveConfigFile();
  }
  static uint32_t mqtt_millis = millis();
  if (millis() - mqtt_millis > 1000)
  {
    send_update = true;
  }

  bool furn_mode = false;
  static bool cool = false, heated = false;
  float threshold = heated ? setpoint_high + 1 : setpoint_high; // increase hysteresis if we have heated
  static uint32_t cool_timer = 0;
  static float last_setpoint_high = 0;
  #define COOL_TIMER_ON (conf["cool_timer"].toInt() * 60 * 1000)
  #define COOL_TIMER_DELAY (30 * 1000)
  #define COOL_TIMER_OFF (COOL_TIMER_ON + COOL_TIMER_DELAY)
  #define COOL_TIMER_BAND 6
  static enum { COOLING_OFF, COOLING_START, COOLING, COOLING_STOP } cool_state = COOLING_OFF;
  switch (mode)
  {
  // don't use the AC auto mode, instead use cool or heat mode depending on actual temp
  case TURBRO::MODE_AUTO:
    static int last_state = 0;
    if (last_state != cool_state)
    {
      last_state = cool_state;
      log_i("state %d\n", cool_state);   
    }
    if (last_setpoint_high != setpoint_high) // reset cool_timer when changing setpoint (or mode)
    {
      last_setpoint_high = setpoint_high;
      cool_timer = millis() - COOL_TIMER_ON;
    }
    else if (cool_state == COOLING_OFF && get_actual() > threshold) // cool mode runs fan all the time. only use if within hysteresis
    {
      cool_state = COOLING_START;
      cool_timer = millis();
    }
    else
    {
      if (cool_state == COOLING_OFF)
      {
        if (turbro.get_action() == TURBRO::ACTION_COOLING)
        {
          cool = true;
          cool_state = COOLING;
        }
      }
      else if (cool_state == COOLING_START)
      {
        cool = true;
        heated = false;
        if (millis() - cool_timer > COOL_TIMER_DELAY)
        {
          cool_state = COOLING;
        }
      }
      else if (cool_state == COOLING)
      {
        if (turbro.get_action() != TURBRO::ACTION_COOLING)
        {
          cool_state = COOLING_STOP;
          cool_timer = millis();
        }
      }
      else if (cool_state == COOLING_STOP)
      {
        if (millis() - cool_timer > COOL_TIMER_DELAY)
        {
          cool = false;
          cool_state = COOLING_OFF;
          cool_timer = millis();
        }
      }
    }

    if (turbro.get_action() == TURBRO::ACTION_HEATING)
    {
      heated = true;
    }

    if (cool)
    {
      ac_mode = TURBRO::MODE_COOL;
      ac_setpoint = get_relative_setpoint(setpoint_high);
    }
    else
    {
      furn_mode = (ac_comm_failed || ac_temp_failed && aux_heat != Furnace::AUX_NEVER) || aux_heat == Furnace::AUX_ONLY || aux_heat == Furnace::AUX_QUICKHEAT;
      ac_mode = (aux_heat == Furnace::AUX_ONLY) ? TURBRO::MODE_OFF : TURBRO::MODE_HEAT;
      ac_setpoint = get_relative_setpoint(setpoint_low);
    }
    break;
  case TURBRO::MODE_HEAT:
    heated = false;
    cool = false;
    ac_setpoint = get_relative_setpoint(setpoint_low);
    furn_mode = (ac_comm_failed || ac_temp_failed && aux_heat != Furnace::AUX_NEVER) || aux_heat == Furnace::AUX_ONLY || aux_heat == Furnace::AUX_QUICKHEAT;
    ac_mode = (aux_heat == Furnace::AUX_ONLY) ? TURBRO::MODE_OFF : mode;
    last_setpoint_high = -1;
    break;
  default:
    heated = false;
    cool = false;
    ac_mode = mode;
    ac_setpoint = get_relative_setpoint(setpoint_high);
    last_setpoint_high = -1;
    break;
  }
  if (turbro.get_action() != TURBRO::ACTION_OFF && turbro.get_comp_rps() > 0 && turbro.get_comp_rps() < conf["therm_ac_min_rps"].toInt())
  {
    log_i("Compressor below min speed\n");
    ac_mode = TURBRO::MODE_OFF; // Stop running when compressor is running too slow; this blocks the lowest level which has a booming resonance sound.
  }
  furnace.set_parms(furn_mode, setpoint_low, conf["therm_furn_deadband"].toFloat());
  turbro.set_parms(ac_mode, fan, ac_setpoint);

  furnace.update();
  int retval = turbro.update();

  if (retval == TURBRO::RET_RXERR)
  {
    log_e("comm failed\n");
    ac_comm_failed = true;
  }
  else if (retval == TURBRO::RET_RXOK)
  {
    if (ac_comm_failed) log_e("comm recover\n");
    ac_comm_failed = false;

    if (mode == TURBRO::MODE_AUTO || mode == TURBRO::MODE_HEAT)
    {
      if (setpoint_low - 2 <= get_actual()) // allow for up to 2 C
      {
        ac_temp_failed = false;
        ac_fail_millis = millis();
      }
      else if (!ac_temp_failed && millis() - ac_fail_millis > (uint32_t)conf["therm_ac_fail_temp_time"].toInt() * 60000UL)
      {
        ac_temp_failed = true;
      }
    }
    else
    {
      ac_temp_failed = false;
      ac_fail_millis = millis();
    }
  }

  if (send_update)
  {
    JsonDocument state_json;

    send_update = false;
    mqtt_millis = millis();

    if (furnace.get_action())
    {
      state_json["action"] = TURBRO::action_lookup[furnace.get_action()];
      if (state_json["action"] != "off")
      {
        last_action = state_json["action"].as<String>();
      }
    }
    else
    {
      state_json["action"] = TURBRO::action_lookup[turbro.get_action()];
      if (state_json["action"] != "off")
      {
        last_action = state_json["action"].as<String>();
      }
    }

    state_json["mode"] = TURBRO::mode_lookup[mode];
    state_json["fan"] = TURBRO::fan_lookup[fan];
    state_json["ac_action"] = TURBRO::action_lookup[turbro.get_action()];
    state_json["furn_action"] = TURBRO::action_lookup[furnace.get_action()];
    state_json["setpoint_low"] = setpoint_low;
    state_json["setpoint_high"] = setpoint_high;
    state_json["aux_heat"] = Furnace::aux_lookup[aux_heat];
    state_json["ac_temp"] = turbro.get_temp();
    state_json["ac_comp_rps"] = turbro.get_comp_rps();
    state_json["ac_setpoint"] = ac_setpoint;
    state_json["sht_temp"] = (int)(get_actual() * 100.0) / 100.0;
    state_json["sht_rh"] = (int)(furnace.get_hum() * 10.0) / 10.0;
    state_json["temp"] = (int)(get_actual() * 10.0) / 10.0;
    
    auto publish = mqtt->begin_publish(conf["mqtt_topic"] + "/state", measureJson(state_json));
    serializeJson(state_json, publish);
    publish.send();
    if (WiFi.status() == WL_IDLE_STATUS)
    {
      sleep(2);
      WiFi.begin();
    }
    //log_i("WiFi status: %d\n", WiFi.status());
  }

  mqtt->loop();
  server.handleClient();

  esp_task_wdt_reset();
}
