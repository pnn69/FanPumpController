
/*
    adc stuff
*/
#include <esp_adc_cal.h>
#include <math.h>
#include "adc.h"
#include "global.h"

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  //Multisampling
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_channel_t channelFAN = ADC_CHANNEL_2; //ADC1 CH2
static const adc_channel_t channelMUX = ADC_CHANNEL_3; //ADC1 CH3

void adc_config(void ) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channelFAN, atten);
    adc1_config_channel_atten(channelMUX, atten);
}

float AdcFanRaw(void){
    return adc1_get_raw((adc1_channel_t)channelFAN); //result in mV
}
float AdcFan(void){
    return adc1_get_raw((adc1_channel_t)channelFAN) * 422 * 3.5 / 4095; //result in mV
}
float AdcSamp2(void){
	return adc1_get_raw((adc1_channel_t)channelMUX) * 347 * 3.5 / 4095; //result in mV
}

float AdcSampRaw(void){
	return adc1_get_raw((adc1_channel_t)channelMUX);
}

float map(float x, float in_min, float in_max, float out_min, float out_max){
	x =  (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	if(x < out_min) return out_min;
	if(x > out_max) return out_max;
	return x;
}


float ReadVoltage(float reading){// Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
	if(reading < 1 || reading > 4095) return 0;
   return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  //return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
  //return reading;
  //https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
  //https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function
  //https://www.youtube.com/watch?v=RlKMJknsNpo&t=145s
} // Added an improved polynomial, use either, comment out as required


