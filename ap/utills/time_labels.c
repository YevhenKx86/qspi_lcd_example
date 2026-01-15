
#include "time_labels.h"
#include <os/os.h>

//-----------------------------------------------------------------------------
void mTimeStamp_Start(TimeStamp_TypeDef * stamp){   
    if(stamp->count == 0){
        for (int i = 0; i < MSG_LENGTH; i++){
            stamp->msg[i] = 0;
        }        
        stamp->timeAcc = 0;
    }     
    stamp->sTime = rtos_get_time();    
}
//-----------------------------------------------------------------------------
void mTimeStamp_Stop(TimeStamp_TypeDef * stamp){
    stamp->timeAcc += rtos_get_time() - stamp->sTime;
    stamp->sTime = rtos_get_time();
    stamp->count++;
}
//-----------------------------------------------------------------------------
void mTimeStamp_StatisticsAndReset(TimeStamp_TypeDef * stamp){
    if(stamp->timeAcc != 0){
        uint32_t ms;
        ms = (1000*stamp->count)/stamp->timeAcc;
        stamp->msg[3] = '0' + (ms%10);
        stamp->msg[2] = '0' + ((ms/10)%10);
        stamp->msg[1] = '0' + ((ms/100)%10);
        stamp->msg[0] = '0' + ((ms/1000)%10);
        stamp->msg[4] = 0;

        //LOGI("TS: %d\r\n", ms);
        BK_LOGI(NULL,"TS(%p): timeAcc %d; cnt %d; times per second %d\r\n", 
                                    stamp, stamp->timeAcc, stamp->count, ms);
    }
    stamp->count = 0;
}
//-----------------------------------------------------------------------------