#ifndef __PKG_AGILE_BEEP_H
#define __PKG_AGILE_BEEP_H
#include <rtthread.h>
#include <stm32g0xx.h>

#ifdef __cplusplus
extern "C" {
#endif

// agile_beep 结构体
typedef struct agile_beep agile_beep_t;

struct agile_beep
{
    uint8_t active;                                  // 激活标志
    rt_base_t pin;                                   // 控制引脚
    rt_base_t active_logic;                          // 有效电平(PIN_HIGH/PIN_LOW)
    rt_base_t now_logic;                             // 当前电平(PIN_HIGH/PIN_LOW)
    rt_base_t save_logic;                            // 存储电平(PIN_HIGH/PIN_LOW)
    uint32_t *light_arr;                             // 闪烁数组
    uint32_t arr_num;                                // 数组元素数目
    uint32_t arr_index;                              // 数组索引
    int32_t loop_init;                               // 循环次数
    int32_t loop_cnt;                                // 循环次数计数
    rt_tick_t tick_timeout;                          // 超时时间
    void (*compelete)(agile_beep_t *beep);             // 操作完成回调函数
    rt_slist_t slist;                                // 单向链表节点
    rt_base_t pin_backup;                            // 备份控制引脚

};

// 创建beep对象
agile_beep_t *agile_beep_create(const char *light_mode, int32_t loop_cnt);
// 删除beep对象
int agile_beep_delete(agile_beep_t *beep);
// 启动beep对象,根据设置的模式执行动作
int agile_beep_start(agile_beep_t *beep);
// 停止beep对象
int agile_beep_stop(agile_beep_t *beep);
// 设置beep对象的模式
int agile_beep_set_light_mode(agile_beep_t *beep, const char *light_mode, int32_t loop_cnt);
// 设置beep对象操作完成的回调函数
int agile_beep_set_compelete_callback(agile_beep_t *beep, void (*compelete)(agile_beep_t *beep));
// beep对象电平翻转
void agile_beep_toggle(agile_beep_t *beep);
// beep对象亮
void agile_beep_on(agile_beep_t *beep);
// beep对象灭
void agile_beep_off(agile_beep_t *beep);

int agile_beep_pause(agile_beep_t *beep);

int agile_beep_resume(agile_beep_t *beep);

#ifdef __cplusplus
}
#endif

#endif
