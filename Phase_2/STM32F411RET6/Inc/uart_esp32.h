#ifndef UART_ESP32_H
#define UART_ESP32_H

#include "stm32f4xx.h"
#include "motor.h"

extern volatile RobotState_t robotState;
extern volatile uint32_t msTicks;
extern volatile uint32_t lastCommandTime;

void UART_ESP32_Init(void);

typedef struct {
	float temperature;
	float humidity;
	float smoke_ppm;
	float light_lux;
} SensorData_t;

#pragma pack(push,1)
typedef struct {
	uint8_t start;
	float temperature;
	float humidity;
	float smoke_ppm;
	float light_lux;
	uint8_t crc;
	uint8_t end;
} SensorDataFrame_t;

void UART_ESP32_SendSensorData(const SensorData_t *data);

#endif
