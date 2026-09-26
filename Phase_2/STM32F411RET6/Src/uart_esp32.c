#include "uart_esp32.h"

volatile RobotState_t robotState = ROBOT_STOP;
volatile uint32_t msTicks = 0;
volatile uint32_t lastCommandTime = 0;

void SysTick_Handler(void) {
	msTicks++;
}

//Xử lý giao thức UART với EPS32
void USART2_IRQHandler(void) {
    // Kiểm tra cờ RXNE (Receive Not Empty) để biết có dữ liệu đến
    if (USART2->SR & USART_SR_RXNE) {
        uint8_t data = USART2->DR; //Data Read

        switch(data)
        {
        case 'F':
        	robotState = ROBOT_FORWARD;
        	lastCommandTime = msTicks;
        	break;
        case 'B':
        	robotState = ROBOT_BACKWARD;
        	lastCommandTime = msTicks;
        	break;
        case 'R':
        	robotState = ROBOT_RIGHT;
        	lastCommandTime = msTicks;
        	break;
        case 'L':
        	robotState = ROBOT_LEFT;
        	lastCommandTime = msTicks;
        	break;
        case 'S':
        	robotState = ROBOT_STOP;
        	lastCommandTime = msTicks;
        	break;
        }
    }
}

// UART_ESP32_Init (USART2 Init) (ESP32 UART - PA2: TX, PA3: RX)
void UART_ESP32_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    //PA2 = TX, PA3 = RX; Alternate Function
    GPIOA->MODER &= ~((3 << 4) | (3 << 6));
    GPIOA->MODER |=  ((2 << 4) | (2 << 6));

    //Alternate Function 7
    GPIOA->AFR[0] &= ~((0xF << 8) | (0xF << 12));
    GPIOA->AFR[0] |=  ((0x7 << 8) | (0x7 << 12));

    // Baudrate 115200 @ 84MHz (Oversampling = 16)
    USART2->BRR = 0x16D;

    // Kích hoạt UART, TX, RX và Ngắt nhận dữ liệu
    USART2->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_UE;
    NVIC_EnableIRQ(USART2_IRQn);
}

static void USART2_SendByte(uint8_t byte) {
	while(!(USART2->SR & USART_SR_TXE));
	USART2->DR = byte;
}

static uint8_t CRC8_Calc(const uint8_t *data, uint8_t len) {
	uint8_t crc = 0x00;
	for(uint8_t i = 0; i < len; i++) {
		crc ^= data[i];
		for(uint8_t b = 0; b < 8; b++) {
			crc = (crc & 0x80) ? (crc<<1) ^ 0x07 : (crc<<1);
		}
	}
	return crc;
}

void UART_ESP32_SendSensorData(const SensorData_t *data) {
	SensorDataFrame_t frame;
	frame.start = 0xAA;
	frame.temperature = data->temperature;
	frame.humidity = data->humidity;
	frame.smoke_ppm = data->smoke_ppm;
	frame.light_lux = data->light_lux;
	frame.crc = CRC8_Calc((uint8_t*)&frame.temperature, 16);
	frame.end = 0x55;

	uint8_t *bytes = (uint8_t*)&frame;
	for(uint8_t i = 0; i < sizeof(frame); i++) {
		USART2_SendByte(bytes[i]);
	}
}
