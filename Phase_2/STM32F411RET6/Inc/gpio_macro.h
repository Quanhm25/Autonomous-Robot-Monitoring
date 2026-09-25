#ifndef GPIO_MACRO_H
#define GPIO_MACRO_H

#include "stm32f4xx.h"

#define PINSET(port, pin) ((port)->BSRR = (1<<pin))
#define PINRESET(port, pin) ((port)->BSRR = (1<<(pin + 16)))

#endif
