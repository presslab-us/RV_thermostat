#include <sdkconfig.h>
#include <driver/pulse_cnt.h>
#include <driver/gpio.h>

#include "encoder.h"

static int enc_offs = 0;
static pcnt_unit_handle_t pcnt_unit = NULL;

/* clang-format: off */
void init_encoder() {
    pinMode(GPIO_NUM_40, INPUT_PULLUP);
    pinMode(GPIO_NUM_41, INPUT_PULLUP);
//    gpio_pullup_en(GPIO_NUM_40);
//    gpio_pullup_en(GPIO_NUM_41);

    pcnt_unit_config_t unit_config = {
        .low_limit = INT16_MIN,
        .high_limit = INT16_MAX,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));
    
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = GPIO_NUM_40,
        .level_gpio_num = GPIO_NUM_41,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = GPIO_NUM_41,
        .level_gpio_num = GPIO_NUM_40,
    };

    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    // Set offset according to intial encoder position
    if (!digitalRead(GPIO_NUM_40) && digitalRead(GPIO_NUM_41)) enc_offs = 1;
    else if (!digitalRead(GPIO_NUM_40) && !digitalRead(GPIO_NUM_41)) enc_offs = 2;
    else if (digitalRead(GPIO_NUM_40) && !digitalRead(GPIO_NUM_41)) enc_offs = 3;
}

int get_encoder() {
    int count = 0;
    pcnt_unit_get_count(pcnt_unit, &count);
    return count + enc_offs;
}
