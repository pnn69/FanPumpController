/*
* Touch stuff
*/
#include <stdio.h>
#include <stdbool.h>
#include "global.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#define TAG "Touch"

#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (1)
#define TOUCH_FILTER_MODE_EN  (1)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

#define tres0 10
#define tres1 15
#define tres2 15

uint16_t TF0,R0,TF1,R1,TF2,R2;
bool T0,T1,T2,key0,key1,key2;

void calibrateTouch(void){
    touch_pad_read_filtered(TOUCH_PAD_0, &TF0);
    touch_pad_read_filtered(TOUCH_PAD_1, &TF1);
    touch_pad_read_filtered(TOUCH_PAD_2, &TF2);
    R0 = TF0-tres0;
    R1 = TF1-tres1;
    R2 = TF2-tres2;
    key0 = false;
    key1 = false;
    key2 = false;
    T0= false;
    T1= false;
    T2= false;
}

/*
    Read values sensed at all available touch pads.
    Print out values in a loop on a serial monitor.
*/
void touch_task(void *pvParameter)
{
	TickType_t touchstamp = xTaskGetTickCount();
    ESP_LOGI(TAG,"Init ");
    touch_pad_init();
    vTaskDelay(500/ portTICK_PERIOD_MS);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V7, TOUCH_HVOLT_ATTEN_1V);
    //touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(TOUCH_PAD_0, TOUCH_THRESH_NO_USE);
    touch_pad_config(TOUCH_PAD_1, TOUCH_THRESH_NO_USE);
    touch_pad_config(TOUCH_PAD_2, TOUCH_THRESH_NO_USE);
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    touch_pad_read_filtered(TOUCH_PAD_0, &TF0);
    touch_pad_read_filtered(TOUCH_PAD_1, &TF1);
    touch_pad_read_filtered(TOUCH_PAD_2, &TF2);
    vTaskDelay(500/ portTICK_PERIOD_MS);
    calibrateTouch();
    vTaskDelay(500/ portTICK_PERIOD_MS);
    calibrateTouch();
    ESP_LOGI(TAG,"Init done!");
    static int cnt=0;;
    while (1) {
        // If open the filter mode, please use this API to get the touch pad count.
        touch_pad_read_filtered(TOUCH_PAD_0, &TF0);
        touch_pad_read_filtered(TOUCH_PAD_1, &TF1);
        touch_pad_read_filtered(TOUCH_PAD_2, &TF2);
        if(cnt++ == 2)cnt=0;
    	if(R0 > TF0){
            T0 = true;
            key0 = true;
            if(R0-TF0 > tres0 ) R0--;
        }else{
        	T0 = false;
        	if(cnt ==0 ){
        		if(TF0-tres0 > R0) R0++;
        		if(TF0-tres0 < R0) R0--;
        	}
        }
        if(R1 > TF1){
            T1 = true;
            key1 = true;
            if(R1-TF1 > tres1 ) R1--;
        }else{
        	T1 = false;
        	if(cnt == 0){
        		if(TF1-tres1 > R1) R1++;
        		if(TF1-tres1 < R1) R1--;
        	}
        }
        if(R2 > TF2){
            T2 =  true;
            key2 = true;
            if(R2-TF2 > tres2 ) R2--;
        }else{
            T2 =  false;
            if(cnt == 0){
            	if(TF2-tres2 > R2) R2++;
            	if(TF2-tres2 < R2) R2--;
            }
        }

    	if (touchstamp + pdMS_TO_TICKS(1000) < xTaskGetTickCount()) {
        	touchstamp = xTaskGetTickCount();
        	//ESP_LOGI("Touch","T0:%d[%3d,%03d,%03d] T1:%d[%3d,%03d,%03d] T2:%d[%3d,%03d,%03d] Approx:%d", T0,TF0,R0,TF0-R0,T1, TF1,R1,TF1-R1,T2, TF2,R2,TF2-R2, aproxtimer);
        	if((!T0 && !T1 && !T2)&&aproxtimer == 0){
        		calibrateTouch();
        	}
        }
        vTaskDelay(50/ portTICK_PERIOD_MS);
    }
}
