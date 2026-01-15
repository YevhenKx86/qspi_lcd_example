
#include "palette.h"
#include <os/os.h>

#define TAG "PAL656"

#define PLOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
//-----------------------------------------------------------------------------
int palette_get_distance_RGB565(uint16_t color1, uint16_t color2){
    
    uint16_t r1 = (uint16_t)((color1 >> 11) & 0x1F); 
    uint16_t g1 = (uint16_t)((color1 >> 5)  & 0x3F);         
    uint16_t b1 = (uint16_t)( color1 & 0x1F);        

    uint16_t r2 = (uint16_t)((color2 >> 11) & 0x1F);
    uint16_t g2 = (uint16_t)((color2 >> 5)  & 0x3F);
    uint16_t b2 = (uint16_t)( color2 & 0x1F);

    int r = (int)(r1-r2); 
    int g = (int)(g1-g2); 
    int b = (int)(b1-b2);

    return (r*r + g*g + b*b); 
}
//-----------------------------------------------------------------------------
void palette_generate(PALETTE * palette){
    // Reset memory
    for(int i = 0; i < PALETTE_SIZE; i++){
        *palette[i] = 0;
    }
    // R & B
    for(int i = 0; i < 32; i++){    
        *palette[i] = i;
        *palette[i + 32 + 64] = i << 11;
    }
    // G
    for(int i = 0; i < 64; i++){    
        *palette[i + 32] = i << 5;
    }

    for(int i = 0; i < 32; i++){    
        *palette[i + 128] = ( 16 << 5) | (i);
    }

    for(int i = 0; i < 32; i++){    
        *palette[i + 128 + 32] = ( 16 << 5) | (i << 11);
    }

    for(int i = 0; i < 32; i++){    
        *palette[i + 128 + 64] = ( 48 << 5) | (i);
    }

    for(int i = 0; i < 32; i++){    
        *palette[i + 128 + 64 + 32] = ( 48 << 5) | (i << 11);
    }

    *palette[0] = 0xFFFF;
    // Log
    /*for(int i = 0; i < PALETTE_SIZE; i++){
        PLOGI("[%d][0x%04X]\r\n", i, *palette[i]);
    }*/
}
//-----------------------------------------------------------------------------
uint8_t palette_get_color_idx(uint16_t color, PALETTE * palette){
    uint8_t idx = 0;
    int dist_min = 32*32 + 64*64 + 32*32;
    int dist = 0;
    //--
    /*uint16_t r1;
    uint16_t g1;        
    uint16_t b1;       

    uint16_t r2;
    uint16_t g2;
    uint16_t b2;*/
    //--
    for(int i = 0; i < PALETTE_SIZE; i++){
        dist = palette_get_distance_RGB565(color, *palette[i]);
        /*r1 = ((color >> 11) & 0x1F); 
        g1 = ((color >> 5)  & 0x3F);         
        b1 = (color & 0x1F);        

        r2 = ((*palette[i] >> 11) & 0x1F);
        g2 = ((*palette[i] >> 5)  & 0x3F);
        b2 = (*palette[i] & 0x1F);

        dist = (r1-r2)*(r1-r2) + (g1-g2)*(g1-g2) + (b1-b2)*(b1-b2);*/

        if(dist < dist_min){
            dist_min = dist;
            idx = i;
        }
    }

    return idx;
}
//-----------------------------------------------------------------------------