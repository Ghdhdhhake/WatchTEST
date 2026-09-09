#include "AppTasks.h"
#include "task.h"
#include "menu.h"
#include "encoder.h"
#include "dino.h"

#define APP_UI_TASK_STACK_WORDS       768U
#define APP_INPUT_TASK_STACK_WORDS    192U
#define APP_TIMEBASE_TASK_STACK_WORDS 128U

#define APP_UI_TASK_PRIORITY          2U
#define APP_INPUT_TASK_PRIORITY       3U
#define APP_TIMEBASE_TASK_PRIORITY    4U

static void AppUiTask(void *argument)
{
    int next_page;

    (void)argument;

    for( ;; )
    {
        next_page = First_Page_Clock();

        if(next_page == 1)
        {
            Menu();
        }
        else if(next_page == 2)
        {
            SettingPage();
        }
    }
}

static void AppInputTask(void *argument)
{
    TickType_t last_wake_time;

    (void)argument;
    last_wake_time = xTaskGetTickCount();

    for( ;; )
    {
        Encoder_Poll();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(2U));
    }
}

static void AppTimebaseTask(void *argument)
{
    TickType_t last_wake_time;

    (void)argument;
    last_wake_time = xTaskGetTickCount();

    for( ;; )
    {
        StopWatch_Tick();
        Dino_Tick();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1U));
    }
}

BaseType_t AppTasks_Create(void)
{
    BaseType_t result;

    if(!Encoder_IsReady())
    {
        return pdFAIL;
    }

    result = xTaskCreate(AppTimebaseTask, "Timebase",
                         APP_TIMEBASE_TASK_STACK_WORDS, NULL,
                         APP_TIMEBASE_TASK_PRIORITY, NULL);
    if(result != pdPASS)
    {
        return pdFAIL;
    }

    result = xTaskCreate(AppInputTask, "Input",
                         APP_INPUT_TASK_STACK_WORDS, NULL,
                         APP_INPUT_TASK_PRIORITY, NULL);
    if(result != pdPASS)
    {
        return pdFAIL;
    }

    result = xTaskCreate(AppUiTask, "UI",
                         APP_UI_TASK_STACK_WORDS, NULL,
                         APP_UI_TASK_PRIORITY, NULL);

    return result;
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    taskDISABLE_INTERRUPTS();
    for( ;; )
    {
    }
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for( ;; )
    {
    }
}
