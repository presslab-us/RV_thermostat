#include <Arduino.h>

#include <WiFi.h>
#include <ConfigAssist.h> 
#include <ConfigAssistHelper.h>
#include <WebServer.h>
#include <PicoMQTT.h>
#include <ArduinoJson.h>

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

Gui gui;
TURBRO turbro(Serial2, RX_GPIO, TX_GPIO);
Furnace furnace(SDA_GPIO, SCL_GPIO);

WebServer server(80);
ConfigAssist conf("/config.ini", VARIABLES_DEF_YAML);
PicoMQTT::Client *mqtt;

// Handler function for Home page
void handleRoot()
{
  String out("<h2>Hello from {name}</h2>");
  out += "<h4>Device time: " + conf.getLocalTime() + "</h4>";
  out += "<a href='/cfg'>Edit config</a>";
  out.replace("{name}", "ESP32");
  server.send(200, "text/html", out);
}

void setup()
{
  sleep(5);
  LOG_I("Starting..\n");
  gui.init();
  furnace.init(); // do this soon so relay will shut off when reset (due to WDT/crash)

  //WiFi.setTxPower(WIFI_POWER_8_5dBm);
  turbro.init();
  turbro.rx_topic = conf("mqtt_topic") + "/rx";

  gui.brightness(conf["gui_brightness"].toInt());

  // conf.deleteConfig(); // Uncomment to remove ini file and re-build

  ConfigAssistHelper confHelper(conf);

  WiFi.setHostname(conf("host_name").c_str());

  // Connect to any available network
  bool bConn = confHelper.connectToNetwork(15000);
  if (!bConn)
    LOG_E("WiFi client connect failed.\n");

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
      setpoint_low = conf["therm_setpoint_low"].toInt();
      if (setpoint_high - conf["therm_setpoint_deadband"].toFloat() < setpoint_low)
      {
        setpoint_high = setpoint_low + conf["therm_setpoint_deadband"].toFloat();
      }
      }
    if (key == "therm_setpoint_high")
    {
      setpoint_high = conf["therm_setpoint_high"].toInt();
      if (setpoint_low + conf["therm_setpoint_deadband"].toFloat() > setpoint_high)
      {
        setpoint_low = setpoint_high - conf["therm_setpoint_deadband"].toFloat();
      }
      }
    if (key == "therm_aux")
    {
      aux_heat = conf["therm_aux"].toInt();
    }
    if (key == "gui_brightness")
    {
      gui.brightness(conf["gui_brightness"].toInt());
    }
  });

  mode = conf["therm_mode"].toInt();
  fan = conf["therm_fan"].toInt();
  setpoint_low = conf["therm_setpoint_low"].toInt();
  setpoint_high = conf["therm_setpoint_high"].toInt();
  aux_heat = conf["therm_aux"].toInt();

  mqtt = new PicoMQTT::Client(conf["mqtt_broker"].c_str(), 1883, nullptr, conf["mqtt_user"].c_str(), conf["mqtt_pass"].c_str());

  // Subscribe to a topic pattern and attach a callback
  mqtt->subscribe(conf["mqtt_topic"] + "/mode/set", [](const char *payload)
  {
    if (!strcasecmp("off", payload)) { mode = 0; }
    else if (!strcasecmp("heat_cool", payload)) { mode = 1; }
    else if (!strcasecmp("cool", payload)) { mode = 2; }
    else if (!strcasecmp("dry", payload)) { mode = 3; }
    else if (!strcasecmp("fan_only", payload)) { mode = 4; }
    else if (!strcasecmp("heat", payload)) { mode = 5; }
    send_update = true;
  });
  mqtt->subscribe(conf["mqtt_topic"] + "/fan/set", [](const char *payload)
  {
    if (!strcasecmp("auto", payload)) { fan = 4; }
    else if (!strcasecmp("low", payload)) { fan = 1; }
    else if (!strcasecmp("medium", payload)) { fan = 2; }
    else if (!strcasecmp("high", payload)) { fan = 3; }
    send_update = true;
  });
  mqtt->subscribe(conf["mqtt_topic"] + "/setpoint_low/set", [](const char *payload)
  {
    setpoint_low = atof(payload);
    if (setpoint_high - conf["therm_setpoint_deadband"].toFloat() < setpoint_low)
    {
      setpoint_high = setpoint_low + conf["therm_setpoint_deadband"].toFloat();
    }
    send_update = true;
  });
  mqtt->subscribe(conf["mqtt_topic"] + "/setpoint_high/set", [](const char *payload)
  {
    setpoint_high = atof(payload);
    if (setpoint_low + conf["therm_setpoint_deadband"].toFloat() > setpoint_high)
    {
      setpoint_low = setpoint_high - conf["therm_setpoint_deadband"].toFloat();
    }
    send_update = true;
  });
  mqtt->subscribe(conf["mqtt_topic"] + "/aux/set", [](const char *payload)
  {
    aux_heat = !strcasecmp("ON", payload) ? true : false;
    send_update = true;
  });
  mqtt->subscribe(conf["mqtt_topic"] + "/brightness/set", [](const char *payload)
  {
    conf["gui_brightness"] = payload;
    gui.brightness(conf["gui_brightness"].toInt());
  });


  // Start the client
  mqtt->begin();
}

void loop()
{
  static bool ac_failed = false;
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

  if (aux_heat)
  {
    bool furn_mode = false;
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
    furnace.set_parms(furn_mode, setpoint_low, conf["therm_furn_deadband"].toFloat());
    ac_setpoint = setpoint_high;
  }
  else
  {
    bool furn_mode = false;
    static bool use_high_setpoint = false;
    ac_mode = mode;
    switch (mode)
    {
      case MODE_AUTO:
        if (turbro.get_temp() > setpoint_high) // use the setpoint closer to actual
        {
          use_high_setpoint = true;
        }
        else if (turbro.get_temp() < setpoint_low)
        {
          use_high_setpoint = false;
        }
        ac_setpoint = use_high_setpoint ? setpoint_high : setpoint_low;
        ac_mode = use_high_setpoint ? MODE_COOL : MODE_HEAT;
        furn_mode = ac_failed;
        break;
      case MODE_HEAT:
        ac_setpoint = setpoint_low;
        furn_mode = ac_failed;
        break;
      default:
        ac_setpoint = setpoint_high;
        break;
    }
    furnace.set_parms(furn_mode, setpoint_low, conf["therm_furn_deadband"].toFloat());
    turbro.set_parms(ac_mode, fan, ac_setpoint);
  }

  furnace.update();
  int retval = turbro.update();

  // this complicated failover routine will do three things:
  //  if ac unit doesn't commuicate for "therm_ac_fail_comm_time" it will set ac_failed to run aux heater (when demanded)
  //  if ac unit doesn't get within "therm_ac_fail_temp" of setpoint (heating side only) for "therm_ac_fail_temp_time" it will set ac_failed
  //  after either failure it will wait for "therm_ac_recover_time" before clearing ac_failed
  if (retval != ERR_OK)
  {
    log_i("turbro retval: %d\n", retval);
    if (ac_failed)
    {
      ac_fail_millis = millis();
    }
    else if (millis() - ac_fail_millis > conf["therm_ac_fail_comm_time"].toInt() * 1000)
    {
      ac_failed = true;
    }
  }
  else
  {
    if ((mode != MODE_AUTO && mode != MODE_HEAT) ||
      ac_setpoint - turbro.get_temp() <= conf["therm_ac_fail_temp"].toInt()) // check if temp is good
    {
      if (ac_failed) // if failed then only clear flag if recover time has passed
      {
        if (millis() - ac_fail_millis > conf["therm_ac_recover_time"].toInt() * 1000)
        {
          ac_failed = false;
          ac_fail_millis = millis();
        }
      }
      else
      {
        ac_fail_millis = millis();
      }
    }
    else if (ac_failed)
    {
      ac_fail_millis = millis();
    }
    if (millis() - ac_fail_millis > conf["therm_ac_fail_temp_time"].toInt() * 1000)
    {
      ac_failed = true;
    }
  }

  if (send_update) {
    JsonDocument state_json;

    send_update = false;
    mqtt_millis = millis();
  
    if (aux_heat || turbro.get_temp() == -100)
    {
      state_json["temp"] = (int)(furnace.get_temp() + 0.5);
    }
    else
    {
      state_json["temp"] = turbro.get_temp();
    }
  
    if (furnace.get_action()) {
      state_json["action"] = TURBRO::action_lookup[furnace.get_action()];
    }
    else
    {
      state_json["action"] = TURBRO::action_lookup[turbro.get_action()];
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
}
