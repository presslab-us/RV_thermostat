#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_NONE
};

// Native global variables

extern int32_t get_var_setpoint_cool();
extern void set_var_setpoint_cool(int32_t value);
extern int32_t get_var_setpoint_heat();
extern void set_var_setpoint_heat(int32_t value);
extern int32_t get_var_mode();
extern void set_var_mode(int32_t value);
extern int32_t get_var_actual();
extern void set_var_actual(int32_t value);
extern int32_t get_var_enc_state();
extern void set_var_enc_state(int32_t value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/