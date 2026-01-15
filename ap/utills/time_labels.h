
#include "stdint.h"

#define MSG_LENGTH 5

typedef struct{
    uint32_t sTime;
    uint32_t timeAcc;
    uint32_t count;
    char msg[MSG_LENGTH];
}TimeStamp_TypeDef;

void mTimeStamp_Start(TimeStamp_TypeDef * stamp);
void mTimeStamp_Stop(TimeStamp_TypeDef * stamp);
void mTimeStamp_StatisticsAndReset(TimeStamp_TypeDef * stamp);
