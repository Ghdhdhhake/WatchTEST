#include "stm32f10x.h"
#include "delay.h"
#include "encoder.h"
#include "FreeRTOS.h"
#include "queue.h"

#define ENCODER_CLK_PIN      GPIO_Pin_3
#define ENCODER_DT_PIN       GPIO_Pin_4
#define ENCODER_SW_PIN       GPIO_Pin_5
#define ENCODER_GPIO_PORT    GPIOB

/* Most encoder modules generate two valid phase changes per detent.
 * Use one UI event per detent so the small OLED menu feels responsive. */
#define ENCODER_STEPS_PER_EVENT 2
#define ENCODER_QUEUE_LENGTH    8

#define ENCODER_CLK_READ() \
    GPIO_ReadInputDataBit(ENCODER_GPIO_PORT, ENCODER_CLK_PIN)
#define ENCODER_DT_READ() \
    GPIO_ReadInputDataBit(ENCODER_GPIO_PORT, ENCODER_DT_PIN)
#define ENCODER_SW_READ() \
    GPIO_ReadInputDataBit(ENCODER_GPIO_PORT, ENCODER_SW_PIN)

static uint8_t Encoder_LastAB;
static uint8_t Encoder_LastSW;
static int8_t Encoder_Step;
static QueueHandle_t Encoder_EventQueue;

void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB,
        ENABLE
    );

    /* PB3/PB4 are JTAG pins. Keep SWD enabled and release JTAG pins. */
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Pin =
        ENCODER_CLK_PIN | ENCODER_DT_PIN | ENCODER_SW_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ENCODER_GPIO_PORT, &GPIO_InitStructure);

    Encoder_LastAB = (uint8_t)((ENCODER_CLK_READ() << 1) |
                               ENCODER_DT_READ());
    Encoder_LastSW = ENCODER_SW_READ();
    Encoder_Step = 0;
    Encoder_EventQueue = xQueueCreate(ENCODER_QUEUE_LENGTH,
                                      sizeof(Encoder_Event));
}

static Encoder_Event Encoder_Scan(void)
{
    static const int8_t TransitionTable[16] = {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };
    uint8_t current_ab;
    uint8_t current_sw;
    uint8_t transition;

    current_ab = (uint8_t)((ENCODER_CLK_READ() << 1) |
                           ENCODER_DT_READ());
    transition = (uint8_t)((Encoder_LastAB << 2) | current_ab);
    Encoder_Step += TransitionTable[transition];
    Encoder_LastAB = current_ab;

    if (Encoder_Step >= ENCODER_STEPS_PER_EVENT)
    {
        Encoder_Step = 0;
        return ENCODER_RIGHT;
    }
    if (Encoder_Step <= -ENCODER_STEPS_PER_EVENT)
    {
        Encoder_Step = 0;
        return ENCODER_LEFT;
    }

    current_sw = ENCODER_SW_READ();
    if ((Encoder_LastSW == 1) && (current_sw == 0))
    {
        Delay_ms(10);
        current_sw = ENCODER_SW_READ();
        if (current_sw == 0)
        {
            Encoder_LastSW = 0;
            return ENCODER_PRESS;
        }
    }

    Encoder_LastSW = current_sw;
    return ENCODER_NONE;
}

void Encoder_Poll(void)
{
    Encoder_Event event = Encoder_Scan();

    if((event != ENCODER_NONE) && (Encoder_EventQueue != NULL))
    {
        if(xQueueSend(Encoder_EventQueue, &event, 0) != pdPASS)
        {
            Encoder_Event discarded_event;
            (void)xQueueReceive(Encoder_EventQueue, &discarded_event, 0);
            (void)xQueueSend(Encoder_EventQueue, &event, 0);
        }
    }
}

uint8_t Encoder_IsReady(void)
{
    return Encoder_EventQueue != NULL;
}

/* Map queued encoder actions to the existing UI input convention:
 * 1 = previous, 2 = next, 3 = confirm. */
uint8_t Encoder_GetKeyNum(void)
{
    Encoder_Event event;

    if((Encoder_EventQueue == NULL) ||
       (xQueueReceive(Encoder_EventQueue, &event, 0) != pdPASS))
    {
        return 0;
    }

    if (event == ENCODER_LEFT)
    {
        return 1;
    }
    if (event == ENCODER_RIGHT)
    {
        return 2;
    }
    if (event == ENCODER_PRESS)
    {
        return 3;
    }

    return 0;
}
