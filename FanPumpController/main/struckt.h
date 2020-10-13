/*
 * struckt.h
 *
 *  Created on: 26 Aug 2020
 *      Author: Peter
 */

#ifndef MAIN_STRUCKT_H_
#define MAIN_STRUCKT_H_

#define NVM_VERSION 2

typedef struct {
	uint8_t nvm_config;
	uint8_t nvm_version;
	bool connected;
	bool purifyvalveOn;
	bool drainOn;
	bool extventOn;
	bool pumpOn;
	bool reverse;
	bool fanOn;
    bool fanManual;
    bool ldr;
    bool cleanManual;
    uint8_t cleantimeset;
    uint8_t fanspeedminimal;
    uint8_t fanspeedminimaltriac;
    uint8_t fanspeedmaximal;
    uint8_t fanspeedmaximalinverter;//60Hz max
    uint8_t pumpspeedminimal;
	uint8_t pumpspeedmaximal;
    uint8_t fanspeedmanual;
	uint8_t fanspeed;
    uint8_t fanspeed_set;
	uint8_t floatsensor; // 0xFF means full 0xB4 means empty
	uint8_t floatsensorError; // 0xFF means full 0xB4 means empty
    uint8_t RHmimimal;
    uint8_t RHmaximal;
	uint8_t RH_day;
    uint8_t RH_night;
    uint8_t pump_max;
    uint8_t pump_min;
    float RH_set;
	float temp_set;
	float tout;
	float current;
	unsigned long pumpTime;
	unsigned long onTime;
	unsigned long offTime;
	float P;
	float I;
	float D;
}sys;

int mode;
uint8_t nvm_version;

esp_err_t store_struckt_name(char *nameStore,void* name,size_t strucktSize);
esp_err_t restore_struckt_name(char *nameStore,void* name, size_t strucktSize);
int32_t read_nvmint32(char *nameStore);
esp_err_t write_nvmint32(char *nameStore,int32_t b);
void initKabel(void);
void initPump(void);


//extern humidifier Kabel400;
sys Kabel400;
sys OuleMould;
sys WeifangMuhe;
sys setup;
sys Pump;

#endif /* MAIN_STRUCKT_H_ */
