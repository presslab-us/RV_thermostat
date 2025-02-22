#include <Arduino.h>
#include <M5Dial.h>
#include <M5GFX.h>

#include "gui.h"

void Gui::init()
{
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);
}

void Gui::brightness(int val)
{
    M5Dial.Display.setBrightness(val);
}

void Gui::update()
{
    M5Dial.Display.fillScreen(0xABE1);
    // M5Dial.Display.setTextDatum(4);
    M5Dial.update();
}
