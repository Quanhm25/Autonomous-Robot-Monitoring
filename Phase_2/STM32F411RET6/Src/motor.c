#include "motor.h"
#include "gpio_macro.h"

//TB6612FNG và 4 DC motor
void Motor_Init(void) {
    // 1. Cấp xung nhịp cho GPIOB và GPIOC
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    GPIOC->MODER &= ~((3U<<0) | (3U<<2) | (3U<<4) | (3U<<6));
    GPIOC->MODER |=  ((1U<<0) | (1U<<2) | (1U<<4) | (1U<<6));
}

//Điều hướng động cơ
void Motor_Forward(void) {
    PINSET(GPIOC, 0);   PINRESET(GPIOC, 1);
    PINSET(GPIOC, 2);   PINRESET(GPIOC, 3);
}

void Motor_Backward(void) {
    PINRESET(GPIOC, 0); PINSET(GPIOC, 1);
    PINRESET(GPIOC, 2); PINSET(GPIOC, 3);
}

void Motor_Left(void) {
    // Rẽ TRÁI: Động cơ cụm TRÁI dừng, Động cơ cụm PHẢI chạy TỚI
    PINSET(GPIOC, 0);  PINRESET(GPIOC, 1);    // Trái dừng
    PINRESET(GPIOC, 2);    PINRESET(GPIOC, 3);    // Phải chạy tới
}

void Motor_Right(void) {
    // Rẽ PHẢI: Động cơ cụm TRÁI chạy TỚI, Động cơ cụm PHẢI dừng
    PINRESET(GPIOC, 0);    PINRESET(GPIOC, 1);    // Trái chạy tới
    PINSET(GPIOC, 2);  PINRESET(GPIOC, 3);    // Phải dừng
}
void Motor_Stop(void) {
    PINRESET(GPIOC, 0); PINRESET(GPIOC, 1);
    PINRESET(GPIOC, 2); PINRESET(GPIOC, 3);
}
