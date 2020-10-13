/*
 *
 * FanAuxBox retro
 * uses hardware version FanAuxBox 1.7
 *
*/
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
#include <freertos/FreeRTOSConfig.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <soc/timer_group_struct.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "global.h"
#include "timer.h"
#include "adc.h"
#include "menu.h"
#include "struckt.h"
#include "ntc.h"
#define TAG "Main"
uint8_t mac[6];
// bool buzzerOnOff = true;
bool buzzerOnOff = false;

float voltage = 0;
void main_task(void *parameter) {
	ESP_LOGI("Main", "Smartbox HW:%s SW:%s NVM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
	ESP_LOGI(TAG, "Main task running...");
	enableOutput = true;
	ESP_LOGI(TAG, "Output enabled");
	OLED_homeScreen();
	TickType_t secstamp = xTaskGetTickCount();
	while (1) {
		uint8_t ch;
		ch = fgetc(stdin);
		if (ch!=0xFF)
		{
			if(ch == '?'){
				printf("dim1=%d\n",dim1);
				printf("T0:%d[%3d,%03d,%03d] T1:%d[%3d,%03d,%03d] T2:%d[%3d,%03d,%03d] Approx:%d", T0,TF0,R0,TF0-R0,T1, TF1,R1,TF1-R1,T2, TF2,R2,TF2-R2, aproxtimer);
			}
		}
		switch (mode){
	  		case modeFanAuxBox:
				voltage = fan_calcPercentageToVoltage(voltageFAN*10)*100;
				if(voltage > 1000) voltage = 1000;
				if (secstamp + OneSec <= xTaskGetTickCount()) {
					secstamp = xTaskGetTickCount();
				}
				if(voltage >= 1000){
					voltage = 0;
				}else{
					voltage = 1000 - voltage;
				}
				dim1 = nextValneg(dim1, (int)voltage);
				if(voltageRH >5.05){
					dim2 = 1000;
					statRH = true;
				}
				if(voltageRH <4.95){
					dim2 = 0;
					statRH = false;
				}
				if(voltageHEAT >5.05){
					dim3 = 1000;
					statHeater = true;
				}
				if(voltageHEAT <4.95){
					dim3 = 0;
					statHeater = false;
				}
			break;

	  		case modeFanAuxBoxRetro:
				voltage = fan_calcPercentageToVoltage(voltageFAN*10)*100;
				if(voltage > 1000) voltage = 1000;
				if (secstamp + OneSec <= xTaskGetTickCount()) {
					secstamp = xTaskGetTickCount();
				}
				if(voltage >= 1000){
					voltage = 0;
				}else{
					voltage = 1000 - voltage;
				}
				dim1 = nextValneg(dim1, (int)voltage);
				if(voltageRH >5.05){
					dim2 = 1000;
					statRH = true;
				}
				if(voltageRH <4.95){
					dim2 = 0;
					statRH = false;
				}
				if(voltageHEAT >5.05){
					dim3 = 1000;
					statHeater = true;
				}
				if(voltageHEAT <4.95){
					dim3 = 0;
					statHeater = false;
				}
			break;

	  		case modeFanPumpController:
				if (secstamp + OneSec <= xTaskGetTickCount()) {
					secstamp = xTaskGetTickCount();
				}
				if(Pump.fanspeed > 0){
					  dim1 = Pump.fanspeed*(10);
					  if(dim1 > Pump.fanspeedminimaltriac*10) dim1 = Pump.fanspeedminimaltriac*10; //min fanspeed
				 } else {
					  dim1 = 0; //fan off
				 }
				  //calculate pump On/Off
				if(voltageFAN > 5){
					 //PressLow < 0  if input voltage drops below 0.5V Indicating no sensor connected
					  //if so skip low pressure protection
					 if (PressLow < 0.3 && PressLow >= 0){
						  dim2 = 0;
					 } else {
						dim2 = 1000;
					 }
				 }else{
					 dim2 = 0;
				 }
	  		 break;

	  		case modeFanPumpBoxRetro:
	  			vTaskDelay(pdMS_TO_TICKS(10));
	  		break;

	  		default:
	  			vTaskDelay(pdMS_TO_TICKS(10));
	  		break;
		}
	vTaskDelay(pdMS_TO_TICKS(1));
	}
}

void app_main() {

	ESP_LOGI(TAG,"Smartbox booting...");
	vTaskDelay(100 / portTICK_PERIOD_MS);  // give system some time
	xTaskCreate(i2c_task, "i2c_task", 1024 * 5, (void *)0, 0, NULL);

	//setup flash drive
	esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    //Get current stored mode
    ESP_LOGI(TAG,"NVM version hard coded: %d",NVM_VERSION);
    nvm_version = read_nvmint32("NVM_VERSION");
    ESP_LOGI(TAG,"NVM version eeprom: %d",nvm_version);
    if(nvm_version != NVM_VERSION){
    	ESP_LOGI(TAG,"Writing default values");
    	initKabel();
    	initPump();
    	store_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
    	store_struckt_name("Pump",&Pump,sizeof(Pump));
    	write_nvmint32("NVM_VERSION",NVM_VERSION);
    	write_nvmint32("MODE",1);
    }

    //Get current stored mode
    mode = read_nvmint32("MODE");
    ESP_LOGI(TAG,"MODE set to %d",mode);

    //Get stored setting
	if(mode == modeFanAuxBox){
		err = restore_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
		if(NVM_VERSION != Kabel400.nvm_version || err != ESP_OK){
			ESP_LOGI(TAG,"False data in memory so store default data");
			initKabel();
			err = store_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
			if (err == ESP_OK){
				write_nvmint32("NVM_VERSION",NVM_VERSION);
				ESP_LOGI(TAG,"Version NVM now up to date");
			}
		}
	}

	if(mode == modeFanPumpController){
		err = restore_struckt_name("Pump",&Pump,sizeof(Pump));
		if(NVM_VERSION != Pump.nvm_version || err != ESP_OK){
			ESP_LOGI(TAG,"False data in memory so store default data");
			initPump();
			err = store_struckt_name("Pump",&Pump,sizeof(Pump));
			if (err == ESP_OK){
				write_nvmint32("NVM_VERSION",NVM_VERSION);
				ESP_LOGI(TAG,"Version NVM now up to date");
			}
		}
	}
   	ESP_LOGI(TAG,"Reading setup from NVS done!");


	adc_config();
	init_zerocross();  // set up zero cross detection
	init_timer(1000);  // start timer in 50Hz mode
	vTaskDelay(1000 / portTICK_PERIOD_MS);  // give system some time for measuring
	init_timer(lstcnt);  // auto detect 50 or 60 Hz. Correct for 60Hz if needed
	if(mode != modeFanAuxBoxRetro && mode != modeFanPumpBoxRetro ){
		xTaskCreate(RS487_task,"RS487_task", 2024* 2 , NULL, 5, NULL);
	}
	xTaskCreate(touch_task, "touch_task", 2024, NULL, 0, NULL);
	ESP_LOGI(TAG, "Setup done!");
	xTaskCreate(main_task, "main_task", 2024, NULL, 0, NULL);

}
