#ifndef BH1750_H
#define BH1750_H

#include "stm32f4xx.h"

#define BH1750_ADDR 0x23
#define BH1750_CMD_POWER_ON 0x01
#define BH1750_CMD_RESET 0x07
#define BH1750_CMD_CONT_H 0x10

void BH1750_Init(void);
uint8_t BH1750_ReadLux(float *lux);

#endif

