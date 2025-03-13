#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: but_heat
lv_style_t *get_style_but_heat_MAIN_DEFAULT();
void add_style_but_heat(lv_obj_t *obj);
void remove_style_but_heat(lv_obj_t *obj);

// Style: but_cool
lv_style_t *get_style_but_cool_MAIN_DEFAULT();
void add_style_but_cool(lv_obj_t *obj);
void remove_style_but_cool(lv_obj_t *obj);

// Style: but_heatcool
lv_style_t *get_style_but_heatcool_MAIN_DEFAULT();
void add_style_but_heatcool(lv_obj_t *obj);
void remove_style_but_heatcool(lv_obj_t *obj);

// Style: but_dry
lv_style_t *get_style_but_dry_MAIN_DEFAULT();
void add_style_but_dry(lv_obj_t *obj);
void remove_style_but_dry(lv_obj_t *obj);

// Style: but_default
lv_style_t *get_style_but_default_MAIN_DEFAULT();
void add_style_but_default(lv_obj_t *obj);
void remove_style_but_default(lv_obj_t *obj);

// Style: lbl_highlight
lv_style_t *get_style_lbl_highlight_MAIN_DEFAULT();
void add_style_lbl_highlight(lv_obj_t *obj);
void remove_style_lbl_highlight(lv_obj_t *obj);

// Style: lbl_default
lv_style_t *get_style_lbl_default_MAIN_DEFAULT();
void add_style_lbl_default(lv_obj_t *obj);
void remove_style_lbl_default(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/