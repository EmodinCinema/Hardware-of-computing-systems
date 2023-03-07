#include "stm32f0xx.h"   	     //Подключение биб-ки с моделью stm32f0xx 
void InitUSART1(void);  	     //Декларация функции инициализации USART1 
void USART1_IRQHandler(void);  //Декларация функции обработки прерывания от USART1 
                           
//Функция задержки: count – кол-во периодов задержки длительностью 2.5 мкс 
void delay(uint32_t); 
