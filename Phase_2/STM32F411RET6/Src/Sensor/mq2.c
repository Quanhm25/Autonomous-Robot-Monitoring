#include "sensor/mq2.h"

//Sensor: MQ2
void MQ2_Init(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	GPIOA->MODER |= (3<<10); //Analog Mode
	ADC1->SQR3 = 5;
	ADC1->CR2 |= ADC_CR2_ADON;
}

uint16_t MQ2_Read(void) {
	ADC1->CR2 |= ADC_CR2_SWSTART;
	while(!(ADC1->SR & ADC_SR_EOC));
	return ADC1->DR;
}
