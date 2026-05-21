#include "timer.h"
#include "lvgl.h"

extern vu16 USART2_RX_STA;
extern vu16 USART6_RX_STA;

void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM7_Int_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// void TIM8_Int_Init(u16 arr, u16 psc)
// {
// 	NVIC_InitTypeDef NVIC_InitStructure;
// 	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

// 	TIM_TimeBaseStructure.TIM_Period = arr;
// 	TIM_TimeBaseStructure.TIM_Prescaler = psc;
// 	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
// 	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
// 	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

// 	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);

// 	NVIC_InitStructure.NVIC_IRQChannel = TIM8_IRQn;
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_Init(&NVIC_InitStructure);
// }

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		lv_tick_inc(1);
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
}

void TIM7_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		USART2_RX_STA |= 1 << 15;
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
		TIM_Cmd(TIM7, DISABLE);
	}
}

// void TIM8_IRQHandler(void)
// {
// 	if (TIM_GetITStatus(TIM8, TIM_IT_Update) == SET)
// 	{
// 		USART6_RX_STA |= 1 << 15;
// 		TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
// 	}
// }
