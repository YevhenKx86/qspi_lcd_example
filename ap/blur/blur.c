

#include "blur.h"
//-----------------------------------------------------------------------------
/*void blur_frame_partial_old(BlurFrameConfig_TypeDef * cfg){
    uint32_t px_val = 0;
    int x = cfg->w0;
    int y = cfg->h0;
    int x_max = cfg->w0 + cfg->w_frame;
    int y_max = cfg->h0 + cfg->h_frame;

    for (x = cfg->w0; x < x_max - cfg->coreW; x++)
    {
        for (y = cfg->h0; y < y_max - cfg->coreW; y++)
        {
            px_val = 0;

            for(int cx = x; cx < x + cfg->coreW; cx++){
                for(int cy = y; cy < y + cfg->coreW; cy++){
                    px_val += cfg->pIn[cfg->w*cx + cy];
                }
            }
            cfg->pOut[(x + cfg->coreW/2)*cfg->w + (y+cfg->coreW/2)] =
                                                px_val/(cfg->coreW*cfg->coreW);
        }
    }
}*/
//-----------------------------------------------------------------------------
void blur_frame_new(BlurFrameConfig_TypeDef * cfg){

    int x_max = cfg->w0 + cfg->w_frame - cfg->coreW;
    int y_max = cfg->h0 + cfg->h_frame - cfg->coreW;
    int c_offset = cfg->coreW/2;
    int div = cfg->coreW*cfg->coreW;

    int inx = 0;
    int iny = 0;

    uint8_t data[2];

    for (int y = cfg->h0; y < y_max; y++){
        for (int x = cfg->w0; x < x_max; x++){
            
            uint32_t sum_r = 0, sum_g = 0, sum_b = 0;
            uint16_t p;

            for(int cy = 0; cy < cfg->coreW; cy++){
                for(int cx = 0; cx < cfg->coreW; cx++){   
                    p = cfg->pIn[(iny + cy)*cfg->h_frame + inx + cx];    
                     
                    sum_r += (uint32_t)((p >> 11) & 0x1F);
                    sum_g += (uint32_t)((p >> 5)  & 0x3F);
                    sum_b += (uint32_t)( p        & 0x1F);
                }                
            }

            uint16_t r = (uint16_t)(sum_r / div);
            uint16_t g = (uint16_t)(sum_g / div);
            uint16_t b = (uint16_t)(sum_b / div);

            // Change to BE for display
            uint16_t tmp = (uint16_t)((r << 11) | (g << 5) | b);
            data[0] = tmp >> 8;
            data[1] = tmp;

            cfg->pOut[(y + c_offset)*cfg->h + (x + c_offset)] = *(uint16_t*)data;
            
            inx++;
        }
        iny++;
        inx = 0;
    }
}
//-----------------------------------------------------------------------------

#define SRC_W 120
#define SRC_H 120
#define DST_W 240
#define DST_H 240

void blur_box5_rgb565(const uint16_t *src, uint16_t *dst)
{
    for (int y = 2; y <= 117; ++y) 
    {
        for (int x = 2; x <= 117; ++x) 
        {
            //Accumulate
            uint32_t sum_r = 0, sum_g = 0, sum_b = 0;
            for (int ky = -2; ky <= 2; ++ky) 
            {
                const uint16_t *row = src + (y + ky) * SRC_W + (x - 2);

                for (int kx = 0; kx < 5; ++kx) 
                {
                    uint16_t p = row[kx];
                    sum_r += (uint32_t)((p >> 11) & 0x1F);
                    sum_g += (uint32_t)((p >> 5)  & 0x3F);
                    sum_b += (uint32_t)( p        & 0x1F);
                }
            }

            //Average of 25 samples
            uint16_t r = (uint16_t)(sum_r / 25U);
            uint16_t g = (uint16_t)(sum_g / 25U);
            uint16_t b = (uint16_t)(sum_b / 25U);

            //Repack result
            dst[y * DST_W + x] = (uint16_t)((r << 11) | (g << 5) | b);
        }
    }
}
//-----------------------------------------------------------------------------