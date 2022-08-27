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
#include "stm32g0xx.h"

uint8_t WarningNowStatus=0;
uint8_t WarningPastStatus=0;
uint8_t WarningStatus=0;

uint8_t water_alarm_state;
uint8_t water_alarm_flag;

rt_thread_t WaterScan_t = RT_NULL;

void WarningWithPeak(uint8_t status)
{
    switch(status)
    {
    case 0://测水线掉落恢复
        led_lost_resume();
        break;
    case 1://测水线掉落
        if(!water_alarm_flag)
        {
           led_lost_alarm();
        }
        break;
    case 2://测水线短路
        water_alarm_flag = 1;
        water_alarm_state = 1;
        led_water_alarm();
        valve_close();
        break;
    case 3://测水线短路解除
        water_alarm_state = 0;
        //MasterStatusChangeToDeAvtive();
        break;
    }
}
void WaterScan_Clear(void)
{
    WarningPastStatus=0;
    WarningNowStatus=0;
    WarningStatus = 0;
}
uint8_t Water_Alarm_Pause;
uint8_t Get_Peak_ON_Level(void)
{
    if(!Water_Alarm_Pause)
    {
        return HAL_GPIO_ReadPin(WATER_LEAK_PORT,WATER_LEAK_PIN);
    }
    else {
        return 1;
    }
}
uint8_t Get_Peak_LOSS_Level(void)
{
    if(!Water_Alarm_Pause)
    {
        return HAL_GPIO_ReadPin(WATER_LOS_PORT,WATER_LOS_PIN);
    }
    else {
        return 0;
    }
}
void WaterScan_Callback(void *parameter)
{
    uint8_t Peak_ON_Level=0;
    uint8_t Peak_Loss_Level=0;
    rt_thread_mdelay(200);
    while(1)//插入是0，短路是0
    {
        if (Get_Peak_LOSS_Level() == 1)
        {
            rt_thread_mdelay(50);
            if (Get_Peak_LOSS_Level() == 1)
            {
                Peak_Loss_Level = 1;
            }
        }
        else
            Peak_Loss_Level = 0;

        if (Get_Peak_ON_Level() == 0)
        {
            rt_thread_mdelay(50);
            if (Get_Peak_ON_Level() == 0)
            {
                Peak_ON_Level = 0;
            }
        }
        else
            Peak_ON_Level = 1;

        if(Peak_Loss_Level!=0)
        {
            WarningNowStatus=1;//测水线掉落
        }
        else
        {
            if(Peak_ON_Level==0)
            {
                WarningNowStatus=2;//测水线短路
            }
            else
            {
                WarningNowStatus=0;//状态正常
            }
        }

        if(WarningNowStatus != WarningPastStatus)
        {
            if(WarningPastStatus==2 && WarningNowStatus==0)
            {
                if(WarningStatus != 1<<0)
                {
                    WarningStatus = 1<<0;
                    WarningWithPeak(3);
                }
            }
            else if(WarningPastStatus==2 && WarningNowStatus==1)
            {
                if(WarningStatus != 1<<1)
                {
                    WarningStatus = 1<<1;
                }
            }
            else if(WarningPastStatus==0 && WarningNowStatus==1)
            {
                if(WarningStatus != 1<<2)
                {
                    WarningWithPeak(1);
                    WarningPastStatus = WarningNowStatus;
                    WarningStatus = 1<<2;
                }
            }
            else if(WarningPastStatus==0 && WarningNowStatus==2)
            {
                if(WarningStatus != 1<<3)
                {
                    WarningWithPeak(2);
                    WarningPastStatus = WarningNowStatus;
                    WarningStatus = 1<<3;
                }
            }
            else if(WarningPastStatus==1 && WarningNowStatus==0)
            {
                if(WarningStatus != 1<<4)
                {
                    WarningWithPeak(0);
                    WarningPastStatus = WarningNowStatus;
                    WarningStatus = 1<<4;
                }
            }
        }
        rt_thread_mdelay(50);
    }
}

void WaterScan_IO_Init(void)
{
    Water_Alarm_Pause = 0;
}
void WaterScan_IO_DeInit(void)
{
    Water_Alarm_Pause = 1;
}
void WaterScan_Init(void)
{
    GPIO_InitTypeDef  gpio_init_structure = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin   = WATER_LOS_PIN|WATER_LEAK_PIN;
    gpio_init_structure.Mode  = GPIO_MODE_INPUT;
    gpio_init_structure.Pull  = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(WATER_LOS_PORT, &gpio_init_structure);
    WaterScan_IO_Init();

    WaterScan_t = rt_thread_create("WaterScan", WaterScan_Callback, RT_NULL, 256, 10, 10);
    if(WaterScan_t!=RT_NULL)rt_thread_startup(WaterScan_t);

}
