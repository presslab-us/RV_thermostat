#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _groups_t {
    lv_group_t *encoder_group;
} groups_t;

extern groups_t groups;

void ui_create_groups();

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *setpoint_cool;
    lv_obj_t *button_mode;
    lv_obj_t *press_to_select;
    lv_obj_t *press_to_confirm;
    lv_obj_t *setpoint_background;
    lv_obj_t *setpoint_heat;
    lv_obj_t *setpoint_actual;
    lv_obj_t *setpoint_cnt_text;
    lv_obj_t *setpoint_cool_text;
    lv_obj_t *setpoint_heat_text;
    lv_obj_t *actual_text;
    lv_obj_t *button_settings;
    lv_obj_t *obj0;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
};

void create_screen_main();
void tick_screen_main();

enum Themes {
    THEME_ID_DEFAULT,
};
enum Colors {
    COLOR_ID_HEAT,
    COLOR_ID_HEATCOOL,
    COLOR_ID_COOL,
    COLOR_ID_DRY,
    COLOR_ID_HIGHLIGHT,
};
void change_color_theme(uint32_t themeIndex);
extern uint32_t theme_colors[1][5];

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/