#include "rs485.h"
#include <driver/gpio.h>
#include <driver/uart.h>
#include <stdint.h>
#include "crc.h"
#include "global.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "struckt.h"
#include "ntc.h"
#include "timer.h"

#define dummyPinrx 36 //this pin is not used and can be used to point to if no data reception is needed
#define dummyPintx 34 //this pin is not used and can be used to point to if no data transmission is needed
//#define PACKET_READ_TICS (100 / portTICK_RATE_MS)

/*********************************************************************
 * Pinout ESP32 RS485 port
 * IO_RS485[DIR][DATA]
 **********************************************************************/
uint8_t IO_RS485[6][2] = {{21,23},{2,26},{0,27},{4,14},{18,16},{5,17}}; //{DIR,DATA}
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)
#define BUF_SIZE (100)
#define ECHO_READ_TOUT          (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks
#define port0 0
#define port1 1
#define port2 2
#define port3 3
#define port4 4
#define port5 5

/*********************************************************************
 * Kabel400 defines
**********************************************************************/

#define purifyvalve 0x40 		//valve purify
#define pump 0x20 		//pump   valve purify
#define drain 0x08 		//drain
#define extvent 0x10 	//swing




#define STORAGE_NAMESPACE "storage"
#define TAG "RS485"

float hum_FG6485;
float tmp_FG6485;
double TMP, HUM;
int rs485status;
bool HedyOnOff = false;
uint8_t rotate = 1;

/*********************************************************************/
/* RS 485 commands */
/* length , data , ... */
/**********************************************************************/
char FG645rdTMP[]   = {6, 0x01, 0x03, 0x00, 0x00, 0x00, 0x02};


/*********************************************************************/
/* Setup output */
/**********************************************************************/
void init_RS485(uint port) {
	ESP_LOGI(TAG, "Port %d setup ",port);
	gpio_pad_select_gpio(IO_RS485[port][0]);gpio_set_direction(IO_RS485[port][0],GPIO_MODE_OUTPUT);
	gpio_pad_select_gpio(IO_RS485[port][1]);gpio_set_direction(IO_RS485[port][1],GPIO_MODE_INPUT);
	gpio_set_level(IO_RS485[0][0],0); //set RX mode
}

void init_UART_RS485(const int uart,uint32_t speed) {
	uart_config_t uart_config = {.baud_rate = speed,
									   .data_bits = UART_DATA_8_BITS,
									   .parity = UART_PARITY_DISABLE,
									   .stop_bits = UART_STOP_BITS_1,
									   .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
	uart_param_config(uart, &uart_config);
	uart_driver_install(uart, BUF_SIZE * 2, 0, 0, NULL, 0);
}

static void uart_write_bytes_RS485(const int uart,uint port,const char* str, uint8_t length)
{
	gpio_set_level(IO_RS485[port][0],1); //set high TX mode
	uart_set_pin(uart, IO_RS485[port][1] ,dummyPinrx, ECHO_TEST_RTS, ECHO_TEST_CTS);
	if (uart_write_bytes(uart, str, length) != length) {
        ESP_LOGE(TAG, "Send data critical failure.");
		uart_set_pin(uart,dummyPintx, IO_RS485[port][1] , ECHO_TEST_RTS, ECHO_TEST_CTS);
		gpio_pad_select_gpio(IO_RS485[port][1]);gpio_set_direction(IO_RS485[port][1],GPIO_MODE_INPUT);
		gpio_set_level(IO_RS485[port][0],0); //set low RX mode
       abort();
    }
	uart_wait_tx_done(uart, 10);
	uart_flush(uart);
	gpio_set_level(IO_RS485[port][0],0); //set low RX mode
	uart_set_pin(uart,IO_RS485[2][1], IO_RS485[port][1] , ECHO_TEST_RTS, ECHO_TEST_CTS);
}

/*
 * preparing data buffer.
 * calculate checksum and add it at the end of the buffer
 * at position 0 the total length will be filled in
 * position 0 contains the original length
 */
char prepair_buf(char *bout, char *data) {
  for (int t = 0; t < data[0]; t++) {
    bout[t] = data[t + 1];
  }
  OutCrc16(bout, data[0]);
  return data[0] + 2;
}

/*********************************************************************
* FG485 stuff
**********************************************************************/
bool FG6485_Temp_RH_task(int uart,int port) {
  int f;
  char writb[20];
  uint8_t readb[20];
  int l = prepair_buf(writb, FG645rdTMP); 				// load command
  uart_write_bytes_RS485(uart, port,writb, l); 			// send command
  f = uart_read_bytes(uart, (uint8_t *)readb, 9, 10 );	// get response
  if (InCrc16((char *)readb, f)) {
	  hum_FG6485 = ((float)(readb[3] << 8 | readb[4]))/10;			// extract humidity
	  tmp_FG6485 = ((float)(readb[5] << 8 | readb[6]))/10;			// extract temperature
	  //ESP_LOGI(TAG, "FG6485 port%d temp: %02.2f, FG6485 hum:  %02.2f",port,tmp_FG6485,hum_FG6485);
	  return 1;											// return 1 on success
  } else{
	  //ESP_LOGI("RS485", "No data from FG6485 datasize:%d",f);
	  return 0;											// no new data found
  }
}

void RS487_humidifier_task(int uart,int port) {
  uint8_t l = 0;
  TickType_t timestamp = xTaskGetTickCount();
  TickType_t timeout = xTaskGetTickCount();
  char writb[8];
  uint8_t readb[20];
  ESP_LOGI(TAG,"Search Humidifier 1");
  // ESP_LOGI("Modbus","Search Humidifier   ");
  while (timeout + pdMS_TO_TICKS(1500) > xTaskGetTickCount()) {
    if (timestamp + pdMS_TO_TICKS(500) < xTaskGetTickCount()) {
      timestamp = xTaskGetTickCount();
      if (WeifangMuhe.fanspeed < 0x0b) WeifangMuhe.fanspeed = 0x0b;
      if (WeifangMuhe.fanspeed > 0x16) WeifangMuhe.fanspeed = 0x16;
      writb[0] = (0xaa);
      if (WeifangMuhe.fanOn)
        writb[1] = 0x12;
      else
        writb[1] = 0x02;
      writb[2] = WeifangMuhe.fanspeed;
      uart_write_bytes_RS485(uart, port,writb, 3); 			// send command
    }
    l = uart_read_bytes(uart, (uint8_t *)readb, 5, 1 );	// get response
    if (l >= 4) {
      if (readb[0] == 0xAA && readb[1] < 100) {
    	  WeifangMuhe.temp_set = readb[1];
    	  WeifangMuhe.current = (float)readb[4] / 10;
        ESP_LOGI(TAG, "New data form humidifier: temp: %02.2f, Current  %02.2f",WeifangMuhe.temp_set ,WeifangMuhe.current);
        timeout = xTaskGetTickCount();
      }
      for(int t=0; t< l;t++){
    		  printf("%x",readb[t]);
      }
      uart_flush_input(UART_NUM_1);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

/*only for a simple test*/
float Setpoint_RH = 50;
float Measured_RH = 49;
float Previous_Error = 0;
bool runOnceTestPid = false;
float Error, Error_Integral, Error_Derivative;
float K_proportional, K_integral, K_derivative;
float PID_Output;
float Error_max;



void HumiPid(void){
	if(!runOnceTestPid){
		runOnceTestPid =  true;
		Error_Integral = 0;
		Error_max = 1;
		Previous_Error = 0;
	}
	Error = Kabel400.RH_set - hum_FG6485;
	Error_Integral = Error_Integral + Error;

	if(Error_Integral> Error_max) //limit intergrator
		Error_Integral = Error_max;
	else if(Error_Integral < 0)
		Error_Integral = 0;
	if(Error <= 0){
		Error_Integral = 0;
	}

	Error_Derivative = Error - Previous_Error;
	Previous_Error = Error;
	PID_Output = (Kabel400.P*Error) + (Kabel400.I*Error_Integral) + (Kabel400.D*Error_Derivative);
	if(PID_Output > Kabel400.fanspeedmaximal)
		PID_Output = Kabel400.fanspeedmaximal;
	else if(PID_Output < 0)
		PID_Output = 0;
	Kabel400.fanspeed = (int)PID_Output;
	//ESP_LOGI("RS485", "Fan speed:%02d P:%02.5f I:%02.5f RH_set:%02.1f RS_act:%02.1f",(int)Kabel400.fanspeed,(K_proportional*Error),(K_integral*Error_Integral), Kabel400.RH_set,hum_FG6485);
}

void PumpFanPid(void){
	if(!runOnceTestPid){
		runOnceTestPid =  true;
		Error_Integral = 0;
		Error_max = 1;
		Previous_Error = 0;
	}
	Error = Pump.temp_set - NTC[Tout];
	Error_Integral = Error_Integral + Error;

	if(Error_Integral> Error_max) //limit intergrator
		Error_Integral = Error_max;
	else if(Error_Integral < 0)
		Error_Integral = 0;
	if(Error <= 0){
		Error_Integral = 0;
	}
	Error_Derivative = Error - Previous_Error;
	Previous_Error = Error;
	PID_Output = (Pump.P*Error) + (Pump.I*Error_Integral) + (Pump.D*Error_Derivative);
	if(PID_Output > Pump.fanspeedmaximal)
		PID_Output = Pump.fanspeedmaximal;
	else if(PID_Output < 0)
		PID_Output = 0;
	Pump.fanspeed = (int)PID_Output;
	//ESP_LOGI("RS485", "Fan speed:%02d P:%02.5f I:%02.5f RH_set:%02.1f RS_act:%02.1f",(int)Kabel400.fanspeed,(K_proportional*Error),(K_integral*Error_Integral), Kabel400.RH_set,hum_FG6485);
}

void humidifierKabel400(int uart,int port) {
	char writb[8];
	uint8_t readb[20];
	writb[0] = (0x08);
	writb[1] = (0x50);
	writb[2] = (0x00);
	if(Kabel400.fanManual == true ){
		Kabel400.fanManual = Kabel400.fanspeedmanual;
	}else{
		HumiPid();
	}

	if(Kabel400.fanspeed > 0){   				//fan should be turned on
		Kabel400.pumpOn = true;					//Okay so turn on pump
		Kabel400.fanOn = true;					//fan on indicator
		Kabel400.pumpTime++;					//increase pump timer
		Kabel400.offTime = 0;
		if(Kabel400.fanspeed >Kabel400.fanspeedmaximalinverter ){
			 Kabel400.fanspeed = Kabel400.fanspeedmaximalinverter;
		}
		if(Kabel400.fanspeed < Kabel400.fanspeedminimal ){
			 Kabel400.fanspeed = Kabel400.fanspeedminimal;
		}
	}else{
		Kabel400.pumpOn = false;
		Kabel400.fanOn = false;
		Kabel400.fanspeed = 0;				//Stop fan
		Kabel400.offTime++;
	}

	//water level protection
	if(Kabel400.floatsensor == 0xFF ){				//Check water level
		Kabel400.floatsensorError = 0;				//reset timeout
	}else if(Kabel400.floatsensorError++ > 60){	//increase timeout timer and check if more than 60 second are passed
		//Kabel400.pumpOn = false;					//if so turn off pump
		//Kabel400.fanspeed = 0;						//Stop fan
		//Kabel400.fanOn = false;
	}
	//flush on timeout;
	if(Kabel400.onTime >= Kabel400.cleantimeset *60*60){
		Kabel400.pumpOn = false;					//if so turn off pump
		Kabel400.drainOn = true;					//open drain
		if(Kabel400.onTime >= Kabel400.cleantimeset *60*60 + 60*5){
			Kabel400.onTime = 0;
		}
	}
	//flush and do not refill
	if(Kabel400.offTime >= 60*60*24*3){
		Kabel400.drainOn = true;					//open drain
	}

	//write to inverter
	writb[3] = Kabel400.fanspeed;			//write to buffer
	writb[4] = Kabel400.pumpOn*pump + Kabel400.drainOn*drain +!Kabel400.drainOn*extvent + Kabel400.purifyvalveOn*purifyvalve;
	writb[5] = 1 + writb[0] + writb[1] + writb[2] + writb[3] + writb[4]; 	//calculate checksum
	uart_write_bytes_RS485(uart, port,writb, 6); 							//send command
	uart_read_bytes(uart, (uint8_t *)readb, 9, 50 );					//get response
	if (readb[0] == 0x08 && readb[1] == 0x51) {
		Kabel400.connected = true;
		Kabel400.floatsensor = readb[5];			// extract aqua level 0xFF means full 0xB4 means empty
		ESP_LOGI("RS485", "Fan speed %02d RH_set:%02.1f RS_act:%02.1f",Kabel400.fanspeed,Kabel400.RH_set,hum_FG6485 );
		Kabel400.onTime++;
	}else{
		Kabel400.connected = false;
	}
}

void PumpFanInvereter(int uart,int port) {
	char writb[8];
	uint8_t readb[20];
	PumpFanPid();
	writb[0] = (0x08);
	writb[1] = (0x50);
	writb[2] = (0x00);
	writb[3] = Pump.fanspeed*(Pump.fanspeedmaximalinverter/100);			//write to buffer
	writb[4] = 0;
	writb[5] = 1 + writb[0] + writb[1] + writb[2] + writb[3] + writb[4]; 	//calculate checksum
	uart_write_bytes_RS485(uart, port,writb, 6); 							//send command
	uart_read_bytes(uart, (uint8_t *)readb, 9, 50 );					//get response
	if (readb[0] == 0x08 && readb[1] == 0x51) {
		Pump.connected = true;
	}else{
		Pump.connected = false;
	}
}

void RS487_init(void) {
	for(int t = 0; t<6;t++){
		init_RS485(t); //setup all RS485 portS
	}
	if (mode == modeFanAuxBox){
		init_UART_RS485(UART_NUM_2,9600);
		init_UART_RS485(UART_NUM_1,1200);
	}
	if (mode ==  modeFanPumpController ){
		init_UART_RS485(UART_NUM_2,9600);
		init_UART_RS485(UART_NUM_1,1200);
	}
}

void RS487_task(void *arg) {
	RS487_init();
	ESP_LOGI(TAG, "RS485 task running...");
   	TickType_t RS485tamp = xTaskGetTickCount();
	while (1) {
		if(RS485tamp + OneSec < xTaskGetTickCount()){
			RS485tamp = xTaskGetTickCount();
			if (mode == modeFanAuxBox){
				for(int t = 1; t<6;t++){ //read 6x RS485 FG6485 TMP/RH device
					FG6485_Temp_RH_task(UART_NUM_2,t);
				}
				humidifierKabel400(UART_NUM_1,port0);
			}
			if (mode ==  modeFanAuxBoxRetro ){
				humidifierKabel400(UART_NUM_1,port0);
			}
			if (mode ==  modeFanPumpController ){
				PumpFanInvereter(UART_NUM_1,port0);
			}
			if (mode ==  modeFanPumpBoxRetro ){
			}
		}
		vTaskDelay(1);
	}
}

