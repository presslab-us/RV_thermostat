#define Q(x) #x
#define QUOTE(x) Q(x)

const char* VARIABLES_DEF_YAML PROGMEM = R"~(
    Thermostat:
      - therm_mode:
          label: Thermostat mode
          options:
            - Off: 0
            - Auto: 1
            - Cool: 2
            - Dry: 3
            - Fan: 4
            - Heat: 5
          default: Off
      - therm_fan:
          label: Thermostat fan
          options:
            - Auto: 4
            - Low: 1
            - Medium: 2
            - High: 3
          default: Auto
      - therm_setpoint_high:
          label: Thermostat setpoint high *C
          range: )~" QUOTE(MIN_TEMP) ", " QUOTE(MAX_TEMP) R"~(, 1
          default: 22
      - therm_setpoint_low:
          label: Thermostat setpoint low *C
          range: )~" QUOTE(MIN_TEMP) ", " QUOTE(MAX_TEMP) R"~(, 1
          default: 20
      - therm_aux:
          label: Prefer aux heat
          checked: False
      - therm_ac_fail_temp_time:
          label: AC fail for temperature time (seconds)
          range: 60, 7200, 1
          default: 1800
      - therm_furn_deadband:
          label: Furnace deadband *C
          default: 1.5
          attribs: min=0.1 max=5 step=0.1
      - therm_setpoint_deadband:
          labal: Setpoint deadband *C
          range: 1, 5, 1
          default: 2
    Wifi settings:
      - st_ssid:
          default: ssid
      - st_pass:
          default: pass
      - host_name:
          default: rvtherm-{mac}
    
    MQTT settings:
      - mqtt_broker:
          label: MQTT Broker (reboot after change)
      - mqtt_user:
          label: MQTT Username
      - mqtt_pass:
          label: MQTT Password
      - mqtt_topic:
          label: MQTT Topic
          default: rvtherm
    
    ConfigAssist settings:
      - display_style:
          label: Choose how the config sections are displayed. Must reboot to apply
          options:
            - AllOpen: 0
            - AllClosed: 1
            - Accordion: 2
            - AccordionToggleClosed: 3
          default: AccordionToggleClosed
    
    Other settings:
       - gui_brightness:
          label: Display brightness
          range: 0, 255, 1
          default: 24
       - debug:
          label: Check to enable debug
          checked: False
      - sensor_type:
          label: Enter the sensor type
          options: 'BMP280', 'DHT12', 'DHT21', 'DHT22'
          default: DHT22
      - time_zone:
          label: Needs to be a valid time zone string
          default: America/Vancouver
          datalist:
            - Etc/GMT,GMT0
            - Etc/GMT-0,GMT0
            - Etc/GMT-1,<+01>-1    
      - cal_data:
          label: Enter data for 2 Point calibration.</br>Data will be saved to /calibration.ini
          file: /calibration.ini
          default:
            X1=222, Y1=1.22
            X2=900, Y2=3.24
    )~";
    