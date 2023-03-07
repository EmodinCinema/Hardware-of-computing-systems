/*----------------------------------------------------------------------------------------------------------------------------------------
**Проект: "Программный отладчик".
**Назначение программы: добавление в код программы функции для приостановки ее выполнения, и вывода текущих значений переменных тестируемой программы.
**Разработчик: Грабовский Александр Сергеевич - 1191б
**Цель: создание функции, которая добавляется в код тестируемой программы для приостановки ее выполнения, 
**			реагирует на команды удаленного терминала и позволяет отображать в нем текущие значения переменных тестируемой программы 
**Решаемые задачи:
**		1. Реализация функции-обработчика внутреннего прерывания микроконтроллера STM32F072RBT (USART);
**		2. Конфигурирование NVIC;
**		3. Создание функции, приостанавливающий пргорамму и выодящей текущие значения переменных.
**----------------------------------------------------------------------------------------------------------------------------------------*/

#include "main.h"

static uint8_t k=0;
//--------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------

static int32_t mes[256];//Буфер данных, передаваемых на ПК посредством USART
static int8_t flag=0x0000;//Флаг текущего сотояния функции debug
//static int32_t a=8,sum=8;
//static uint8_t i=1;

static int32_t sum = 26;//Переменная хранящая сумму членов арифметики 
static int32_t i = 1;//Переменная хранящая количество итераций
//static int32_t q=-1;
static int32_t a=26;//Переменная хранящая текущее значение
//static uint8_t flag;//Флаг текущего сотояния функции debug
//static uint8_t flagF = 0;
int main()
{
	
	
	__disable_irq();//Глобальное запрещение прерываний
	//--------------------------------------------------------------------------------------------------------------------------------------
	uint32_t half_period, n;
	//Настройка порта GPIOB для контроля светодиода (настройка аппаратных средств микроконтроллера)
	RCC->AHBENR|=RCC_AHBENR_GPIOBEN;													//Включение тактирования порта В
	GPIOB->MODER|= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER8_0; //Переключение линий 0 и 8 порта В в режим "Output"
	GPIOB->MODER&=~ (GPIO_MODER_MODER12 | GPIO_MODER_MODER13);//Переключение линий 12(SW4) и 13(SW3) порта В в режим "Input"
	GPIOB->ODR|=0x100;																				//Разрешение работы светодиодов на стенде CТМ_01 с помощью установки логической "1" на выводе РВ.8									
	//--------------------------------------------------------------------------------------------------------------------------------------
	
	static int32_t q=-1;//Переменная хранящая разность
	//int8_t q=-5;
	InitUSART1();//Инициализация модуля USART1																						
	NVIC->ISER[0] |= 0x08000000;//Разрешение в NVIC прерывания от модуля USART1  													
	__enable_irq();//Глобальное разрешение прерываний 	

// Выполнение арифметической прогрессии	
	while(i<18)//По проишествии 17 итераций
	{														
		debug();//Вызов функции debug
		a=a+q;//Добавление к текущему значению разности арифметической прогрессии		
		sum=sum+a;//Вычисление суммы всех членов прогрессии
		i++;//Увелить на 1 переменную отображающую количество итераций
	}
	//--------------------------------------------------------------------------------------------------------------------------------------
		half_period=50000;	
	while(1){
		n=((GPIOB->IDR)&0x3000)>>12;														//Определение степени коэффициента базовой задержки: К=2^n
		GPIOB->BSRR= 0x1;																				//Зажечь светодиод, подключенный к выводу РВ.О
		delay(half_period<<n);																	//Задержка после включения светодиода
		GPIOB->BSRR=0x10000; 																		//Погасить светодиод, подключенный к выводу РВ.О
		delay(half_period<<n);																	//Задержка после выключения светодиода 
	}
	//--------------------------------------------------------------------------------------------------------------------------------------
}

//Функция инициализации USART лабораторного комплекса
void InitUSART1(){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;													//Включение тактирования USART1
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;														//Включение тактирования порта А
	
	//Настройка линий порта А: РА9(ТХ_1) - выход передатчика; PA10(RX_1) - вход приёмника
	GPIOA->MODER |= 0x00280000;																		//Перевести линии РА9 и РА10 в режим альтернативной функции
	GPIOA->AFR[1] |= 0x00000110;																	//Включить на линиях РА9 и РА10 альтернативную функцию AF1
	
	//Настройка линии передатчика Тх (РА9)
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;														//Сбросить 9 бит GPIOA->OTYPER - переключение в режим push-pull для линии РА9 (активный выход) 
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9;														//Отключение подтягивающих резисторов для линии РА9 
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9;											//Установка высокой скорости синхронизации линии РА9
	
	//Настройка линии приемника Rx (РА10)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;													//Сброс режима подтягивающих резисторов для линии РА10
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;													//Включение подтягивающего резистора pull-up на входной линии РА10 (вход приемника Rx)
	
	//Конфигурирование USART
	USART1->CR1 &= ~USART_CR1_UE;																	//Запрещение работы модуля USART1 для изменения параметров его конфигурации
	USART1->BRR=69;																								/*Настройка делителя частоты, тактирующего USART и задающего скорость приема и передачи данных на уровне 115200 бит/с: 
																																	Частота тактирующего генератора = 8 МГц 
																																	Скорость обмена по USART - 115200 бит/с; коэффициент деления - 8000000 / 115200 - 69,4444(4); Округленное значение - 69*/
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;										/*Разрешить работу приемника и передатчика USART. Остальные биты этого регистра сброшены, что обеспечивает: 
																																	количество бит данных в пакете 8;
																																	контроль четности - отключен; 
																																	прерывания по любым флагам USART - запрещены;
																																	состояние USART - отключен*/
	USART1->CR1 |= USART_CR1_RXNEIE | USART_CR1_TCIE; 						/*Разрешение (в модуле USART1) на выдачу сигнала прерывания при возникновении событий:
																																	прием кадра в буферный регистр; завершение передачи кадра */
	USART1->CR2 = 0;																							//Количество стоповых бит - 1
	USART1->CR3 = 0;																							//DMA1 - отключен
	USART1->CR1 |= USART_CR1_UE;																	//По завершении конфигурирования USART разрешить его работу (биту UE регистра CR1 присвоить 1)
}

//Функция задержки: count - количество элементарных периодов задержки с длительностью примерно 2.5 мкс 
void delay(uint32_t count)
{
	volatile uint32_t j;//объявляем неоптимизируемую переменную																			
	for (j=0;j<count;j++);//Выполнение пустых циклов для реализации программной задержки																			
}

//debug - функция приостановки выполнения программы и вывода текущих значений переменных
void debug(void)
{
	uint8_t mes_a[2]={0x61,0x3d};
	uint8_t mes_n[2]={0x6e,0x3d};
	uint8_t mes_q[2]={0x2d,0x31};
	uint8_t mes_s[2]={0x73,0x3d};
	while(1)
	{
		if((flag&0xF8)>>3==31)
		{
			flag=0;
			break;
		}
		if((flag&0x4)>>2==1)//i
		{																			
			for(uint8_t v=0;v<2;v++)
			{	
				while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						
				USART1->TDR = mes_n[v];
			}
			number_out(i);//Вызов функции number_out для вывода значения i
			while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
			USART1->TDR = 0x0D;//Переход на новую строку
			while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
			USART1->TDR = 0x0A;//Возврат каретки
			flag&=~0x4;
		}																												
		if((flag&0x2)>>1==1)//e
		{																					
			for(uint8_t v=0;v<3;v++)
			{	
				while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
				USART1->TDR = mes_a[v];
			}
			//number_out(a);
			while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
			USART1->TDR = 0x3d;
			number_out(a);//Вызов функции number_out для вывода значения a
			for(uint8_t v=0;v<3;v++)
			{	
				while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
				USART1->TDR = mes_q[v];
			}
			while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
			USART1->TDR = 0x0D;//Переход на новую строку
			while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
			USART1->TDR = 0x0A;//Возврат каретки

			flag&=~0x2;
		}														
		if((flag&0x1)==1)//s
		{
			
			for(uint8_t v=0;v<2;v++)
			{	
				while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
				USART1->TDR = mes_s[v];
			}
			number_out(sum);//Вызов функции number_out для вывода значения sum
			while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
			USART1->TDR = 0x0D;//Переход на новую строку
			while ((USART1->ISR & USART_ISR_TXE) == 0) {}//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК 						
			USART1->TDR = 0x0A;//Возврат каретки
			flag&=~0x1;
		}
	}
}

//Функция-обработчик прерывания от модуля USART1
void USART1_IRQHandler(void)
{
		uint8_t pack;//Переменная хранящая принятый пакет
		if (USART1->ISR & USART_ISR_RXNE)//Если в регистре состояний USART1 установлен флаг "RXNE", то 
			{ 												
		pack=(uint8_t)USART1->RDR;//Чтение принятого битового пакета из буферного регистра приемника USART1  																
		
				//Обработка на основе принятого пакета
				switch (pack) {															
		case 0x69://При нажатии на клавишу "i"																					
			flag|=0x4;																																																 
			break;//Выход из обработчика																						
		case 0x65://При нажатии на клавишу "e"																							
			flag|=0x2;																																																		
			break;//Выход из обработчика																							
		case 0x73://При нажатии на клавишу "s"																							
			flag|=0x1;																																																	
			break;//Выход из обработчика																								
			
		// Начало обработки нажатия на F5
		case 27://При вводе первого кода нажатия на F5																									
				flag|=0x8;																					
			break;//Выход из обработчика																								
		case 91://При вводе второго кода нажатия на F5																								
				flag|=0x10;																							
			break;//Выход из обработчика																								
		case 49://При вводе третьего кода нажатия на F5																									
				flag|=0x20;																									
			break;																									
		case 53://При вводе четвёртого кода нажатия на F5																									
				flag|=0x40;																										
			break;//Выход из обработчика																									
		case 126://При вводе пятого кода нажатия на F5																									
				flag|=0x80;																										
			break;//Выход из обработчика																																																		
		} 						
				
	}
	if (USART1->ISR & USART_ISR_TC)//Если в регистре состояний USART1 установлен флаг "TC", то  
		{
			USART1->ICR=USART_ICR_TCCF;//В регистре состояний USART1 установлен флаг "TCCF"
		}
}

//number_out функция для вывода числа
//num - число которое нужно вывести
void number_out(int32_t num)
{
	uint8_t j=k;
	uint8_t flag_0=0;//Переменная хранящая 
	//if(num<0)
	//{
	//	while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						
	//	USART1->TDR = 0x2d;
	//	num=num*(-1);
	//}
	for(;num!=0;k++)
	{
		mes[k]=(uint32_t)num%10+0x30;
		num=num/10;
	}	
	for(uint8_t d=0;k-j>=d;d++)
	{
			flag_0=1;
		
			//Дождаться готовности передатчика USART1 к приему битового пакета для отправки на ПК
			while ((USART1->ISR & USART_ISR_TXE) == 0) {} 						 						
			USART1->TDR = (uint8_t)mes[k-d];		
	}

}

/*----------------------------------------------------------------------------------------------------------------------------------------
**Руководство пользователя:
**		1. Запустите программу на лабораторном комплексе ЗТМ_01;
**		2. На компьютере запустите приложение PuTTY и подключитесь к соответствующему COM-порту на скорости 115200 бит/с;
** 		3. При нажатии на клавиши:
**			 - «i» - вывод в окне терминала PuTTY значения переменной с номером итерации вычислений;
**			 - «e» - вывод в окне терминала PuTTY значения переменной с элементом вычислений, соответствующим текущей итерации;
**			 - «s» - вывод в окне терминала PuTTY текущего значения переменной с накапливаемым результатом вычислений;
**			 - «F5» - переход на следующую итерацию
**----------------------------------------------------------------------------------------------------------------------------------------*/
