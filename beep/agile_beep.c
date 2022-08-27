#include <beep/agile_beep.h>
/*************************************************
 All rights reserved.
 File name:     agile_beep.c
 Description:
 History:
 1. Version:      v1.0.0
 Date:         2019-10-09
 Author:       Longwei Ma
 Modification: 新建版本
 *************************************************/

#include <stdlib.h>
#include <string.h>
#include "hardinit.h"

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME    "agile_beep"
#ifdef PKG_AGILE_BEEP_DEBUG
#define DBG_LEVEL           DBG_LOG
#else
#define DBG_LEVEL           DBG_INFO
#endif
#include <rtdbg.h>

// agile_beep 线程堆栈大小
#ifndef PKG_AGILE_BEEP_THREAD_STACK_SIZE
#define PKG_AGILE_BEEP_THREAD_STACK_SIZE 256
#endif

// agile_beep 线程优先级
#ifndef PKG_AGILE_BEEP_THREAD_PRIORITY
#define PKG_AGILE_BEEP_THREAD_PRIORITY RT_THREAD_PRIORITY_MAX - 4
#endif

// agile_beep 单向链表
static rt_slist_t agile_beep_list = RT_SLIST_OBJECT_INIT(agile_beep_list);
// agile_beep 互斥锁
static rt_mutex_t lock_mtx = RT_NULL;
// agile_beep 初始化完成标志
static uint8_t is_initialized = 0;

TIM_HandleTypeDef htim16;

/**
 * Name:             agile_beep_default_compelete_callback
 * Brief:            beep对象默认操作完成回调函数
 * Input:
 *   @beep:           agile_beep对象指针
 * Output:           none
 */
static void agile_beep_default_compelete_callback(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    //LOG_D("beep pin:%d compeleted.", beep->pin);
}

/**
 * Name:             agile_beep_get_light_arr
 * Brief:            获取beep对象闪烁模式
 * Input:
 *   @beep:           agile_beep对象指针
 *   @light_mode:    闪烁模式字符串
 * Output:           RT_EOK:成功
 *                   !=RT_EOK:异常
 */
static int agile_beep_get_light_arr(agile_beep_t *beep, const char *light_mode)
{
    RT_ASSERT(beep);
    RT_ASSERT(beep->light_arr == RT_NULL);
    RT_ASSERT(beep->arr_num == 0);
    const char *ptr = light_mode;
    while (*ptr)
    {
        if (*ptr == ',')
            beep->arr_num++;
        ptr++;
    }
    if (*(ptr - 1) != ',')
        beep->arr_num++;

    if (beep->arr_num == 0)
        return -RT_ERROR;

    beep->light_arr = (uint32_t *) rt_malloc(beep->arr_num * sizeof(uint32_t));
    if (beep->light_arr == RT_NULL)
        return -RT_ENOMEM;
    rt_memset(beep->light_arr, 0, beep->arr_num * sizeof(uint32_t));
    ptr = light_mode;
    for (uint32_t i = 0; i < beep->arr_num; i++)
    {
        beep->light_arr[i] = atoi(ptr);
        ptr = strchr(ptr, ',');
        if (ptr)
            ptr++;
    }
    return RT_EOK;
}

/**
 * Name:             agile_beep_create
 * Brief:            创建beep对象
 * Input:
 *   @pin:           控制beep的引脚
 *   @active_logic:  beep有效电平(PIN_HIGH/PIN_LOW)
 *   @light_mode:    闪烁模式字符串
 *   @loop_cnt:      循环次数
 * Output:           !=RT_NULL:agile_beep对象指针
 *                   RT_NULL:异常
 */
//agile_beep_t *agile_beep_create(GPIO_TypeDef *GPIOx,rt_base_t pin, rt_base_t active_logic, const char *light_mode, int32_t loop_cnt)
agile_beep_t *agile_beep_create(const char *light_mode, int32_t loop_cnt)
{
    if (!is_initialized)
    {
        LOG_E("Agile beep haven't initialized!");
        return RT_NULL;
    }
    agile_beep_t *beep = (agile_beep_t *) rt_malloc(sizeof(agile_beep_t));
    if (beep == RT_NULL)
        return RT_NULL;
    beep->active = 0;
//    beep->pin = pin;
//    beep->active_logic = active_logic;
    beep->light_arr = RT_NULL;
    beep->arr_num = 0;
    beep->arr_index = 0;
    if (light_mode)
    {
        if (agile_beep_get_light_arr(beep, light_mode) < 0)
        {
            rt_free(beep);
            return RT_NULL;
        }
    }
    beep->loop_init = loop_cnt;
    beep->loop_cnt = beep->loop_init;
    beep->tick_timeout = rt_tick_get();
    beep->compelete = agile_beep_default_compelete_callback;
    rt_slist_init(&(beep->slist));

    Hard_TIM16_Init();

    return beep;
}

/**
 * Name:             agile_beep_delete
 * Brief:            删除beep对象
 * Input:
 *   @beep:           beep对象指针
 * Output:           RT_EOK:成功
 */
int agile_beep_delete(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    rt_slist_remove(&(agile_beep_list), &(beep->slist));
    beep->slist.next = RT_NULL;
    rt_mutex_release(lock_mtx);
    if (beep->light_arr)
    {
        rt_free(beep->light_arr);
        beep->light_arr = RT_NULL;
    }
    rt_free(beep);
    return RT_EOK;
}

/**
 * Name:             agile_beep_start
 * Brief:            启动beep对象,根据设置的模式执行动作
 * Input:
 *   @beep:           beep对象指针
 * Output:           RT_EOK:成功
 *                   !=RT_OK:异常
 */
int agile_beep_start(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    if (beep->active)
    {
        rt_mutex_release(lock_mtx);
        return -RT_ERROR;
    }
    if ((beep->light_arr == RT_NULL) || (beep->arr_num == 0))
    {
        rt_mutex_release(lock_mtx);
        return -RT_ERROR;
    }
    beep->arr_index = 0;
    beep->loop_cnt = beep->loop_init;
    beep->tick_timeout = rt_tick_get();
    rt_slist_append(&(agile_beep_list), &(beep->slist));
    beep->active = 1;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
 * Name:             agile_beep_stop
 * Brief:            停止beep对象
 * Input:
 *   @beep:           beep对象指针
 * Output:           RT_EOK:成功
 */
int agile_beep_stop(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    if (!beep->active)
    {
        rt_mutex_release(lock_mtx);
        return RT_EOK;
    }
    rt_slist_remove(&(agile_beep_list), &(beep->slist));
    beep->slist.next = RT_NULL;
    beep->active = 0;
    rt_mutex_release(lock_mtx);
//    HAL_GPIO_WritePin(GPIOA,beep->pin,!beep->active_logic);
    Beep_off();
    return RT_EOK;
}

int agile_beep_pause(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    beep->pin_backup = beep->pin;
    beep->pin = 0;
    beep->save_logic = beep->now_logic;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

int agile_beep_resume(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    beep->pin = beep->pin_backup;
    if (beep->save_logic == beep->active_logic)
    {
        agile_beep_on(beep);
    }
    else
    {
        agile_beep_off(beep);
    }
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}
/**
 * Name:             agile_beep_set_light_mode
 * Brief:            设置beep对象的模式
 * Input:
 *   @beep:           beep对象指针
 *   @light_mode:    闪烁模式字符串
 *   @loop_cnt:      循环次数
 * Output:           RT_EOK:成功
 *                   !=RT_EOK:异常
 */
int agile_beep_set_light_mode(agile_beep_t *beep, const char *light_mode, int32_t loop_cnt)
{
    RT_ASSERT(beep);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);

    if (light_mode)
    {
        if (beep->light_arr)
        {
            rt_free(beep->light_arr);
            beep->light_arr = RT_NULL;
        }
        beep->arr_num = 0;
        if (agile_beep_get_light_arr(beep, light_mode) < 0)
        {
            agile_beep_stop(beep);
            rt_mutex_release(lock_mtx);
            return -RT_ERROR;
        }
    }
    beep->loop_init = loop_cnt;
    beep->arr_index = 0;
    beep->loop_cnt = beep->loop_init;
    beep->tick_timeout = rt_tick_get();
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
 * Name:             agile_beep_set_compelete_callback
 * Brief:            设置beep对象操作完成的回调函数
 * Input:
 *   @beep:           beep对象指针
 *   @compelete:     操作完成回调函数
 * Output:           RT_EOK:成功
 */
int agile_beep_set_compelete_callback(agile_beep_t *beep, void (*compelete)(agile_beep_t *beep))
{
    RT_ASSERT(beep);
    rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
    beep->compelete = compelete;
    rt_mutex_release(lock_mtx);
    return RT_EOK;
}

/**
 * Name:             agile_beep_toggle
 * Brief:            beep对象电平翻转
 * Input:
 *   @beep:           beep对象指针
 * Output:           none
 */
void agile_beep_toggle(agile_beep_t *beep)
{
    RT_ASSERT(beep);
//    HAL_GPIO_WritePin(GPIOA, beep->pin, !HAL_GPIO_ReadPin(GPIOA, beep->pin));
}

/**
 * Name:             agile_beep_on
 * Brief:            beep对象亮
 * Input:
 *   @beep:           beep对象指针
 * Output:           none
 */
void agile_beep_on(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    beep->now_logic = beep->active_logic;
//    HAL_GPIO_WritePin(GPIOA,beep->pin,beep->active_logic);
    Beep_on();
}

/**
 * Name:             agile_beep_off
 * Brief:            beep对象灭
 * Input:
 *   @beep:           beep对象指针
 * Output:           none
 */
void agile_beep_off(agile_beep_t *beep)
{
    RT_ASSERT(beep);
    beep->now_logic = !beep->active_logic;
//    HAL_GPIO_WritePin(GPIOA,beep->pin,!beep->active_logic);
    Beep_off();
}

void Beep_on(void)
{
    HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
}
void Beep_off(void)
{
    HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
}

/**
 * Name:             beep_process
 * Brief:            agile_beep线程
 * Input:
 *   @parameter:     线程参数
 * Output:           none
 */
static void beep_process(void *parameter)
{
    rt_slist_t *node;
    while (1)
    {
        rt_mutex_take(lock_mtx, RT_WAITING_FOREVER);
        rt_slist_for_each(node, &(agile_beep_list))
        {
            agile_beep_t *beep = rt_slist_entry(node, agile_beep_t, slist);
            if (beep->loop_cnt == 0)
            {
                agile_beep_stop(beep);
                if (beep->compelete)
                {
                    beep->compelete(beep);
                }
                node = &agile_beep_list;
                continue;
            }
            __repeat: if ((rt_tick_get() - beep->tick_timeout) < (RT_TICK_MAX / 2))
            {
                if (beep->arr_index < beep->arr_num)
                {
                    if (beep->light_arr[beep->arr_index] == 0)
                    {
                        beep->arr_index++;
                        goto __repeat;
                    }
                    if (beep->arr_index % 2)
                    {
                        agile_beep_off(beep);
                    }
                    else
                    {
                        agile_beep_on(beep);
                    }
                    beep->tick_timeout = rt_tick_get() + rt_tick_from_millisecond(beep->light_arr[beep->arr_index]);
                    beep->arr_index++;
                }
                else
                {
                    beep->arr_index = 0;
                    if (beep->loop_cnt > 0)
                        beep->loop_cnt--;
                }
            }
        }
        rt_mutex_release(lock_mtx);
        rt_thread_mdelay(5);
    }
}

/**
 * Name:             agile_beep_init
 * Brief:            agile_beep初始化
 * Input:            none
 * Output:           RT_EOK:成功
 *                   !=RT_EOK:失败
 */
static int agile_beep_init(void)
{
    rt_thread_t tid = RT_NULL;
    lock_mtx = rt_mutex_create("beep_mtx", RT_IPC_FLAG_FIFO);
    if (lock_mtx == RT_NULL)
    {
        LOG_E("Agile beep initialize faibeep! lock_mtx create faibeep!");
        return -RT_ENOMEM;
    }

    tid = rt_thread_create("agile_beep", beep_process, RT_NULL,
    PKG_AGILE_BEEP_THREAD_STACK_SIZE, PKG_AGILE_BEEP_THREAD_PRIORITY, 100);
    if (tid == RT_NULL)
    {
        LOG_E("Agile beep initialize faibeep! thread create faibeep!");
        rt_mutex_delete(lock_mtx);
        return -RT_ENOMEM;
    }
    rt_thread_startup(tid);
    is_initialized = 1;
    return RT_EOK;
}
INIT_APP_EXPORT(agile_beep_init);
