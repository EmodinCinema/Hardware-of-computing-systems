#include "stm32f0xx.h"						//	����������� ���������� � ������� stm32f0xx
void InitPortB(void);							//	���������� ������� ������������� ����� B
void InitTimer(void);						//	���������� ������� ������������� ������� TIM6
void TIM6_DAC_IRQHandler(void);		//	���������� ������� ��������� ����������� �� ������� TIM6
void TIM7_IRQHandler(void);		//	���������� ������� ��������� ����������� �� ������� TIM7
void InitMass(void);
