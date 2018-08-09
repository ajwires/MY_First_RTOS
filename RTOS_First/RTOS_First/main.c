#include <stm32f4xx_hal.h>
#include <stm32_hal_legacy.h>

#include <stdint.h>
#include "rtos.h"

OSThread blink_1;
OSThread blink_2;
uint32_t stack_blink_1[40];
uint32_t stack_blink_2[40];

#ifdef __cplusplus
extern "C"
#endif
	
	
/*void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}*/

void blink_1_func(void)
{
	while (1)
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_Delay(500);
	}
}
void blink_2_func(void)
{
	while (1)
	{
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_Delay(500);
	}
}

//put setup stuff here
void setup(void)
{
		
}

int main(void)
{
	HAL_Init();
	SystemInit();
	OS_init();

	__GPIOG_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.Pin = GPIO_PIN_14;

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	OSThread_start(&blink_1, &blink_1_func, stack_blink_1, sizeof(stack_blink_1));
	OSThread_start(&blink_2, &blink_2_func, stack_blink_2, sizeof(stack_blink_2));
	
	/* transfer control to the RTOS to run the threads */
	OS_run();
}
