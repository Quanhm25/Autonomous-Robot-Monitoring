#include "stm32f4xx.h"
#include "sensor/bh1750.h"
#include "sensor/dht22.h"
#include "sensor/mq2.h"
#include "motor.h"
#include "uart_esp32.h"

// =======================================================================
// CẤU HÌNH CLOCK (84 MHz)
// =======================================================================
static void SystemClock_Config(void) {
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    // Flash latency = 2WS cho 84MHz
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_2WS;

    // Cấu hình PLL: HSI(16MHz) / M(16) * N(336) / P(4) = 84MHz
    RCC->PLLCFGR = (16 << RCC_PLLCFGR_PLLM_Pos) |
                   (336 << RCC_PLLCFGR_PLLN_Pos) |
                   (1 << RCC_PLLCFGR_PLLP_Pos) | // PLLP = 01 (DIV4)
                   (7 << RCC_PLLCFGR_PLLQ_Pos) |
                   (0 << RCC_PLLCFGR_PLLSRC_Pos); // Source = HSI

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    // Chia clock cho bus AHB/APB
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE)  | RCC_CFGR_HPRE_DIV1;
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE1) | RCC_CFGR_PPRE1_DIV2;
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE2) | RCC_CFGR_PPRE2_DIV1;

    // Chọn PLL làm SYSCLK
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    // SysTick ngắt mỗi 1ms @ 84MHz
    SysTick->LOAD = (84000 - 1);
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

int main(void) {
    SystemClock_Config();

    MQ2_Init();
    DHT_GPIO_Init();
    DHT_TIM_Init();
    BH1750_Init();

    UART_ESP32_Init();

    Motor_Init();
    Motor_Stop();

    while(1) {
    	if(msTicks - lastCommandTime > 200) {
    		robotState = ROBOT_STOP;
    	}

    	switch(robotState) {
    	case ROBOT_FORWARD:
    		Motor_Forward();
    		break;
    	case ROBOT_BACKWARD:
    		Motor_Backward();
    		break;
    	case ROBOT_RIGHT:
    		Motor_Right();
    		break;
    	case ROBOT_LEFT:
    		Motor_Left();
    		break;
    	default:
    		Motor_Stop();
    		break;
    	}
    }
}
