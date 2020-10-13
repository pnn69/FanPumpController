/*
    i2c stuff
*/
#include "i2c.h"
#include "RS485.h"
#include "global.h"
#include "touch.h"
#include "timer.h"
#include "adc.h"
#include "ntc.h"
#include "menu.h"
#include "struckt.h"
#include <driver/i2c.h>
#include <stdbool.h>
#include <esp_log.h>
extern bool buzzerOnOff;
bool approx = false;
int aproxtimer = 600;
int menu = 0; //position menu
bool statFan = false;
bool statRH = false;
bool statHeater = false;
uint8_t PCF8574_I2C_ADDR = 0x38; //(A version)
//uint8_t PCF8574_I2C_ADDR = 0x20;
#define TAG "I2C"


void i2c_master_init() {
  int i2c_master_port = I2C_PORT_NUM;
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
  conf.scl_io_num = I2C_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
  conf.master.clk_speed = I2C_FREQ_HZ;
  i2c_param_config(i2c_master_port, &conf);
  i2c_driver_install(i2c_master_port, conf.mode, I2C_RX_BUF_DISABLE, I2C_TX_BUF_DISABLE, 0);
}

uint8_t i2c_read_device(uint8_t addres) {
  uint8_t data;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addres << 1) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, &data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_PORT_NUM, cmd, 10 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return data;
}

void i2c_write_device(uint8_t addres, uint8_t data) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addres << 1) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(I2C_PORT_NUM, cmd, 10 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
}

void setI2CaddresMUX(void){
	  esp_err_t res;
	  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	  i2c_master_start(cmd);
	  i2c_master_write_byte(cmd, (PCF8574_I2C_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
	  i2c_master_stop(cmd);
	  res = i2c_master_cmd_begin(I2C_PORT_NUM, cmd, 1000 / portTICK_RATE_MS);
	  i2c_cmd_link_delete(cmd);
	  if(res != ESP_OK){
		  PCF8574_I2C_ADDR = 0x20;
	  }
}

static uint8_t IOstatus = 0;
static uint8_t adc_select=0;
float voltageMUX = 0;
void i2c_task(void *arg) {
  TickType_t timestamp = xTaskGetTickCount();
  TickType_t adcstamp = xTaskGetTickCount();
  ESP_LOGI(TAG, "I2C tasK running... ");
  i2c_master_init();  // start i2c task
  setI2CaddresMUX(); //two types of mux addreses possible
  adc_select = 0;
  IOstatus = i2c_read_device(PCF8574_I2C_ADDR);
  IOstatus ^= (-!1 ^ IOstatus) & (1UL << buzzer);  	// Changing the nth bit to x
  i2c_write_device(PCF8574_I2C_ADDR, IOstatus);  // updat leds
  IOstatus = 1 << buzzer;                       // toggel bit 3 (led on pcb)
  i2c_write_device(PCF8574_I2C_ADDR, IOstatus);  // updat leds
  ssd1306_init();
  ssd1306_display_clear();
  OLED_infoScreen();
  IOstatus ^= (-!0 ^ IOstatus) & (1UL << buzzer);  	// Buzzer off
  i2c_write_device(PCF8574_I2C_ADDR, IOstatus);  // updat leds
  vTaskDelay(5);
  IOstatus ^= (-!0 ^ IOstatus) & (1UL << buzzer);  	// Buzzer off
  i2c_write_device(PCF8574_I2C_ADDR, IOstatus);  // updat leds
  aproxtimer = 600;
  float adcsamp=0;
  float adcVoltage=0;
  for (;;) {
	  if (adcstamp + pdMS_TO_TICKS(200) <= xTaskGetTickCount()) {
    	adcstamp = xTaskGetTickCount();
    	adcsamp = 0;
    	for(int l=0;l<10;l++){
    		adcsamp +=AdcSampRaw();
    	}
    	adcsamp = adcsamp/10;
    	adcVoltage = ReadVoltage(adcsamp)*1.881;
    	voltageFAN = ReadVoltage(AdcFanRaw())*4.34367;
    	switch(adc_select){
    		case 0:
    			NTC[2] = new_ntc_sample(adcsamp);
    			ADC[2] = adcsamp;
    			break;
    		case 1:
    			NTC[1] = new_ntc_sample(adcsamp);
    			ADC[1] = adcsamp;
    			break;
    		case 2:
    			NTC[0] = new_ntc_sample(adcsamp);
    			ADC[0] = adcsamp;
    			//ESP_LOGI("I2C", "NTC0 RAW %.0f Temp1 %03.02f",adcsamp,NTC[0]);
    			break;
    		case 3:
    			NTC[3] = new_ntc_sample(adcsamp);
    			ADC[3] = adcsamp;
    			break;
    		case 4:
    			NTC[4] = new_ntc_sample(adcsamp);
    			ADC[4] = adcsamp;
    			if(adcVoltage >= 0.5){
    				PressLow = map(adcVoltage,0.5,4.5,0,8); //inp,Vmin,Vmax,Barrmin,Barrmax
    			} else {
    				PressLow = -1;
    			}
    			break;
    		case 5:
    			NTC[5] = new_ntc_sample(adcsamp);
    			ADC[5] = adcsamp;
    			PressHigh = map(adcVoltage,0.5,4.5,0,8); //inp,Vmin,Vmax,Barrmin,Barrmax
    			break;
    		case 6:
    			voltageRH = adcsamp* 347 * 3.5 / 4095; //result in mV;
    			voltageRH = ReadVoltage(adcsamp)*4.2016;
    			//ESP_LOGI(TAG, "VoltageRH %f",voltageRH);

    			break;
    		case 7:
    			voltageHEAT = ReadVoltage(adcsamp)*4.2016; //result in mV;
    			break;
		}
		if(adc_select++ >6) adc_select = 0;
	  }
	  if (timestamp + pdMS_TO_TICKS(100) < xTaskGetTickCount()) {
    	timestamp = xTaskGetTickCount();

    	if(dim1) statFan = true;
    	else statFan = false;
    	if(dim2) statRH = true;
    	else statRH = false;
    	if(dim3) statRH = true;
    	else statHeater = false;
    	IOstatus = i2c_read_device(PCF8574_I2C_ADDR);
    	approx = (bool)(IOstatus & (1UL << approxPin));  			//get IO status
    	IOstatus ^= (-!statFan ^ IOstatus) & (1UL << ledFan);  		//set led out1
    	IOstatus ^= (-!statRH ^ IOstatus) & (1UL << ledRH);  		//set led out2
    	IOstatus ^= (-!statHeater ^ IOstatus) & (1UL << ledHeater); //set led out3
    	IOstatus ^= (-!0 ^ IOstatus) & (1UL << buzzer);  			//buzzer of
    	IOstatus ^= (-!0 ^ IOstatus) & (1UL << approxPin);  		//Changing the nth bit to x
    	IOstatus = IOstatus & 199;
    	IOstatus = IOstatus | (adc_select << 3);					//set mux to selected channel
    	i2c_write_device(PCF8574_I2C_ADDR, IOstatus);  				// update IO extender

		if(approx==true){
			aproxtimer = 600;
			if(dispayOnOff ==  false){
				ssd1306_display_OnOff(true);
				OLED_homeScreen();
				calibrateTouch();
			}
		}else{
			if(aproxtimer-- == 0 && dispayOnOff ==  true){
				ssd1306_display_OnOff(false);
			}
		}
		if(dispayOnOff){
			if((key0 || key1 || key2)){
				//if(key0)ESP_LOGI(TAG, "Key UP");
				//if(key1)ESP_LOGI(TAG, "Key DOWN");
				//if(key2)ESP_LOGI(TAG, "Key ENTER");
				if(buzzerOnOff && (key0 || key1 || key2)){
					IOstatus ^= (-!1 ^ IOstatus) & (1UL << buzzer);  	// Changing the nth bit to x
					i2c_write_device(PCF8574_I2C_ADDR, IOstatus);  		// update buzzer
					IOstatus ^= (-!0 ^ IOstatus) & (1UL << buzzer);  	// Changing the nth bit to x
					i2c_write_device(PCF8574_I2C_ADDR, IOstatus);  		// update buzzer
				}
		  	}
			if(key0 == 0 && key1 == 0 && key2 == 0){
				if(mode == modeFanAuxBox) 				LCD_menu_1(0);
				else if(mode == modeFanAuxBoxRetro) 	LCD_menu_2(0);
				else if(mode == modeFanPumpController) 	LCD_menu_3(0);
				else if(mode == modeFanPumpBoxRetro) 	LCD_menu_4(0);
			}
			if(key0){
				if(mode == modeFanAuxBox) 				LCD_menu_1(UP);
				else if(mode == modeFanAuxBoxRetro) 	LCD_menu_2(UP);
				else if(mode == modeFanPumpController) 	LCD_menu_3(UP);
				else if(mode == modeFanPumpBoxRetro) 	LCD_menu_4(UP);
				key0 = 0;
			}
			if(key1){
				if(mode == modeFanAuxBox) 				LCD_menu_1(DOWN);
				else if(mode == modeFanAuxBoxRetro) 	LCD_menu_2(DOWN);
				else if(mode == modeFanPumpController) 	LCD_menu_3(DOWN);
				else if(mode == modeFanPumpBoxRetro) 	LCD_menu_4(DOWN);
				key1 = 0;
			}
			if(key2){
				if(mode == modeFanAuxBox) 				LCD_menu_1(ENTER);
				else if(mode == modeFanAuxBoxRetro) 	LCD_menu_2(ENTER);
				else if(mode == modeFanPumpController) 	LCD_menu_3(ENTER);
				else if(mode == modeFanPumpBoxRetro) 	LCD_menu_4(ENTER);
				key2 = 0;
			}
		}
	  }
	  vTaskDelay(1);
  }
}
