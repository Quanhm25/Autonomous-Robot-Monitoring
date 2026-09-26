#include "stm32f4xx.h"
#include "stdio.h"
#include "sensor/bh1750.h"
#include "sensor/dht22.h"
#include "sensor/mq2.h"
#include "i2c1.h"
#include "ssd1306.h"
#include "motor.h"
#include "uart_esp32.h"

#define SENSOR_SEND_INTERVAL_MS 1000

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

static void Formatdisplay(float val, char *out) {
	int whole = (int)val;
	int frac = (int)((val - whole)*10);
	if(frac < 0) frac = - frac;
	sprintf(out, "%d.%d", whole, frac);
}

static void FormatforMQ2(float val, char *out) {
    int whole = (int)val;
    float frac_f = val - whole;
    if (frac_f < 0) frac_f = -frac_f;
    int frac = (int)(frac_f * 1000);
    sprintf(out, "%d.%03d", whole, frac);
}

int main(void) {
    SystemClock_Config();

    // Peripheral
    I2C1_Init();
    SSD1306_Init();
    MQ2_Init();
    MQ2_R0 = 20.0f;
    DHT_GPIO_Init();
    DHT_TIM_Init();
    BH1750_Init();


    // ESP32
    UART_ESP32_Init();

    // Actuator
    Motor_Init();
    Motor_Stop();

    float temperature = 0, humidity = 0, lux = 0, smokePPM = 0.0f;

    uint32_t lastdhtTrigger = 0, lastLuxRead = 0, lastDisplayUpdate = 0, lastSensorSend = 0;

    while(1) {
    	DHT_CheckTimeout();

    	if(msTicks - lastdhtTrigger >= 2500) {
    		lastdhtTrigger = msTicks;
    		DHT_StartRead();
    	}
    	float t, h;
    	if(DHT_Process(&t, &h)) {
    		temperature = t;
    		humidity = h;
    	}

    	if(msTicks - lastLuxRead >= 500) {
    		lastLuxRead = msTicks;
    		BH1750_ReadLux(&lux);
    	}

    	if(msTicks - lastDisplayUpdate >= 500) {
    		lastDisplayUpdate = msTicks;

    		smokePPM = MQ2_GetSmokePPM();

    		char line[22], numbuf[10];
    		SSD1306_Clear();

    	    SSD1306_WriteStringCentered("Robot Car - Phase 2", 0);
    	    SSD1306_WriteStringCentered("Hoang Minh Quan", 8);


    		Formatdisplay(temperature, numbuf);
    		SSD1306_SetCursor(2, 18);
    		snprintf(line, sizeof(line), "Nhiet do: %s C", numbuf);
    		SSD1306_WriteString(line);

    		Formatdisplay(humidity, numbuf);
    		SSD1306_SetCursor(2, 28);
    		snprintf(line, sizeof(line), "Do am: %s%%", numbuf);
    		SSD1306_WriteString(line);

    		SSD1306_SetCursor(2, 38);
    		snprintf(line, sizeof(line), "Anh sang: %d lux", (int)lux);
    		SSD1306_WriteString(line);

    		SSD1306_SetCursor(1, 48);
    		if(smokePPM < 0) {
    			snprintf(line, sizeof(line), "Khoi: chua hieu chinh");
    		} else {
    			FormatforMQ2(smokePPM, numbuf);
    			snprintf(line, sizeof(line), "Khoi: %s ppm", numbuf);
    		}
    		SSD1306_WriteString(line);

    		SSD1306_UpdateScreen();

    	}

    	if(msTicks - lastSensorSend >= SENSOR_SEND_INTERVAL_MS) {
    		lastSensorSend = msTicks;
    		SensorData_t data = { temperature, humidity, smokePPM, lux };
    		UART_ESP32_SendSensorData(&data);
    	}

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

