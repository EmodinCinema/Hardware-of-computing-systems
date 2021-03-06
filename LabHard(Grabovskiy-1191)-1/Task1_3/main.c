//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Разработчик проекта: Грабовский Александр Сергеевич - группа 1191б
//
//Цель: разработать программу для вывода чисел в восьмиричной системы счисления на семисегментном индикаторе
//
//Решаемые проектом задачи:
//       1.На сонове константных и варьируемых бит определять значение числа в десятичной системе счисления
//			 2.Преобразовать число из десятичной системы счисления  в систему с основанием восемь
//       3.Кодировать каждую цифру числа символом для семисегментного индикатора
//       4.Выводить все цифры числа на семисегментном индикаторе, начиная со старшей цифры
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "main.h"                   //Заголовочный файл с описанием библиотечных модулей

//main-обязательная функция для исполнения кода пользователя
//После подачи питания или счётчика с кнопкой "reset" стартует Загрузчик, который выполняет начальную настроку основных регистров микроконтроллера
//В завершении работы Загрузчик передаёт управление микроконтроллером функции main
int main(void)
{
	//Конфигурация поортов GPIOB
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN; 				//Включение тактирование порта B: RCC_AHBENR_GPIOBEN=0x00040000 
	GPIOB->OSPEEDR |= 0x00005555;							//Установка частоты преключения  выводов PB.0-PB.7 на уровне 10 МГц
	GPIOB->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0 | 						//Переключение линии 0-7 и 9 порта B в режим "Output" 
	                                                                 GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | 
	                                                                 GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER9_0;	
	GPIOB->MODER&=~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | 																		//Пререключаем линии 12(SW4),(SW3),(SW2),(SW1) порта B в режим "Input"
	                                                                 GPIO_MODER_MODER14 | GPIO_MODER_MODER15);
	
  uint16_t input[8]={0,0,0,0,1,0,0,0};			//Входное значение числа (в двоичной системе)
  uint16_t n=0;															//Значение числа в десятичной системе счисления
	uint16_t output[3]={0,0,0};								//Массив с числом в троичной системе (в обратном порядке)
  unsigned reg[8]={0x0000023F, 0x00000206, 0x0000025B, 0x0000024F, 0x00000266, 0x0000026D, 0x0000027D, 0x00000207};									//Регистры семисегментного индикатора, цифры  0,1,2,3,4,5,6,7
	//Реализация на семисегментный индикатор
	while(1){
		n=0;
		output[0]=0;															//Присваеваем значение массиву "output"
		output[1]=0;															//Присваеваем значение массиву "output"
		output[2]=0;															//Присваеваем значение массиву "output"
		input[0]=((GPIOB->IDR)&0x8000)>>15;				//Считываем значение переключателя SW1
		input[3]=((GPIOB->IDR)&0x4000)>>14;				//Считываем значение переключателя SW2
		input[5]=((GPIOB->IDR)&0x2000)>>13;				//Считываем значение переключателя SW3
		input[7]=((GPIOB->IDR)&0x1000)>>12;				//Считываем значение переключателя SW4
		uint32_t j=7;															//Разряд двоичного числа
		
		//Перевод числа в десятичную систему
		for(uint16_t i=0; i<8; i++){
			n += input[i]* powi (2, j);							//Расчёт числа в десятичной системе 
			j--;																		//Переход к следующему разряду
		}
		
		//Перевод числа в восьмеричную систему счисления 
		for(uint16_t i=0; i<3; i++){
			if(n<8){																//Условие срабатывает, если десятичное число меньше разрядности
				output[i]=n;													//В выходной массив записывается значение числа
				break;																//Цикл прерывается 
			}
			output[i]= n%8;													//В выходной массив записывается остаток от деления на 8
			n=n/8;																	//В число n записывается целая часть от деления
		}
		
		//Ввод числа на семисегментном индикаторе
		for(int16_t i=2; i>=0; i--){							
			if(i == 2 & output[i] == 0){						//Отбрасываем 0 в старшей цифре числа (если он есть)
				continue;}
			GPIOB->BSRR |= reg[output[i]];					//Вывод циры тчисла
				delay(200000);										    //Задержка в 2 секунды
				GPIOB->BSRR |= 0xffff0000;				    //Включение ндикатора 
				delay(50000);													//Задержка в 0,5 секунды
			}
			GPIOB->BSRR |= 0x00000280;							//Вывод точки
			delay(500000);													//Задержка в 0,5 секунды  
			GPIOB->BSRR |= 0xffff0000;							//Выключение индикатора
		
		}
}

//powi функция для вычисления степени числа
//x- число которое нужно возвести в степень, n-степень в которую нужно возвести
//возвращает число возведённое в степень
uint32_t powi(uint32_t x, uint32_t n)
{
	if(n == 0)																	//Если степень равна 0
		return 1;																	//Возвращается единица 
	else if(n == 1)															//Если степень равна
		return x;																	//Возврщается искомое число
	else if(n % 2 == 0)													//Если степень чётная 
		return powi(x*x, n/2);										//Производится рекурсия , передаётся число умноженное на себя и половина степени
	else																				//Если степень нечёная 
		return powi(x*x, n/2)*x;									//Производится рекурсия, передаётся число умноженное на себя и  половина степени
	}																						//Результат умножается на искомое 

	//Функция задержки: count-колличество элементарных периодов задержки с длительностью примерно 2,5 мкс
void delay(uint32_t count)
{
	volatile uint32_t i;												//Объявляем неоптимизируемую переменную	
  for(i=0;i<count;i++);												//Выволнеие пустых циклов для реализации програмной задержки

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Руководство пользователя:
//			1. Для запуска загруженной программы удалите перемычку "boot" на стенде и нажмите "reset"
//			2. Число задаётся с помощью переключателей SW1, SW2, SW3, SW4
//			3. Задаваемое число имеет вид: (SW1)00(SW2)1(SW3)0(SW4)
//			4.  На семисегментном индикаторе, начиная со старшей цифры, выводится число, в окончании выводится точка
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

