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
