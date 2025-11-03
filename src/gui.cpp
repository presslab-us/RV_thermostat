#include <Arduino.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <esp_timer.h>
#include <eez-framework.h> // for eez-framwork only
#include "./eez/src/ui/ui.h"
#include "./eez/src/ui/screens.h"
#include "./eez/src/ui/vars.h"
#include "encoder.h"
#include "turbro.h"
#include "gui.h"

#include "./eez/src/ui/actions.h"

extern float temperature, humidity;
extern float setpoint_low, setpoint_high;
extern volatile int mode, fan;
extern String last_action;
void set_low(float &low, float &high);
void set_high(float &low, float &high);
float get_actual(void);

volatile int enc_diff = 0;
volatile bool enc_but = false;
volatile unsigned long enc_millis = 0;
int unval_mode;
float setpoint_unval_low, setpoint_unval_high;
enum { ENC_IDLE, ENC_SELECT_HI, ENC_SELECT_LO, ENC_SET_HI, ENC_SET_LO, ENC_MODE } enc_state;
constexpr int32_t HOR_RES = 240;
constexpr int32_t VER_RES = 240;

lv_display_t *display;
lv_indev_t *indev;

void action_confirm(lv_event_t * e)
{
  enc_but = true;
}

void my_display_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  lv_draw_sw_rgb565_swap(px_map, w * h);
  M5.Display.pushImageDMA<uint16_t>(area->x1, area->y1, w, h, (uint16_t *)px_map);
  lv_disp_flush_ready(disp);
}

uint32_t my_tick_function()
{
  return (esp_timer_get_time() / 1000LL);
}

void my_touchpad_read(lv_indev_t *drv, lv_indev_data_t *data)
{
  M5.update();
  auto count = M5.Touch.getCount();

  if (count == 0)
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
  else
  {
    auto touch = M5.Touch.getDetail(0);
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touch.x;
    data->point.y = touch.y;
  }
}

void my_knob_read(lv_indev_t *drv, lv_indev_data_t *data)
{
  static int32_t last_knob = 0;
  static bool last_but = false;

  int32_t enc = get_encoder();
  if (last_knob - enc >= 4)
  {
    last_knob -= 4;
    enc_diff = -1;
    //data->enc_diff = -1;
    M5.Speaker.tone(2000, 10);
  }
  else if (last_knob - enc <= -4)
  {
    last_knob += 4;
    enc_diff = 1;
    //data->enc_diff = 1;
    M5.Speaker.tone(2200, 10);
  }

  if (M5.BtnA.isPressed() && !last_but)
  {
    M5.Speaker.tone(4000, 10);
    enc_but = true;
  }
  last_but = M5.BtnA.isPressed();

  if (M5.BtnA.isPressed())
  {
    //data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    //data->state = LV_INDEV_STATE_RELEASED;
  }
}

void Gui::init()
{
  M5.begin();
  init_encoder();
  lv_init();

  lv_tick_set_cb(my_tick_function);

  display = lv_display_create(HOR_RES, VER_RES);
  lv_display_set_flush_cb(display, my_display_flush);

  static lv_color_t buf1[HOR_RES * 15];
  lv_display_set_buffers(display, buf1, nullptr, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  ui_create_groups();

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_ENCODER);
  lv_indev_set_read_cb(indev, my_knob_read);
  lv_indev_set_group(indev, groups.encoder_group);

  ui_init();
  enc_state = ENC_IDLE;
}

void Gui::brightness(int val)
{
  M5.Display.setBrightness(val);
}

void Gui::update()
{
  lv_task_handler();
  ui_tick();
  if (enc_state != ENC_IDLE && millis() - enc_millis > 10000)
  {
    enc_state = ENC_IDLE;
  }

  if (enc_but)
  {
    if (enc_state == ENC_SELECT_HI)
    {
      setpoint_unval_high = setpoint_high;
      setpoint_unval_low = setpoint_low;
      enc_state = ENC_SET_HI;
      enc_millis = millis();
    }
    else if (enc_state == ENC_SELECT_LO)
    {
      setpoint_unval_high = setpoint_high;
      setpoint_unval_low = setpoint_low;
      enc_state = ENC_SET_LO;
      enc_millis = millis();
    }
    else if (enc_state == ENC_SET_LO)
    {
      setpoint_low = setpoint_unval_low;
      setpoint_high = setpoint_unval_high;
      enc_state = ENC_IDLE;
    }
    else if (enc_state == ENC_SET_HI)
    {
      setpoint_low = setpoint_unval_low;
      setpoint_high = setpoint_unval_high;
      enc_state = ENC_IDLE;
    }
    else if (enc_state == ENC_MODE)
    {
      mode = unval_mode;
      enc_state = ENC_IDLE;
    }
    enc_but = false;
  }
  if (enc_diff)
  {
    enc_millis = millis();
    switch (mode)
    {
    case TURBRO::MODE_AUTO:
      if (enc_state == ENC_IDLE || enc_state == ENC_SELECT_HI || enc_state == ENC_SELECT_LO)
      {
        if (enc_diff > 0)
        {
          enc_state = ENC_SELECT_HI;
        }
        else
        {
          enc_state = ENC_SELECT_LO;
        }
      }
      else if (enc_state == ENC_SET_LO)
      {
        setpoint_unval_low += enc_diff;
        set_low(setpoint_unval_low, setpoint_unval_high);
      }
      else if (enc_state == ENC_SET_HI)
      {
        setpoint_unval_high += enc_diff;
        set_high(setpoint_unval_low, setpoint_unval_high);
      }
      break;
    case TURBRO::MODE_HEAT:
      if (enc_state == ENC_IDLE)
      {
        setpoint_unval_high = setpoint_high;
        setpoint_unval_low = setpoint_low;
        unval_mode = mode;
        enc_state = ENC_SET_LO;
      }
      if (enc_state == ENC_SET_LO)
      {
        setpoint_unval_low += enc_diff;
        set_low(setpoint_unval_low, setpoint_unval_high);
      }
      break;
    case TURBRO::MODE_COOL:
    case TURBRO::MODE_DRY:
      if (enc_state == ENC_IDLE)
      {
        setpoint_unval_high = setpoint_high;
        setpoint_unval_low = setpoint_low;
        unval_mode = mode;
        enc_state = ENC_SET_HI;
      }
      if (enc_state == ENC_SET_HI)
      {
        setpoint_unval_high += enc_diff;
        set_high(setpoint_unval_low, setpoint_unval_high);
      }
      break;
    }
    enc_diff = 0;
  }
}

int32_t get_var_actual()
{
  return (int)(get_actual() * 10);
}

void set_var_actual(int32_t value)
{
}

int32_t get_var_mode()
{
  if (enc_state == ENC_MODE)
  {
    return unval_mode;
  }
  return mode;
}

void set_var_mode(int32_t value)
{
  enc_millis = millis();
  enc_state = ENC_MODE;
  unval_mode = value;
}

int32_t get_var_setpoint_cool()
{
  if (enc_state == ENC_SET_LO || enc_state == ENC_SET_HI)
  {
    return setpoint_unval_high;
  }
  return setpoint_high;
}

void set_var_setpoint_cool(int32_t value)
{
}

int32_t get_var_setpoint_heat()
{
  if (enc_state == ENC_SET_LO || enc_state == ENC_SET_HI)
  {
    return setpoint_unval_low;
  }
  return setpoint_low;
}

void set_var_setpoint_heat(int32_t value)
{
}

int32_t get_var_enc_state()
{
  return enc_state;
}

void set_var_enc_state(int32_t value)
{
}
