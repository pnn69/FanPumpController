
/*
 * global.h
 *
 */
#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "sdkconfig.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <driver/i2c.h>
#include <driver/periph_ctrl.h>
#include <driver/timer.h>
#include <esp_adc_cal.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_types.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <soc/timer_group_struct.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "rs485.h"
#include "i2c.h"
#include "display.h"
#include "touch.h"


#define HW_VERSION "1.7"
#define SW_VERSION "1.0"

#define ZerroCrossPin 35

#define OneSec pdMS_TO_TICKS(1000)

#define UP 1
#define DOWN 2
#define ENTER 3

#define TOUCH_PAD_2   3
#define TOUCH_PAD_1   4
#define TOUCH_PAD_0   5

#define ledFan 		0 // pos led Fan IO extender
#define ledRH 		1 // pos led RH IO extender
#define ledHeater 	2 // pos led Heater IO extender

#define AC_pin1 33       // Output to Opto Triac1
#define AC_pin2 25       // Output to Opto Triac2
#define AC_pin3 32       // Output to Opto Triac3

#define modeFanAuxBox 1
#define modeFanAuxBoxRetro 2
#define modeFanPumpController 3
#define modeFanPumpBoxRetro 4

#define Tout 1
#define Tin 0
extern bool onesec;
extern int lstcnt;

TimerHandle_t tmr;


#endif /* GLOBAL_H_ */
