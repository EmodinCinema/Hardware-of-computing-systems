#include "stm32f0xx.h" 								//����������� ���-�� � ������� stm32f0xx
void InitUSART1(void); 								//���������� ������� ������������� USART1
void USART1_IRQHandler(void); 				//���������� ������� ��������� ���������� �� USART1
void delay(uint32_t);									//���������� ������� ��������
uint32_t powi(uint32_t, uint32_t);		//���������� ������� ��������� �������
void updateDelay(void);								//���������� ������� ���������� ��������
void outFirstChar(void);							//���������� ������� ������ ������� �������
