#include "sensor/mq2.h"
#include <math.h>

float MQ2_R0 = 0.0f;

void MQ2_Init(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	GPIOA->MODER |= (3<<10);
	ADC1->SQR3 = 5;
	ADC1->CR2 |= ADC_CR2_ADON;
}

uint16_t MQ2_Read(void) {
	ADC1->CR2 |= ADC_CR2_SWSTART;
	while(!(ADC1->SR & ADC_SR_EOC));
	return ADC1->DR;
}

float MQ2_ReadAverageVout(uint8_t samples) {
	uint32_t sum = 0;
	for(uint8_t i = 0; i < samples; i++) {
		sum +=  MQ2_Read();
	}
	uint16_t avg_raw = sum / samples;
	return ((float)avg_raw/ MQ2_ADC_MAX) * MQ2_VREF;
}

float MQ2_CalcRs(float Vout) {
	if(Vout < 0.01f) Vout = 0.01f;
	return ((MQ2_VREF - Vout)/Vout) * MQ2_RL;
}

float MQ2_CalibrateR0(uint8_t samples) {
	float Vout = MQ2_ReadAverageVout(samples);
	float Rs_clean_air = MQ2_CalcRs(Vout);
	return Rs_clean_air / 9.8f;
}

float MQ2_GetSmokePPM(void) {
	if(MQ2_R0 <= 0.0f) return -1.0f;
	float Rs = MQ2_CalcRs(MQ2_ReadAverageVout(20));
	float ratio = Rs / MQ2_R0;
	return 3616.1f * powf(ratio, (-2.675f));
}
