#include <beep/agile_beep.h>
#include <rtthread.h>
#include "pin_config.h"

#include "key.h"
#include "led.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

int main(void)
{
    button_Init();
    led_Init();
    WaterScan_Init();
    Moto_Init();
    RTC_Init();
    Watchdog_Init();

    while (1)
    {
        Watchdog_Refresh();
//        set_relay_on();
//        rt_thread_mdelay(1000);
//        set_relay_off();
        rt_thread_mdelay(1000);
    }
    return RT_EOK;
}
