/*----------------------------------------------------------------------------------------------------------------------------------------
**Проект: "Interrupt-NVIC".
**Назначение программы: псевдопараллельное выполнение двух задач:
**																									* мигание светодиодом с аппаратным контролем частоты переключения;
**																									* определение ASCII-кодов клавиш, нажимаемых на удаленном терминале;
**Разработчик: Грабовский Александр Сергеевич - 1191б
**Цель: создание программы, использующей внутренние прерывания микроконтроллера STM32F072RBT 
**Решаемые задачи:
**		1. Реализация функции-обработчика внутреннего прерывания микроконтроллера STM32F072RBT (USART);
**		2. Конфигурирование NVIC;
**		3. Настройка модуля USART на генерацию сигнала прерывания при возникновении заданных событий.
**----------------------------------------------------------------------------------------------------------------------------------------*/


#include "main.h"

static uint8_t buf[256];							 											//Буфер данных, передаваемых на ПК посредством USART
static uint32_t iReadyTX, iCompleteTX; 											//Количество битовых пакетов готовых для передачи и переденных на ПК соответственно int main ()

int main()
{
	//---Настройка аппаратных стредств микроконтроллера
	uint32_t half_period, n;																	//Половина периода базовой задержки переключения светодиода и степень коэффициента задержки: К=2^n
	__disable_irq();																					/*Глобальное запрещение прерываний. После вызова этой функции  преостановить работу 
																															функции main() не способен ни один из обработчиков прерывания*/
	
	//Настройка порта GPIOB для контроля светодиода (настройка аппаратных средств микроконтроллера)
	RCC->AHBENR|=RCC_AHBENR_GPIOBEN;													//Включение тактирования порта В
	GPIOB->MODER|= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER8_0; //Переключение линий 0 и 8 порта В в режим "Output"
	GPIOB->MODER&=~ (GPIO_MODER_MODER12 | GPIO_MODER_MODER13);//Переключение линий 12(SW4) и 13(SW3) порта В в режим "Input"
	GPIOB->ODR|=0x100;																				//Разрешение работы светодиодов на стенде CТМ_01 с помощью установки логической "1" на выводе РВ.8									
	//---------------------------------------------
	iReadyTX = 0;																							//Сброс количества битовых пакетов, подготовленных для передачи на ПК
	iCompleteTX = 0;																					//Сброс количества битовых пакетов, переданных на ПК через USART
	InitUSART1();																							//Инициализация модуля USART1
	NVIC->ISER[0] |= 0x08000000; 															//Разрешение и просмотр статуса каналов прерывания в NVIC от модуля USART1 	
	__enable_irq();																						//Глобальное разрешение прерываний в микроконтроллере 

	half_period=50000;																				//Инициализация переменной, хранящей половину периода переключения светодиода
	
	/*---Производится синхронный опрос состояния микропереключате-лей SW3 и SW4 и 
	реализуется программная задержка с помощью функции delay() после включения и выключения светодиода.*/
	while(1){
		n=((GPIOB->IDR)&0x3000)>>12;														//Определение степени коэффициента базовой задержки: К=2^n
		GPIOB->BSRR= 0x1;																				//Зажечь светодиод, подключенный к выводу РВ.О
		delay(half_period<<n);																	//Задержка после включения светодиода
		GPIOB->BSRR=0x10000; 																		//Погасить светодиод, подключенный к выводу РВ.О
		delay(half_period<<n);																	//Задержка после выключения светодиода 
	}
}

//Функция инициализации USART лабораторного комплекса
void InitUSART1(){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;											//Включение тактирования USART1
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;												//Включение тактирования порта А
	
	//Настройка линий порта А: РА9(ТХ_1) - выход передатчика; PA10(RX_1) - вход приёмника
	GPIOA->MODER |= 0x00280000;																//Перевести линии РА9 и РА10 в режим альтернативной функции
	GPIOA->AFR[1] |= 0x00000110;															//Включить на линиях РА9 и РА10 альтернативную функцию AF1
	
	//Настройка линии передатчика Тх (РА9)
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;												//Сбросить 9 бит GPIOA->OTYPER - переключение в режим push-pull для линии РА9 (активный выход) 
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9;												//Отключение подтягивающих резисторов для линии РА9 
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9;									//Установка высокой скорости синхронизации линии РА9
	
	//Настройка линии приемника Rx (РА10)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;											//Сброс режима подтягивающих резисторов для линии РА10
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;											//Включение подтягивающего резистора pull-up на входной линии РА10 (вход приемника Rx)
	
	//Конфигурирование USART
	USART1->CR1 &= ~USART_CR1_UE;															//Запрещение работы модуля USART1 для изменения параметров его конфигурации
	USART1->BRR=69;																						/*Настройка делителя частоты, тактирующего USART и задающего скорость приема и передачи данных на уровне 115200 бит/с: 
																															Частота тактирующего генератора = 8 МГц 
																															Скорость обмена по USART - 115200 бит/с; коэффициент деления - 8000000 / 115200 - 69,4444(4); Округленное значение - 69*/
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;								/*Разрешить работу приемника и передатчика USART. Остальные биты этого регистра сброшены, что обеспечивает: 
																															количество бит данных в пакете 8;
																															контроль четности - отключен; 
																															прерывания по любым флагам USART - запрещены;
																															состояние USART - отключен*/
	USART1->CR1 |= USART_CR1_RXNEIE | USART_CR1_TCIE; 				/*Разрешение (в модуле USART1) на выдачу сигнала прерывания при возникновении событий:
																															прием кадра в буферный регистр(RXNE); завершение передачи кадра(TC). 
																															А так же подавал сигнал запроса прерывания в NVIC. */
	USART1->CR2 = 0;																					//Количество стоповых бит - 1
	USART1->CR3 = 0;																					//DMA1 - отключен
	USART1->CR1 |= USART_CR1_UE;															//По завершении конфигурирования USART разрешить его работу (биту UE регистра CR1 присвоить 1)
}

//Функция задержки: count - количество элементарных периодов задержки с длительностью примерно 2.5 мкс 
void delay(uint32_t count)
{
	volatile uint32_t i;																			//объявляем неоптимизируемую переменную
	for (i=0;i<count;i++);																		//Выполнение пустых циклов для реализации программной задержки
}

//Функция-обработчик прерывани(позволяет минимизировать время реакции на событие) от модуля USART1.
void USART1_IRQHandler(void)
{
	uint8_t pack, d100;
	//Событие готовности принятых данных к чтению.(прерывание произошло позавершению приёма)
	if (USART1->ISR & USART_ISR_RXNE) { 											//Если в регистре состояний USART1 установлен флаг "RXNE"(прерывание произошло по приёму), то
		pack=(uint8_t)USART1->RDR; 															//Чтение принятого битового пакета из буферного регистра приемника USART1 
		d100=(uint8_t)pack/100; 																//Определение количества сотен в принятом значении
		pack -= d100*100;																				//Убрать из принятого значения сотни и оставить в числе 2 значащие цифры, которые соответствуют десяткам и единицам 
		buf[(uint8_t)iReadyTX++] = d100+48;											//Представить количество сотен в принятом значении в виде ASCII-кода и разместить полученные данные в буфере для передачи на ПК; 
																														//увеличить количество данных готовых для передачи на единицу
		buf[(uint8_t)iReadyTX++] = (uint8_t)pack/10+48;					//Представить количество десятков в принятом значении в виде ASCII-кода и разместить полученные данные в буфере для передачи на ПК
																														//увеличить количество данных готовых для передачи на единицу
		buf[(uint8_t)iReadyTX++] = (uint8_t)pack%10+48;					//Представить количество единиц в принятом значении в виде ASCII-кода и разместить полученные данные в буфере для передачи на ПК;
																														//увеличить количество данных готовых для передачи на единицу
		buf[(uint8_t)iReadyTX++] = 32;													//Поместить после цифр принятого значения разделитель - "пробел";
																														//увеличить количество данных готовых для передачи на единицу
		while ((USART1->ISR & USART_ISR_TXE) == 0) {} 					//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
		USART1->TDR = buf[(uint8_t)iCompleteTX++];							//Отправить старшую цифру ASCII-кода в передатчик USART1; 
																														//увеличить количество переданных на ПК данных на единицу
	}
	
	//Событие завершение передачи битового пакета.(прерывание произошло по завершению передачи)
	if (USART1->ISR & USART_ISR_TC) {													//Если в регистре состояний USART1 установлен флаг "ТС"(завершение передачи кадра), то
		
		// Сброс флага(TC) завершения передачи кадра 
		USART1->ICR=USART_ICR_TCCF;															//Сбросить флаг "ТС"(завершения передачи кадра), чтобы прерывание не сработало повторно
		
		//Если количество переданных данных меньше, чем количество подготовленных для передачи, то передать следующий битовой пакет из программного буфера в USART1 для отправки на ПК
		if (iCompleteTX<iReadyTX){
			USART1->TDR = buf[(uint8_t)iCompleteTX++];
		}
	}	
}

/*----------------------------------------------------------------------------------------------------------------------------------------
**Руководство пользователя:
**		1. Запустите программу на лабораторном комплексе;
**		2. В управлении частотой мигания светодиода используйте микропереключатели SW3(старший разряд) и SW4(младший разряд). Примерная частота мигания: 00-4 Гц; 01-2 Гц; 10-1 Гц; 11 - 0.5 Гц;
**		3. На компьютере запустите приложение PuTTY и подключитесь к соответствующему COM-порту на скорости 115200 бит/с;
** 		4. При активном окне терминала нажмите различные кнопки клавиатуры и отследите ASCII-код каждой клавиши (управляющим клавишам должны соответствовать последовательности ASCII-кодов);
**----------------------------------------------------------------------------------------------------------------------------------------*/
