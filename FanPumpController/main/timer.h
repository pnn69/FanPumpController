/*
 * timer.h
 *
 */

#ifndef TIMER_H_
#define TIMER_H_
#include <stdbool.h>

int lstcnt;

extern int dim1;
extern int dim2;
extern int dim3;
extern bool statFan;
extern bool statRH;
extern bool statHeater;
float fan_calcPercentageToVoltage(float measP ); // percentage in voltage out
int IRAM_ATTR nextValneg(int curr, int max);
void init_timer(int);
void init_zerocross(void);
extern bool enableOutput;

#endif /* TIMER_H_ */
