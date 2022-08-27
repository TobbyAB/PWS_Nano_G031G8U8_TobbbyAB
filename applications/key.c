/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-25     Rick       the first version
 */
#include <rthw.h>
#include <rtthread.h>

#include "pin_config.h"
#include "key.h"
#include "button.h"

Button_t Key0;

rt_thread_t button_task = RT_NULL;

extern uint8_t water_alarm_flag; //报警标志
extern uint8_t water_alarm_state; //当前状态
extern uint8_t Valve_Alarm_Flag;

void Key_IO_Init(void)
{
    GPIO_InitTypeDef gpio_init_structure = { 0 };
    __HAL_RCC_GPIOA_CLK_ENABLE()
    ;
    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin = KEY_PIN;
    gpio_init_structure.Mode = GPIO_MODE_INPUT;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(KEY_PORT, &gpio_init_structure);
}
void Key_IO_DeInit(void)
{
    GPIO_InitTypeDef gpio_init_structure = { 0 };
    __HAL_RCC_GPIOA_CLK_ENABLE()
    ;
    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin = KEY_PIN;
    gpio_init_structure.Mode = GPIO_MODE_ANALOG;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(KEY_PORT, &gpio_init_structure);
}
uint8_t Read_RESET_Level(void)
{
    return HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN);
}

void K0_Sem_Release(void *parameter) //off
{
    if (water_alarm_state)
    {
        in_alarm_press_calcel_sound();
    }
    else if (Valve_Alarm_Flag)
    {
        in_alarm_press();
    }
    else if (Get_ValveNowStatus() == 1)
    {
        led_red_in_check();
        Moto_Detect();
    }
    else
    {
        water_alarm_flag = 0;
        WaterScan_Clear();
        led_water_resume();
        button_press();
        valve_open();
    }
}

void button_task_entry(void *parameter)
{
    Key_IO_Init();
    Button_Create("RESET", &Key0, Read_RESET_Level, 0);
    Button_Attach(&Key0, BUTTON_DOWM, K0_Sem_Release);
    while (1)
    {
        Button_Process();
        rt_thread_mdelay(10);
    }
}
void button_Init(void)
{
    button_task = rt_thread_create("button_task", button_task_entry, RT_NULL, 512, 5, 10);
    if (button_task != RT_NULL)
        rt_thread_startup(button_task);
}
