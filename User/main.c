#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "menu.h"
#include "AppTasks.h"
#include "FreeRTOS.h"
#include "task.h"
//This si from branch dev
/**
  * 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~127
  * 纵向向下为Y轴，取值范围：0~63
  * 
  *       0             X轴           127 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  * 
  */

int main(void)
{
	Delay_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	OLED_Init();
	Peripheral_Init();

	if(AppTasks_Create() != pdPASS)
	{
		for( ;; )
		{
		}
	}

	vTaskStartScheduler();

	for( ;; )
	{
	}
}

