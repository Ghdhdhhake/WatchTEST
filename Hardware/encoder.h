#ifndef _ENCODER_H
#define _ENCODER_H

#include "stm32f10x.h"

typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_LEFT,
    ENCODER_RIGHT,
    ENCODER_PRESS
} Encoder_Event;

void Encoder_Init(void);
Encoder_Event Encoder_Scan(void);
uint8_t Encoder_GetKeyNum(void);

#endif
