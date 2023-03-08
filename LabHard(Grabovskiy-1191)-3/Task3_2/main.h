#include "stm32f0xx.h" 								//Подключение биб-ки с моделью stm32f0xx
void InitUSART1(void); 								//Декларация функции инициализации USART1
void USART1_IRQHandler(void); 				//Декларация функции обработки прерывания от USART1
void delay(uint32_t);									//Декларация Функции задержки
uint32_t powi(uint32_t, uint32_t);		//Декларация функции получения степени
void updateDelay(void);								//Декларация функции обновление задержки
void outFirstChar(void);							//Декларация функции вывода первого символа
