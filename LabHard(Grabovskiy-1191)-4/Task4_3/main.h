#include "stm32f0xx.h"						//	Подключение библиотеки с моделью stm32f0xx
void InitPortB(void);							//	Декларация функции инициализации порта B
void InitTimer(void);						//	Декларация функции инициализации таймера TIM6
void TIM6_DAC_IRQHandler(void);		//	Декларация функции обработки прперывания от таймера TIM6
void TIM7_IRQHandler(void);		//	Декларация функции обработки прперывания от таймера TIM7
void InitMass(void);
