/*
    timer stuff
*/
#include "global.h"
#include <stdbool.h>
#include "adc.h"

#define GPIO_INPUT_PIN_SEL (1ULL << ZerroCrossPin)
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_OUTPUT_PIN_SEL ((1ULL << AC_pin1) | (1ULL << AC_pin2) | (1ULL << AC_pin3))


static intr_handle_t s_timer_handle;
TimerHandle_t tmr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile char tenth;  // timing parameter
int dim1 = 0;
int dim2 = 0;
int dim3 = 0;

bool Hz50 = true;
//static float voltage = 0;
static bool ZerroCross;
static int cnt = 0;
int lstcnt;
bool enableOutput = false;


/*
 * Stuff to make the phase cut output linear
 */


float IRAM_ATTR fan_lineairCompFan(double measP, double botvalueP, double topV, double nextValueV)
    {
    double diffV = topV - nextValueV;
    double perVP = diffV / 5.0;
    double diffP = measP - botvalueP;
    return topV - (diffP * perVP);
    }


float IRAM_ATTR fan_calcPercentageToVoltage(float measP ) // percentage in voltage out
    {
    float returvalue = 10.0;
    if (measP <= 10)
        returvalue =  fan_lineairCompFan(measP, 10, 9.79, 9.49);

    else if (measP <= 15)
        returvalue =  fan_lineairCompFan(measP, 15, 9.49, 9.19);

    else if (measP <= 20)
        returvalue =  fan_lineairCompFan(measP, 20, 9.19, 8.79);

    else if (measP <= 25)
        returvalue =  fan_lineairCompFan(measP, 25, 8.79, 8.39);

    else if (measP <= 30)
        returvalue =  fan_lineairCompFan(measP, 30, 8.39, 7.99);

    else if (measP <= 35)
        returvalue =  fan_lineairCompFan(measP, 35, 7.99, 7.59);

    else if (measP <= 40)
        returvalue =  fan_lineairCompFan(measP, 40, 7.59, 7.19);

    else if (measP <= 45)
        returvalue =  fan_lineairCompFan(measP, 45, 7.19, 6.79);

    else if (measP <= 50)
        returvalue =  fan_lineairCompFan(measP, 50, 6.79, 6.39);

    else if (measP <= 55)
        returvalue =  fan_lineairCompFan(measP, 55, 6.39, 5.99);

    else if (measP <= 60)
        returvalue =  fan_lineairCompFan(measP, 60, 5.99, 5.59);

    else if (measP <= 65)
        returvalue =  fan_lineairCompFan(measP, 65, 5.59, 5.14);

    else if (measP <= 70)
        returvalue =  fan_lineairCompFan(measP, 70, 5.14, 4.64);

    else if (measP <= 75)
        returvalue =  fan_lineairCompFan(measP, 75, 4.64, 4.07);

    else if (measP <= 80)
        returvalue =  fan_lineairCompFan(measP, 80, 4.07, 3.40);

    else if (measP <= 85)
        returvalue =  fan_lineairCompFan(measP, 85, 3.40, 2.63);

    else if (measP <= 90)
        returvalue =  fan_lineairCompFan(measP, 85, 2.63, 1.80);

    else if (measP <= 95)
        returvalue =  fan_lineairCompFan(measP, 95, 1.80, 0.93);

    else if (measP <= 97)
        returvalue =  fan_lineairCompFan(measP, 99, 0.93, 0.13);

   else if (measP == 100)
        returvalue = 0.12;
    else
        returvalue = 0; // just to be sure fan never off  0.12;
    return returvalue = (returvalue);
    }

/*
 * end Stuff to make the phase cut output linear
 */

/*
static int IRAM_ATTR nextVal(int curr, int max) {
	if (curr >= max) {
		if (max<0){
			return 0;
		}
		return max;
	}
  	if (max - curr > 10) {
  		return curr + 10;
  	}
  	return curr + 1;
}
*/
int IRAM_ATTR nextValneg(int curr, int max) {
	if (1000 - max <= curr) {
		if(1000 - max < 0){
			return 0;
		}
		return 1000 - max;
  	  }
  	  if (1000 - max - curr > 10) {
  		  return curr + 10;
  	  }
  	  return curr + 1;
}

//runs each tenth of a second
//void IRAM_ATTR timerCallBack(TimerHandle_t xTimer) {
//	//notihing to do
//}

static bool skip1 = false;
void IRAM_ATTR gpio_isr_handler(void *arg) {
	uint32_t gpio_num = (uint32_t)arg;
	if (gpio_num == ZerroCrossPin) {
		if (cnt > 500) {
			if(skip1){						//skip negative part
				ZerroCross = true;
			}
			skip1 = !skip1;
			lstcnt = cnt;
			cnt = 0;
		}
	}
}


void init_zerocross(void) {
  gpio_config_t io_conf;
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);
  gpio_set_intr_type(ZerroCrossPin, GPIO_INTR_POSEDGE);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(ZerroCrossPin, gpio_isr_handler, (void *)ZerroCrossPin);
}




static int16_t fireCNT = 0;
static int16_t fire1 = 0;
static int16_t fire2 = 0;
static int16_t fire3 = 0;


static void IRAM_ATTR timer_isr(void *arg) {
	portENTER_CRITICAL(&timerMux);
	TIMERG0.int_clr_timers.t0 = 1;
	TIMERG0.hw_timer[0].config.alarm_en = 1;
	cnt++;
	fireCNT++;
	if (ZerroCross == true){
		ZerroCross = false;
		fireCNT =0;
		fire1 = 1000 - dim1;
		fire2 = 1000 - dim2;
		fire3 = 1000 - dim3;
		portEXIT_CRITICAL(&timerMux);
		return;
	}

	if(fireCNT >= 1000){					//1000 interrupts during one period. Reset to zero for negative part of the mains cycle
		fireCNT = 0;
	}
	if(enableOutput){
		if(fireCNT < fire1 )gpio_set_level(AC_pin1, 0);		//deactivate optocupler
		else gpio_set_level(AC_pin1, 1);					//activate optocupler

		if(fireCNT < fire2 )gpio_set_level(AC_pin2, 0);
		else gpio_set_level(AC_pin2, 1);

		if(fireCNT < fire3 )gpio_set_level(AC_pin3, 0);
		else gpio_set_level(AC_pin3, 1);
	}else{
		gpio_set_level(AC_pin1, 0);		//deactivate optocupler
		gpio_set_level(AC_pin2, 0);		//deactivate optocupler
		gpio_set_level(AC_pin3, 0);		//deactivate optocupler
	}
	portEXIT_CRITICAL(&timerMux);
}

/*
    setup timer auto reload
    100us interval at 50Hz
    83us  interval at 60Hz
*/
void init_timer(int ipp) {
  int timer_period_us;
  timer_config_t config = {
      .alarm_en = true,
      .counter_en = false,
      .intr_type = TIMER_INTR_LEVEL,
      .counter_dir = TIMER_COUNT_UP,
      .auto_reload = true,
      .divider = 8 /* 1 us per tick */
  };
  if (ipp >= 900) {
    timer_period_us = 100;
    Hz50 = true;
    ESP_LOGI("Timer", "setup->50Hz");
  } else {
    timer_period_us = 83;
    Hz50 =  false;
    ESP_LOGI("Timer", "setup->60Hz");
  }
  timer_init(TIMER_GROUP_0, TIMER_0, &config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timer_period_us);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_register(TIMER_GROUP_0, TIMER_0, &timer_isr, NULL, 1, &s_timer_handle);
  timer_start(TIMER_GROUP_0, TIMER_0);
  gpio_pad_select_gpio(AC_pin1);gpio_set_direction(AC_pin1,GPIO_MODE_OUTPUT);
  gpio_pad_select_gpio(AC_pin2);gpio_set_direction(AC_pin2,GPIO_MODE_OUTPUT);
  gpio_pad_select_gpio(AC_pin3);gpio_set_direction(AC_pin3,GPIO_MODE_OUTPUT);

}

