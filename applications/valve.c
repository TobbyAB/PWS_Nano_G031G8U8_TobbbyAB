/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-26     Rick       the first version
 */
#include "rtthread.h"
#include "pin_config.h"

rt_timer_t Moto1_Timer_Act, Moto2_Timer_Act = RT_NULL;
rt_timer_t Moto1_Timer_Detect, Moto2_Timer_Detect = RT_NULL;
rt_timer_t MotoCheck_Outimer = RT_NULL;

uint8_t Turn1_Flag;
uint8_t Turn2_Flag;

uint8_t ValvePastStatus;
uint8_t ValveNowStatus;

uint8_t Moto1_Fail_FLag;
uint8_t Moto2_Fail_FLag;

uint8_t Moto_Open_Status;

uint8_t Valve_Alarm_Flag;

uint32_t count = 0;

void relay_init(void)
{
    GPIO_InitTypeDef gpio_init_structure = { 0 };
    __HAL_RCC_GPIOC_CLK_ENABLE()
    ;
    __HAL_RCC_GPIOA_CLK_ENABLE()
    ;
    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin = GPIO_PIN_6 | GPIO_PIN_2;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio_init_structure);
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);
}

void set_relay_on()
{
    HAL_GPIO_WritePin(RELAY_ON_PORT, RELAY_ON_PIN, 0);
    HAL_GPIO_WritePin(RELAY_OFF_PORT, RELAY_OFF_PIN, 0);
    HAL_GPIO_WritePin(RELAY_ON_PORT, RELAY_ON_PIN, 1);
    rt_thread_mdelay(200);
    HAL_GPIO_WritePin(RELAY_ON_PORT, RELAY_ON_PIN, 0);
}
void set_relay_off()
{
    HAL_GPIO_WritePin(RELAY_ON_PORT, RELAY_ON_PIN, 0);
    HAL_GPIO_WritePin(RELAY_OFF_PORT, RELAY_OFF_PIN, 0);
    HAL_GPIO_WritePin(RELAY_OFF_PORT, RELAY_OFF_PIN, 1);
    rt_thread_mdelay(200);
    HAL_GPIO_WritePin(RELAY_OFF_PORT, RELAY_OFF_PIN, 0);
}

void valve_init(void)
{
    GPIO_InitTypeDef gpio_init_structure = { 0 };
    __HAL_RCC_GPIOA_CLK_ENABLE()
    ;
    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin = VALVE_1_PIN | VALVE_2_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio_init_structure);
}

void valve_open(void)
{
//    led_valve_open();
    ValvePastStatus = ValveNowStatus;
    ValveNowStatus = 1;
    HAL_GPIO_WritePin(GPIOA, VALVE_1_PIN | VALVE_2_PIN, 1);
    rt_timer_start(MotoCheck_Outimer);
    set_relay_on();
}
void valve_close(void)
{
    led_valve_close();
    ValvePastStatus = ValveNowStatus;
    ValveNowStatus = 0;
    Moto_Open_Status = 0;
    HAL_GPIO_WritePin(GPIOA, VALVE_1_PIN | VALVE_2_PIN, 0);
    set_relay_off();
}
void Turn1_Edge_Callback(void)
{
//    led_valve_alarm();
    Turn1_Flag++;
}
void Turn2_Edge_Callback(void)
{
//    led_valve_alarm();
    Turn2_Flag++;
}

uint8_t Get_ValveNowStatus(void)
{
    return Moto_Open_Status;
}

void Change_ValveNowStatus(void)
{
     Moto_Open_Status = 1;
}

uint8_t Get_Moto1_Fail_FLag(void)
{
    return Moto1_Fail_FLag;
}
uint8_t Get_Moto2_Fail_FLag(void)
{
    return Moto2_Fail_FLag;
}
void Turn1_Timer_Callback(void)
{
//    rt_kprintf("Turn1_Timer_Callback1\n");
//    Key_IO_Init();
//    WaterScan_IO_Init();
////    HAL_GPIO_WritePin(GPIOA, VALVE_1_PIN, 1);
//    rt_kprintf("Turn1_Timer_Callback2\n");
//    if (Turn1_Flag < 2)
//    {
//        rt_kprintf("turun1\n");
//        if (!Moto2_Fail_FLag)
//        {
//            rt_kprintf("Turn1_Flag = %d\n", Turn1_Flag);
//            led_valve_alarm();
//            rt_kprintf("turun3\n");
//            Valve_Alarm_Flag = 1;
//        }
//        Moto1_Fail_FLag = 1;
//    }
//    else
//    {
//        rt_kprintf("turun2\n");
//        if (!Moto2_Fail_FLag)
//        {
//            led_valve_resume();
//            Valve_Alarm_Flag = 0;
//        }
//        Moto1_Fail_FLag = 0;
//    }
//    led_red_out_check();

    Key_IO_Init();
    WaterScan_IO_Init();
    if (Turn1_Flag < 2)
    {
        led_valve_alarm();
        Moto1_Fail_FLag = 1;
        Valve_Alarm_Flag = 1;
    }
    else
    {
        led_valve_resume();
        Moto1_Fail_FLag = 0;
        Valve_Alarm_Flag = 0;
        led_red_out_check();
    }

}
void Turn2_Timer_Callback(void)
{
    Key_IO_Init();
    WaterScan_IO_Init();
    if (Turn2_Flag < 2)
    {
        led_valve_alarm();
        Moto2_Fail_FLag = 1;
        Valve_Alarm_Flag = 1;
    }
    else
    {
        led_valve_resume();
        Moto2_Fail_FLag = 0;
        Valve_Alarm_Flag = 0;
        led_red_out_check();
    }
}
void EXTI4_15_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI4_15_IRQn 0 */

    /* USER CODE END EXTI4_15_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    /* USER CODE BEGIN EXTI4_15_IRQn 1 */

    /* USER CODE END EXTI4_15_IRQn 1 */
}
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
    case GPIO_PIN_5:
        Turn1_Edge_Callback();
        break;
    case GPIO_PIN_4:
        Turn2_Edge_Callback();
        break;
    default:
        break;
    }
}

void Moto1_Timer_Act_Callback(void *parameter)
{
    HAL_GPIO_WritePin(GPIOA, VALVE_1_PIN, 1);
    rt_timer_start(Moto1_Timer_Detect);
}

void Moto2_Timer_Act_Callback(void *parameter)
{
    HAL_GPIO_WritePin(GPIOA, VALVE_2_PIN, 1);
    rt_timer_start(Moto2_Timer_Detect);
}

void MotoCheck_Outimer_Callback(void *parameter)
{
    Moto_Open_Status = 1;
//    button_press();
//    led_valve_close();
}

void Moto_Init(void)
{
    valve_init();
    relay_init();
//    led_water_alarm();
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE()
    ;
    __HAL_RCC_GPIOA_CLK_ENABLE()
    ;

    /*Configure GPIO pins : PA4 PA5 */
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
//    led_water_alarm();
    Moto1_Timer_Act = rt_timer_create("Moto1_Timer_Act", Moto1_Timer_Act_Callback, RT_NULL, 5100,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
//    led_water_alarm();
    Moto2_Timer_Act = rt_timer_create("Moto2_Timer_Act", Moto2_Timer_Act_Callback, RT_NULL, 5000,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    Moto1_Timer_Detect = rt_timer_create("Moto1_Timer_Detect", Turn1_Timer_Callback, RT_NULL, 5000,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    Moto2_Timer_Detect = rt_timer_create("Moto2_Timer_Detect", Turn2_Timer_Callback, RT_NULL, 5000,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    MotoCheck_Outimer = rt_timer_create("MotoCheck_Outimer", MotoCheck_Outimer_Callback, RT_NULL, 11000,
    RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

//    list_mem();

    valve_open();

//    button_press();
//    led_water_alarm();
}
void Moto_Detect(void)
{
    uint8_t ValveFuncFlag = ValveNowStatus;
    rt_kprintf("ValveFuncFlag = %d ,count = %d \n", ValveFuncFlag, count++);
    led_valve_resume();
    if (ValveFuncFlag == 1)
    {
        Turn1_Flag = 0;
        Turn2_Flag = 0;
        Moto1_Fail_FLag = 0;
        Moto2_Fail_FLag = 0;
        if (HAL_GPIO_ReadPin(GPIOB, HALL_1_PIN))
        {
            led_red_in_check();
            rt_kprintf("Actuater_1 check start .. \n");
            Key_IO_DeInit();
            WaterScan_IO_DeInit();
            HAL_GPIO_WritePin(GPIOA, VALVE_1_PIN, 0);
            rt_timer_start(Moto1_Timer_Act);
        }
        if (HAL_GPIO_ReadPin(GPIOB, HALL_2_PIN))
        {
            led_red_in_check();
            rt_kprintf("Actuater_2 check start .. \n");
            Key_IO_DeInit();
            WaterScan_IO_DeInit();
            HAL_GPIO_WritePin(GPIOA, VALVE_2_PIN, 0);
            rt_timer_start(Moto2_Timer_Act);
        }
    }
}
