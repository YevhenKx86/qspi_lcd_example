
#include "stdint.h"

typedef struct{
    const uint16_t * pIn;
    uint16_t * pOut;
    uint16_t w;
    uint16_t h;
    uint16_t coreW;
    uint16_t w0;
    uint16_t h0;
    uint16_t w_frame;
    uint16_t h_frame;
}BlurFrameConfig_TypeDef;

void blur_frame_new(BlurFrameConfig_TypeDef * cfg);
