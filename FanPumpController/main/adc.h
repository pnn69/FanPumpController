/*
 * adc.h
 *
 */
#ifndef ADC_H_
#define ADC_H_

float map(float x, float in_min, float in_max, float out_min, float out_max);
float ReadVoltage(float reading);
float voltageFAN;
float voltageRH;
float voltageHEAT;
float PressHigh;
float PressLow;


int ADC[6];
void adc_config(void );
void init_adc(void);
void adc_task(void * parameter);
float AdcFanRaw(void);
float AdcFan(void);
float AdcSamp2(void);
float AdcSampRaw(void);

#endif /* ADC_H_ */
