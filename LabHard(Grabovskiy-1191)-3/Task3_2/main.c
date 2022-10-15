#include "main.h"

static uint8_t buf[256];							 //Буфер данных, передаваемых на ПК посредством USART
static uint32_t iReadyTX, iCompleteTX; //Количество битовых пакетов готовых для передачи и переденных на ПК соответственно int main ()
static uint32_t delayCountArr[6]={1, 2, 3, 4, 5, 6};
static uint32_t tempDelayArr[6]={0, 0, 0, 0, 0, 0};
static  int8_t count= -1;
static uint32_t delayCount= 0;
static uint8_t flag= 0;

int main()
{
	__disable_irq();					//Глобальное запрещение прерываний
	
	//Настройка порта GPIOB для контроля светодиода
	RCC->AHBENR|=RCC_AHBENR_GPIOBEN;													//Включение тактирования порта В
	GPIOB->MODER|= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER8_0; //Переключение линий 0 и 8 порта В в режим "Output"
	GPIOB->MODER&=~ (GPIO_MODER_MODER12 | GPIO_MODER_MODER13);//Переключение линий 12(SW4) и 13(SW3) порта В в режим "Input"
	GPIOB->ODR|=0x100;																				//Разрешение работы светодиодов на стенде CТМ_01 с помощью установки логической "1" на выводе РВ.8									
	//---------------------------------------------
	iReadyTX = 0;																							//Сброс количества битовых пакетов, подготовленных для передачи на ПК
	iCompleteTX = 0;																					//Сброс количества битовых пакетов, переданных на ПК через USART
	InitUSART1();																							//Инициализация модуля USART1
	NVIC->ISER[0] |= 0x08000000; 															//Разрешение в NVIC прерывания от модуля USART1 	
	__enable_irq();																						//Глобальное разрешение прерываний 

	while(1){
		for(int32_t i=5, j=0; i>-1;  i--, j++){
			delayCount=0;
			delayCount+=delayCountArr[i]*powi(10, (uint32_t)j);
		}
		GPIOB->BSRR=0x1;
		delay(delayCount);
		GPIOB->BSRR=0x10000;
		delay(delayCount);
	}
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
void delay(uint32_t counts)
{
	volatile uint32_t i;																				//объявляем неоптимизируемую переменную
	for (i=0;i<counts;i++);																			//Выполнение пустых циклов для реализации программной задержки
}

//Функция-обработчик прерывания от модуля USART1 
void USART1_IRQHandler(void)
{
	uint8_t pack;
	//массив для вывода сообщения: "Период мигания, мкс: "
	uint8_t textStatus[23]= {0xCF, 0xE5, 0xF0, 0xE8, 0xEE, 0xE4, 0x20, 0xEC, 0xE8, 0xE3, 0xE0, 0xED, 0xE8, 0xFF, 0x2C, 0x20, 0xEC, 0xEA, 0xF1, 0x3A, 0x20};
	//массив для вывода сообения: "Новое значение, мкс: "	
	uint8_t textInput[23]= {0xD, 0xA, 0xCD, 0xEE, 0xE2, 0xEE, 0xE5, 0x20, 0xE7, 0xED, 0xE0, 0xF7, 0xE5, 0xED, 0xE8, 0xE5, 0x2C, 0x20, 0xEC, 0xEA, 0xF1, 0x3A, 0x20};
		
		
  if(USART1->ISR & USART_ISR_RXNE){
			pack=(uint8_t)USART1->RDR;
	
		switch(pack){
			case 0x0D:
				if(flag == 0){
					for(uint8_t i=0; i<23; i++){
						buf[(uint8_t)iReadyTX++]=textStatus[i];
					}
					for(uint8_t i=0; i<6; i++){
						buf[(uint8_t)iReadyTX++]=(uint8_t)delayCountArr[i]+48;
					}
					for(uint8_t i=0; i<22; i++){
						buf[(uint8_t)iReadyTX++]=textInput[i];
					}
					flag=1;
					outFirstChar();
				}
				else{
					flag=0;
					updateDelay();
					buf[(uint8_t)iReadyTX++]=0xD;
					buf[(uint8_t)iReadyTX++]=0xA;
					outFirstChar();
				}
				break;
				
			case 0x30:
				if(count<5){
					count++;
					tempDelayArr[count]=0;
					buf[(uint8_t)iReadyTX++]=0x30;
					outFirstChar();
				}
				break;
				
			case 0x31:
				if(count<5){
					count++;
					tempDelayArr[count]=1;
					buf[(uint8_t)iReadyTX++]=0x31;
					outFirstChar();
				}
				break;
				
			case 0x32:
				if(count<5){
					count++;
					tempDelayArr[count]=2;
					buf[(uint8_t)iReadyTX++]=0x32;
					outFirstChar();
				}
				break;
				
			case 0x33:
				if(count<5){
					count++;
					tempDelayArr[count]=3;
					buf[(uint8_t)iReadyTX++]=0x33;
					outFirstChar();
				}
				break;
				
			case 0x34:
				if(count<5){
					count++;
					tempDelayArr[count]=4;
					buf[(uint8_t)iReadyTX++]=0x34;
					outFirstChar();
				}
				break;
				
			case 0x35:
				if(count<5){
					count++;
					tempDelayArr[count]=5;
					buf[(uint8_t)iReadyTX++]=0x35;
					outFirstChar();
				}
				break;
				
			case 0x36:
				if(count<5){
					count++;
					tempDelayArr[count]=6;
					buf[(uint8_t)iReadyTX++]=0x36;
					outFirstChar();
				}
				break;
				
			case 0x37:
				if(count<5){
					count++;
					tempDelayArr[count]=7;
					buf[(uint8_t)iReadyTX++]=0x37;
					outFirstChar();
				}
				break;
				
			case 0x38:
				if(count<5){
					count++;
					tempDelayArr[count]=8;
					buf[(uint8_t)iReadyTX++]=0x38;
					outFirstChar();
				}
				break;
				
			case 0x39:
				if(count<5){
					count++;
					tempDelayArr[count]=9;
					buf[(uint8_t)iReadyTX++]=0x39;
					outFirstChar();
				}
				break;
				
			case 127:
				if(count>-1){
					tempDelayArr[count]=9;
					while((USART1->ISR & USART_ISR_TXE)==0){}
					count--;
				}
				break;
		}
	}

	//Событие завершение передачи битового пакета 
	if (USART1->ISR & USART_ISR_TC) {														//Если в регистре состояний USART1 установлен флаг "ТС", то
		// Сброс флага завершения передачи кадра 
		USART1->ICR=USART_ICR_TCCF;																//Сбросить флаг завершения передачи кадра, чтобы прерывание не сработало повторно
		//Если количество переданных данных меньше, чем количество подготовленных для передачи, то передать следующий битовой пакет из программного буфера в USART1 для отправки на ПК
		if (iCompleteTX<iReadyTX){
			USART1->TDR = buf[(uint8_t)iCompleteTX++];
		}
	}	
}

uint32_t powi(uint32_t x, uint32_t n){
	if(n==0)
		return 1;
	else if(n==1)
		return x;
	else if(n%2==0)
		return powi(x*x,n/2);
	else
		return powi(x*x,n/2)*x;
}

void updateDelay(){
	if(count>-1){
		for(uint8_t i=0; i<6; i++){
			delayCountArr[i]=0;
		}
		for(int8_t i=5; i>=0;i--){
			delayCountArr[i]=tempDelayArr[count];
			count--;
			if(count==-1){
				break;
			}
		}
		for(uint8_t i=0; i<6; i++){
			tempDelayArr[i]=0;
		}
		count=-1;
	}
}

void outFirstChar(){
	while((USART1->ISR & USART_ISR_TXE)==0){}
	USART1->TDR=buf[(uint8_t)iCompleteTX++];
}
