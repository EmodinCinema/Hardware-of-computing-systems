#include "stm32f0xx.h" //��������� ����������
void InitPortB(void);  //���������� ������� ������������� ����� B
void InitTimer6(void); //���������� ������� ������������� �������
void TIM6_DAC_IRQHandler(void); /*���������� ������� ��������� 
																	���������� �� ������� TIM6*/
