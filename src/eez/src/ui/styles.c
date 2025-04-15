#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: but_heat
//

void init_style_but_heat_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][0]));
};

lv_style_t *get_style_but_heat_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_but_heat_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_but_heat(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_but_heat_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_but_heat(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_but_heat_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: but_cool
//

void init_style_but_cool_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][2]));
};

lv_style_t *get_style_but_cool_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_but_cool_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_but_cool(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_but_cool_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_but_cool(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_but_cool_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: but_heatcool
//

void init_style_but_heatcool_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][1]));
};

lv_style_t *get_style_but_heatcool_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_but_heatcool_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_but_heatcool(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_but_heatcool_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_but_heatcool(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_but_heatcool_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: but_dry
//

void init_style_but_dry_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][3]));
};

lv_style_t *get_style_but_dry_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_but_dry_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_but_dry(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_but_dry_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_but_dry(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_but_dry_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: but_fan
//

void init_style_but_fan_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff21f3e2));
};

lv_style_t *get_style_but_fan_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_but_fan_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_but_fan(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_but_fan_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_but_fan(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_but_fan_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: but_default
//

void init_style_but_default_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff8c8c8c));
};

lv_style_t *get_style_but_default_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_but_default_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_but_default(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_but_default_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_but_default(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_but_default_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: lbl_highlight
//

void init_style_lbl_highlight_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_opa(style, 255);
};

lv_style_t *get_style_lbl_highlight_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_lbl_highlight_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_lbl_highlight(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_lbl_highlight_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_lbl_highlight(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_lbl_highlight_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: lbl_default
//

void init_style_lbl_default_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_opa(style, 0);
};

lv_style_t *get_style_lbl_default_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_lbl_default_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_lbl_default(lv_obj_t *obj) {
    lv_obj_add_style(obj, get_style_lbl_default_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_lbl_default(lv_obj_t *obj) {
    lv_obj_remove_style(obj, get_style_lbl_default_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_but_heat,
        add_style_but_cool,
        add_style_but_heatcool,
        add_style_but_dry,
        add_style_but_fan,
        add_style_but_default,
        add_style_lbl_highlight,
        add_style_lbl_default,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_but_heat,
        remove_style_but_cool,
        remove_style_but_heatcool,
        remove_style_but_dry,
        remove_style_but_fan,
        remove_style_but_default,
        remove_style_lbl_highlight,
        remove_style_lbl_default,
    };
    remove_style_funcs[styleIndex](obj);
}

