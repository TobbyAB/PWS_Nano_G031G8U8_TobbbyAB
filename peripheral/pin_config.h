/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-25     Rick       the first version
 */
#ifndef APPLICATIONS_PIN_CONFIG_H_
#define APPLICATIONS_PIN_CONFIG_H_

#include "stm32g0xx.h"

//OUTPUT
#define LED_GREEN_PORT          GPIOA
#define LED_GREEN_PIN           GPIO_PIN_11
#define LED_RED_PORT            GPIOA
#define LED_RED_PIN             GPIO_PIN_12

#define BUZZER_PORT             GPIOB
#define BUZZER_PIN              GPIO_PIN_8

#define VALVE_1_PORT            GPIOA
#define VALVE_1_PIN             GPIO_PIN_1
#define VALVE_2_PORT            GPIOA
#define VALVE_2_PIN             GPIO_PIN_3
//INPUT
#define HALL_1_PORT             GPIOB
#define HALL_1_PIN              GPIO_PIN_5
#define HALL_2_PORT             GPIOB
#define HALL_2_PIN              GPIO_PIN_4

#define KEY_PORT                GPIOA
#define KEY_PIN                 GPIO_PIN_15


#define WATER_LOS_PORT          GPIOB
#define WATER_LOS_PIN           GPIO_PIN_6

#define WATER_LEAK_PORT         GPIOB
#define WATER_LEAK_PIN          GPIO_PIN_7

#define RELAY_ON_PORT           GPIOC
#define RELAY_ON_PIN            GPIO_PIN_6

#define RELAY_OFF_PORT          GPIOA
#define RELAY_OFF_PIN           GPIO_PIN_2

#endif /* APPLICATIONS_PIN_CONFIG_H_ */
