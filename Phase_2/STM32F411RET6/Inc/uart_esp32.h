#ifndef UART_ESP32_H
#define UART_ESP32_H

#include "stm32f4xx.h"
#include "motor.h"

extern volatile RobotState_t robotState;
extern volatile uint32_t msTicks;
extern volatile uint32_t lastCommandTime;

void UART_ESP32_Init(void);

#endif
