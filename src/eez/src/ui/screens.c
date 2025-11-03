#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

groups_t groups;
static bool groups_created = false;

objects_t objects;
lv_obj_t *tick_value_change_obj;

static void event_handler_cb_main_main(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_SCREEN_LOAD_START) {
        // group: encoder_group
        lv_group_remove_all_objs(groups.encoder_group);
        lv_group_add_obj(groups.encoder_group, objects.setpoint_cool);
        lv_group_add_obj(groups.encoder_group, objects.setpoint_heat);
    }
}

static void event_handler_cb_main_setpoint_cool(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_arc_get_value(ta);
            assignIntegerProperty(flowState, 2, 4, value, "Failed to assign Value in Arc widget");
        }
    }
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 2, 0, e);
    }
}

static void event_handler_cb_main_setpoint_heat(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_arc_get_value(ta);
            assignIntegerProperty(flowState, 3, 4, value, "Failed to assign Value in Arc widget");
        }
    }
}

static void event_handler_cb_main_setpoint_actual(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_arc_get_value(ta);
            assignIntegerProperty(flowState, 4, 3, value, "Failed to assign Value in Arc widget");
        }
    }
}

static void event_handler_cb_main_button_mode(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_RELEASED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 9, 0, e);
    }
}

static void event_handler_cb_main_press_to_select(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        action_confirm(e);
    }
}

static void event_handler_cb_main_press_to_confirm(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        action_confirm(e);
    }
}

void create_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_add_event_cb(obj, event_handler_cb_main_main, LV_EVENT_ALL, flowState);
    {
        lv_obj_t *parent_obj = obj;
        {
            // setpoint_background
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.setpoint_background = obj;
            lv_obj_set_pos(obj, LV_PCT(5), LV_PCT(5));
            lv_obj_set_size(obj, LV_PCT(90), LV_PCT(90));
            lv_arc_set_range(obj, 0, 0);
            lv_arc_set_value(obj, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // setpoint_cool
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.setpoint_cool = obj;
            lv_obj_set_pos(obj, LV_PCT(5), LV_PCT(5));
            lv_obj_set_size(obj, LV_PCT(90), LV_PCT(90));
            lv_arc_set_range(obj, 5, 35);
            lv_arc_set_mode(obj, LV_ARC_MODE_REVERSE);
            lv_obj_add_event_cb(obj, event_handler_cb_main_setpoint_cool, LV_EVENT_ALL, flowState);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE);
            lv_obj_add_state(obj, LV_STATE_FOCUS_KEY);
            lv_obj_set_style_bg_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][2]), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][2]), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
        {
            // setpoint_heat
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.setpoint_heat = obj;
            lv_obj_set_pos(obj, LV_PCT(5), LV_PCT(5));
            lv_obj_set_size(obj, LV_PCT(90), LV_PCT(90));
            lv_arc_set_range(obj, 5, 35);
            lv_obj_add_event_cb(obj, event_handler_cb_main_setpoint_heat, LV_EVENT_ALL, flowState);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE);
            lv_obj_add_state(obj, LV_STATE_FOCUS_KEY);
            lv_obj_set_style_arc_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][0]), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][0]), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // setpoint_actual
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.setpoint_actual = obj;
            lv_obj_set_pos(obj, LV_PCT(5), LV_PCT(5));
            lv_obj_set_size(obj, LV_PCT(90), LV_PCT(90));
            lv_arc_set_range(obj, 50, 350);
            lv_obj_add_event_cb(obj, event_handler_cb_main_setpoint_actual, LV_EVENT_ALL, flowState);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa2a2a2), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 1, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // setpoint_cnt_text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.setpoint_cnt_text = obj;
            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(-20));
            lv_obj_set_size(obj, LV_PCT(30), LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
            add_style_lbl_default(obj);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // setpoint_cool_text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.setpoint_cool_text = obj;
            lv_obj_set_pos(obj, LV_PCT(15), LV_PCT(-18));
            lv_obj_set_size(obj, LV_PCT(31), LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
            add_style_lbl_default(obj);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][2]), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // setpoint_heat_text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.setpoint_heat_text = obj;
            lv_obj_set_pos(obj, LV_PCT(-15), LV_PCT(-18));
            lv_obj_set_size(obj, LV_PCT(31), LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
            add_style_lbl_default(obj);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][0]), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // actual_text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.actual_text = obj;
            lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(-8));
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
            add_style_lbl_default(obj);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // button_mode
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.button_mode = obj;
            lv_obj_set_pos(obj, LV_PCT(-18), LV_PCT(10));
            lv_obj_set_size(obj, LV_PCT(25), LV_PCT(25));
            lv_obj_add_event_cb(obj, event_handler_cb_main_button_mode, LV_EVENT_ALL, flowState);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
            add_style_but_default(obj);
            lv_obj_set_style_radius(obj, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.obj0 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_lbl_default(obj);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_material_design_48, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
            }
        }
        {
            // button_settings
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.button_settings = obj;
            lv_obj_set_pos(obj, LV_PCT(18), LV_PCT(10));
            lv_obj_set_size(obj, LV_PCT(25), LV_PCT(25));
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
            add_style_but_default(obj);
            lv_obj_set_style_radius(obj, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff8c8c8c), LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    add_style_lbl_default(obj);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_material_design_48, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "\ue8b8");
                }
            }
        }
        {
            // press_to_select
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.press_to_select = obj;
            lv_obj_set_pos(obj, 0, 84);
            lv_obj_set_size(obj, LV_PCT(50), LV_PCT(16));
            lv_obj_add_event_cb(obj, event_handler_cb_main_press_to_select, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_PRESS_LOCK);
            add_style_lbl_default(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "\npress to select");
        }
        {
            // press_to_confirm
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.press_to_confirm = obj;
            lv_obj_set_pos(obj, 0, 84);
            lv_obj_set_size(obj, LV_PCT(50), LV_PCT(16));
            lv_obj_add_event_cb(obj, event_handler_cb_main_press_to_confirm, LV_EVENT_ALL, flowState);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_PRESS_LOCK);
            add_style_lbl_default(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(theme_colors[eez_flow_get_selected_theme_index()][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "\npress to confirm");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    {
        bool new_val = evalBooleanProperty(flowState, 2, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.setpoint_cool, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_cool;
            if (new_val) lv_obj_add_flag(objects.setpoint_cool, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(objects.setpoint_cool, LV_OBJ_FLAG_HIDDEN);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = evalIntegerProperty(flowState, 2, 4, "Failed to evaluate Value in Arc widget");
        int32_t cur_val = lv_arc_get_value(objects.setpoint_cool);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_cool;
            lv_arc_set_value(objects.setpoint_cool, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 3, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.setpoint_heat, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_heat;
            if (new_val) lv_obj_add_flag(objects.setpoint_heat, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(objects.setpoint_heat, LV_OBJ_FLAG_HIDDEN);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = evalIntegerProperty(flowState, 3, 4, "Failed to evaluate Value in Arc widget");
        int32_t cur_val = lv_arc_get_value(objects.setpoint_heat);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_heat;
            lv_arc_set_value(objects.setpoint_heat, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = evalIntegerProperty(flowState, 4, 3, "Failed to evaluate Value in Arc widget");
        int32_t cur_val = lv_arc_get_value(objects.setpoint_actual);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_actual;
            lv_arc_set_value(objects.setpoint_actual, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 5, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.setpoint_cnt_text, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_cnt_text;
            if (new_val) lv_obj_add_flag(objects.setpoint_cnt_text, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(objects.setpoint_cnt_text, LV_OBJ_FLAG_HIDDEN);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 5, 4, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.setpoint_cnt_text);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.setpoint_cnt_text;
            lv_label_set_text(objects.setpoint_cnt_text, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 6, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.setpoint_cool_text, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_cool_text;
            if (new_val) lv_obj_add_flag(objects.setpoint_cool_text, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(objects.setpoint_cool_text, LV_OBJ_FLAG_HIDDEN);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 6, 4, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.setpoint_cool_text);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.setpoint_cool_text;
            lv_label_set_text(objects.setpoint_cool_text, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 7, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.setpoint_heat_text, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.setpoint_heat_text;
            if (new_val) lv_obj_add_flag(objects.setpoint_heat_text, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(objects.setpoint_heat_text, LV_OBJ_FLAG_HIDDEN);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 7, 4, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.setpoint_heat_text);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.setpoint_heat_text;
            lv_label_set_text(objects.setpoint_heat_text, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 8, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.actual_text);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.actual_text;
            lv_label_set_text(objects.actual_text, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 10, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.obj0);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.obj0;
            lv_label_set_text(objects.obj0, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 13, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.press_to_select, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.press_to_select;
            if (new_val) lv_obj_add_flag(objects.press_to_select, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(objects.press_to_select, LV_OBJ_FLAG_HIDDEN);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 14, 3, "Failed to evaluate Hidden flag");
        bool cur_val = lv_obj_has_flag(objects.press_to_confirm, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.press_to_confirm;
            if (new_val) lv_obj_add_flag(objects.press_to_confirm, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(objects.press_to_confirm, LV_OBJ_FLAG_HIDDEN);
            tick_value_change_obj = NULL;
        }
    }
}

void change_color_theme(uint32_t theme_index) {
    lv_style_set_bg_color(get_style_but_heat_MAIN_DEFAULT(), lv_color_hex(theme_colors[theme_index][0]));
    
    lv_style_set_bg_color(get_style_but_cool_MAIN_DEFAULT(), lv_color_hex(theme_colors[theme_index][2]));
    
    lv_style_set_bg_color(get_style_but_heatcool_MAIN_DEFAULT(), lv_color_hex(theme_colors[theme_index][1]));
    
    lv_style_set_bg_color(get_style_but_dry_MAIN_DEFAULT(), lv_color_hex(theme_colors[theme_index][3]));
    
    lv_obj_set_style_bg_color(objects.setpoint_cool, lv_color_hex(theme_colors[theme_index][2]), LV_PART_KNOB | LV_STATE_DEFAULT);
    
    lv_obj_set_style_arc_color(objects.setpoint_cool, lv_color_hex(theme_colors[theme_index][2]), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    
    lv_obj_set_style_arc_color(objects.setpoint_heat, lv_color_hex(theme_colors[theme_index][0]), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    
    lv_obj_set_style_bg_color(objects.setpoint_heat, lv_color_hex(theme_colors[theme_index][0]), LV_PART_KNOB | LV_STATE_DEFAULT);
    
    lv_obj_set_style_border_color(objects.setpoint_cool_text, lv_color_hex(theme_colors[theme_index][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_set_style_text_color(objects.setpoint_cool_text, lv_color_hex(theme_colors[theme_index][2]), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_set_style_border_color(objects.setpoint_heat_text, lv_color_hex(theme_colors[theme_index][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_set_style_text_color(objects.setpoint_heat_text, lv_color_hex(theme_colors[theme_index][0]), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_set_style_text_color(objects.press_to_select, lv_color_hex(theme_colors[theme_index][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_set_style_text_color(objects.press_to_confirm, lv_color_hex(theme_colors[theme_index][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_invalidate(objects.main);
}

extern void add_style(lv_obj_t *obj, int32_t styleIndex);
extern void remove_style(lv_obj_t *obj, int32_t styleIndex);

void ui_create_groups() {
    if (!groups_created) {
        groups.encoder_group = lv_group_create();
        eez_flow_init_groups((lv_group_t **)&groups, sizeof(groups) / sizeof(lv_group_t *));
        groups_created = true;
    }
}

static const char *screen_names[] = { "Main" };
static const char *object_names[] = { "main", "setpoint_cool", "button_mode", "press_to_select", "press_to_confirm", "setpoint_background", "setpoint_heat", "setpoint_actual", "setpoint_cnt_text", "setpoint_cool_text", "setpoint_heat_text", "actual_text", "button_settings", "obj0" };
static const char *group_names[] = { "encoder_group" };
static const char *style_names[] = { "but_heat", "but_cool", "but_heatcool", "but_dry", "but_fan", "but_default", "lbl_highlight", "lbl_default" };
static const char *theme_names[] = { "Default" };

uint32_t theme_colors[1][5] = {
    { 0xfff7480b, 0xffbbff00, 0xff4da9ff, 0xffffe45c, 0xffe6e38e },
};


typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    ui_create_groups();
    
    eez_flow_init_styles(add_style, remove_style);
    
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    eez_flow_init_group_names(group_names, sizeof(group_names) / sizeof(const char *));
    eez_flow_init_style_names(style_names, sizeof(style_names) / sizeof(const char *));
    eez_flow_init_themes(theme_names, sizeof(theme_names) / sizeof(const char *), change_color_theme);
    
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
