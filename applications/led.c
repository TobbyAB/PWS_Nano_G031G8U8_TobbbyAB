#include <agile_led.h>
#include <agile_beep.h>
#include "led.h"
#include "pin_config.h"

static agile_led_t *led_green = RT_NULL;

static agile_led_t *led_red = RT_NULL;
static agile_led_t *press_red = RT_NULL;
static agile_led_t *in_alarm_press_red = RT_NULL;
static agile_led_t *lost_red = RT_NULL;
static agile_led_t *in_check_led_red = RT_NULL;

static agile_beep_t *beep = RT_NULL;
static agile_beep_t *press_beep = RT_NULL;
static agile_beep_t *in_alarm_press_beep = RT_NULL;
static agile_beep_t *lost_beep = RT_NULL;
void led_Init(void)
{
    if (led_green == RT_NULL)
    {
        led_green = agile_led_create(LED_GREEN_PORT, LED_GREEN_PIN, 0, "200,0", -1);
    }
    if (led_red == RT_NULL)
    {
        led_red = agile_led_create(LED_RED_PORT, LED_RED_PIN, 0, "200,200", -1);
        in_check_led_red = agile_led_create(LED_RED_PORT, LED_RED_PIN, 0, "200,0", -1);
//        list_mem();
        press_red = agile_led_create(LED_RED_PORT, LED_RED_PIN, 0, "200,1", 1);
        in_alarm_press_red = agile_led_create(LED_RED_PORT, LED_RED_PIN, 0, "200,200,200,200,200,1", 1);
        lost_red = agile_led_create(LED_RED_PORT, LED_RED_PIN, 0, "200,200,200,5000", -1);
    }
    if (beep == RT_NULL)
    {
        beep = agile_beep_create("200,200", -1);
        press_beep = agile_beep_create("200,1", 1);
        in_alarm_press_beep = agile_beep_create("200,200,200,200,200,1", 1);
        lost_beep = agile_beep_create("200,200,200,5000", -1);
    }
}

void led_red_in_check(void)
{
    agile_led_stop(led_green);
    agile_led_start(in_check_led_red);
}

void led_red_out_check(void)
{
    agile_led_stop(in_check_led_red);
    agile_led_start(led_green);
}

void button_press(void)
{
    agile_beep_start(press_beep);
}
void in_alarm_press(void)
{
    agile_led_start(in_alarm_press_red);
    agile_beep_start(in_alarm_press_beep);
}
void in_alarm_press_calcel_sound(void)
{
    agile_beep_stop(beep);
}
void led_valve_open(void)
{
    agile_led_start(led_green);
}
void led_valve_close(void)
{
    agile_led_stop(led_green);
    agile_led_start(press_red);
}


void led_water_alarm(void)
{
    agile_led_set_light_mode(led_red,"200,200,200,200,200,5000", -1);
    agile_beep_set_light_mode(beep,"200,200,200,200,200,5000", -1);
    agile_led_stop(led_green);
    agile_led_start(led_red);
    agile_beep_start(beep);
}
void led_water_resume(void)
{
    agile_led_stop(led_red);
    agile_beep_stop(beep);
}
void led_lost_alarm(void)
{
    agile_led_stop(led_green);
    agile_led_start(lost_red);
    agile_beep_start(lost_beep);
}
void led_lost_resume(void)
{
    agile_led_stop(lost_red);
    agile_beep_stop(lost_beep);
    agile_led_start(led_green);
}
void led_valve_alarm(void)
{
    agile_led_stop(led_green);
    agile_led_stop(in_check_led_red);
    agile_led_set_light_mode(led_red, "200,200,200,200,200,200,200,200,200,200,200,5000", -1);
    agile_beep_set_light_mode(beep,"200,200,200,200,200,200,200,200,200,200,200,5000", -1);
    agile_led_start(led_red);
    agile_beep_start(beep);
}
void led_valve_resume(void)
{
    agile_led_stop(led_red);
    agile_beep_stop(beep);
}
