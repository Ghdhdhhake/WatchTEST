#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

#define DELAY_DWT_CTRL   (*(volatile uint32_t *)0xE0001000UL)
#define DELAY_DWT_CYCCNT (*(volatile uint32_t *)0xE0001004UL)
#define DELAY_DWT_CYCCNT_ENABLE (1UL << 0)

void Delay_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DELAY_DWT_CYCCNT = 0;
	DELAY_DWT_CTRL |= DELAY_DWT_CYCCNT_ENABLE;
}

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{
	uint32_t start;
	uint32_t cycles;

	if((DELAY_DWT_CTRL & DELAY_DWT_CYCCNT_ENABLE) == 0)
	{
		Delay_Init();
	}

	start = DELAY_DWT_CYCCNT;
	cycles = (SystemCoreClock / 1000000UL) * xus;
	while((uint32_t)(DELAY_DWT_CYCCNT - start) < cycles)
	{
	}
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	if((xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) && (xms > 0))
	{
		vTaskDelay(pdMS_TO_TICKS(xms));
	}
	else
	{
		while(xms--)
		{
			Delay_us(1000);
		}
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs > 0)
	{
		Delay_ms(1000U);
		xs--;
	}
} 
