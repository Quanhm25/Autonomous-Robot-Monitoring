#ifndef I2C1_H
#define I2C1_H

#include "stm32f4xx.h"

void I2C1_Init(void);
uint8_t I2C1_WriteBytes(uint8_t addr, uint8_t *data, uint8_t len);
uint8_t I2C1_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t I2C1_ReadBytes(uint8_t addr, uint8_t *buf, uint8_t len);

#endif

