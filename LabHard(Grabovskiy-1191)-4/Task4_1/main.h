#include "stm32f0xx.h" //Заголовок устройства
void InitPortB(void);  //Декларация функции инициализации порта B
void InitTimer6(void); //Декларация функции инициализации таймера
void TIM6_DAC_IRQHandler(void); /*Декларация функции обработки 
																	прерывания от таймера TIM6*/
