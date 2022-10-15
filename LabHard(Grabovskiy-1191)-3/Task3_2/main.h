#include "stm32f0xx.h"                  // Device header
void InitUSART1(void);
void USART1_IRQHandler(void);
void delay(uint32_t);
uint32_t powi(uint32_t, uint32_t);
void updateDelay(void);
void outFirstChar(void);
