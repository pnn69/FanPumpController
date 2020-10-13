/*
*NTC stuff
*
* NTC connected to 5V
* Devide resisteror 680E
* Vref 3,3V
* retuns temp in degrees celcius
* Uses Steinhart-Hart correction
* Steinhart-Hart Coefficient Calculator: 227
* https://docs.google.com/spreadsheets/d/1loZOUvEIKg3FN3XMkUbffdzAgFlPnjNALKsVrhj87Dw/edit?ts=5efea640#gid=0
*/
#include <math.h>
#include <esp_log.h>
#include "global.h"
#include "ntc.h"

float NTC[6];

// Steinhart-Hart coeffecients from the spreadsheet.
double                        dKelvin = 273.15;                     // degrees kelvin 
double                        dDegreesC = 0.0;                      // calculated degrees Ca
double                        dResistor = 26000;                      // in ohms
double  					dProbeA = -4.1401032E-03;
double                      dProbeB = 9.4490660E-04;              // value B from spreadsheet
double                        dProbeC = -1.8485579E-06;              // value C from spreadsheet
#define TAG "ntc"

double new_ntc_sample(int ntc){
	double  dThermistor = dResistor * ((4095.0 / (double)ntc) - 1.0);
    // The Steinhart-Hart equation uses log(dRThermistor) four times, so calculate it once.
    double dLogdRThermistor = log(dThermistor);
    // Then calculate degrees C.
    return (1.0 / (dProbeA + (dProbeB * dLogdRThermistor) + (dProbeC * dLogdRThermistor * dLogdRThermistor * dLogdRThermistor)) - dKelvin);
}
