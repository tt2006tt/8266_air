#ifndef __KEY_H__
#define __KEY_H__

#include "gpio.h"

typedef void (* key_function)(void);

struct single_key_param {
    uint8 key_level;
    uint8 gpio_id;
    uint8 gpio_func;
    uint32 gpio_name;
    os_timer_t key_1_5s;
    os_timer_t key_50ms;
    key_function short_press;
    key_function long_press;
};

struct keys_param {
    uint8 key_num;
    struct single_key_param **single_key;
};

#define Gas_Pin_NUM         5
#define Gas_Pin_FUNC        FUNC_GPIO5
#define Gas_Pin_MUX         PERIPHS_IO_MUX_GPIO5_U

#define Gas_Pin_Rd_Init()   GPIO_DIS_OUTPUT(SWITCH_Pin_NUM)
#define Gas_Pin_Wr_Init()   GPIO_OUTPUT_SET(SWITCH_Pin_NUM,0)
#define Gas_Pin_Set_High()  GPIO_OUTPUT_SET(SWITCH_Pin_NUM,1)
#define Gas_Pin_Set_Low()   GPIO_OUTPUT_SET(SWITCH_Pin_NUM,0)
#define Gas_Pin_State       ( GPIO_INPUT_GET(SWITCH_Pin_NUM) != 0 )

struct single_key_param *key_init_single(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func, key_function long_press, key_function short_press);
void key_init(struct keys_param *key);

#endif
