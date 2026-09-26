#ifndef MQ2_H
#define MQ2_H

#include <stdint.h>
#include "stm32f4xx.h"

#define MQ2_RL 9.8f
#define MQ2_VREF 3.3f
#define MQ2_ADC_MAX 4095.0f

extern float MQ2_R0;

void MQ2_Init(void);
uint16_t MQ2_Read(void);

float MQ2_ReadAverageVout(uint8_t samples);
float MQ2_CalcRs(float Vout);
float MQ2_CalibrateR0(uint8_t samples);
float MQ2_GetSmokePPM(void);

#endif

