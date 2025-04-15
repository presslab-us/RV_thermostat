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
volatile bool send_update = false, aux_heat;
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
  if (aux_heat || turbro.get_temp() == -100)
  {
    return furnace.get_temp();
  }
  else
  {
    return turbro.get_temp();
  }
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
  LOG_I("Starting..\n");
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
    LOG_E("WiFi client connect failed.\n");
  }

  WiFi.enableIPv6();
  WiFi.softAPenableIPv6();
  //confHelper.setReconnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setTxPower(WIFI_POWER_5dBm);
  WiFi.setSleep(false);
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
    if (!strcasecmp("off", payload)) { mode = 0; }
    else if (!strcasecmp("heat_cool", payload)) { mode = 1; }
    else if (!strcasecmp("cool", payload)) { mode = 2; }
    else if (!strcasecmp("dry", payload)) { mode = 3; }
    else if (!strcasecmp("fan_only", payload)) { mode = 4; }
    else if (!strcasecmp("heat", payload)) { mode = 5; }
    send_update = true; });
  mqtt->subscribe(conf["mqtt_topic"] + "/fan/set", [](const char *payload)
                  {
    if (!strcasecmp("auto", payload)) { fan = 4; }
    else if (!strcasecmp("low", payload)) { fan = 1; }
    else if (!strcasecmp("medium", payload)) { fan = 2; }
    else if (!strcasecmp("high", payload)) { fan = 3; }
    send_update = true; });
  mqtt->subscribe(conf["mqtt_topic"] + "/setpoint_low/set", [](const char *payload)
                  {
                    setpoint_low = atof(payload);
                    set_low(setpoint_low, setpoint_high);
                    send_update = true; });
  mqtt->subscribe(conf["mqtt_topic"] + "/setpoint_high/set", [](const char *payload)
                  {
                    setpoint_high = atof(payload);
                    set_high(setpoint_low, setpoint_high);
                    send_update = true; });
  mqtt->subscribe(conf["mqtt_topic"] + "/aux/set", [](const char *payload)
                  {
    aux_heat = !strcasecmp("ON", payload) ? true : false;
    send_update = true; });
  mqtt->subscribe(conf["mqtt_topic"] + "/brightness/set", [](const char *payload)
                  {
    conf["gui_brightness"] = payload;
    gui.brightness(conf["gui_brightness"].toInt()); });

  // Start the client
  mqtt->begin();
}

void loop()
{
  static bool ac_comm_failed = false;
  static bool ac_temp_failed = false;
  static int ac_comm_failed_ctr = 0;
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
  if (aux_heat)
  {
    ac_mode = mode;
    switch (mode)
    {
    case MODE_AUTO:
      ac_mode = MODE_COOL; // change from heat/cool to cool
      furn_mode = true;
      break;
    case MODE_HEAT:
      ac_mode = MODE_OFF; // disable AC unit for heat only
      furn_mode = true;
      break;
    }
    ac_setpoint = setpoint_high;
  }
  else
  {
    static bool cool = false;
    switch (mode)
    {
    // don't use the AC auto mode, instead use cool or heat mode depending on actual temp
    case MODE_AUTO:
      if (turbro.get_temp() > setpoint_high) // cool mode runs fan all the time. only use if within hysteresis
      {
        cool = true;
      }
      else if (turbro.get_temp() < setpoint_high)
      {
        cool = false;
      }

      if (cool)
      {
        ac_mode = MODE_COOL;
        ac_setpoint = setpoint_high;
      }
      else
      {
        ac_mode = MODE_HEAT;
        ac_setpoint = setpoint_low;
      }
      furn_mode = ac_comm_failed || ac_temp_failed;
      break;
    case MODE_HEAT:
      ac_mode = mode;
      ac_setpoint = setpoint_low;
      furn_mode = ac_comm_failed || ac_temp_failed;
      break;
    default:
      ac_mode = mode;
      ac_setpoint = setpoint_high;
      break;
    }
  }
  furnace.set_parms(furn_mode, setpoint_low, conf["therm_furn_deadband"].toFloat());
  turbro.set_parms(ac_mode, fan, ac_setpoint);

  furnace.update();
  int retval = turbro.update();
  
  if (retval == TURBRO_ERR)
  {
    log_i("turbro retval: %d\n", retval);
    if (ac_comm_failed_ctr >= 3)
    {
      ac_comm_failed = true;
    }
    else
    {
      ac_comm_failed_ctr ++;
    }
  }
  else if (retval == TURBRO_OK)
  {
    ac_comm_failed_ctr = 0;
    ac_comm_failed = false;

    if (mode == MODE_AUTO || mode == MODE_HEAT)
    {
      if (ac_setpoint <= turbro.get_temp())
      {
        ac_fail_millis = 0;
        ac_temp_failed = false;
      }
      else if (ac_fail_millis == 0)
      {
        ac_fail_millis = millis();
      }

      if (ac_fail_millis && (millis() - ac_fail_millis > conf["therm_ac_fail_temp_time"].toInt() * 1000))
      {
        ac_temp_failed = true;
      }
    }
    else
    {
      ac_temp_failed = false;
    }
  }

  if (send_update)
  {
    JsonDocument state_json;

    send_update = false;
    mqtt_millis = millis();

    state_json["temp"] = (int)(get_actual() + 0.5);

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
    state_json["aux_heat"] = aux_heat ? "ON" : "OFF";
    state_json["ac_temp"] = turbro.get_temp();
    state_json["ac_comp_rps"] = turbro.get_comp_rps();
    state_json["sht_temp"] = (int)(furnace.get_temp() * 100.0) / 100.0;
    state_json["sht_rh"] = (int)(furnace.get_hum() * 10.0) / 10.0;
    state_json["diag"] = ac_setpoint;

    auto publish = mqtt->begin_publish(conf["mqtt_topic"] + "/state", measureJson(state_json));
    serializeJson(state_json, publish);
    publish.send();
  }

  server.handleClient();
  mqtt->loop();
  gui.update();

  esp_task_wdt_reset();
}
