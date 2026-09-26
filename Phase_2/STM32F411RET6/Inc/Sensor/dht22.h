#ifndef DHT22_H
#define DHT22_H

#include "stm32f4xx.h"

void DHT_GPIO_Init(void);
void DHT_TIM_Init(void);
void DHT_CheckTimeout(void);
uint8_t DHT_StartRead(void);
uint8_t DHT_Process(float *thermal, float *humidity);

#endif

