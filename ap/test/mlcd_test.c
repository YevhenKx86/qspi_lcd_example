
#include "mlcd_test.h"
#include "multi_lcd.h"
#include "benchmark_video.h"
#include "blur.h"
#include "time_labels.h"
#include "screen_images.h"
//#include "media_types.h"


//#define MLCD_TEST_RUN
#define MLCD_PALETTE_TEST_RUN

#ifdef MLCD_PALETTE_TEST_RUN
#include "palette.h" 
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define TAG "C2_0_LCD"

#define MLOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define MLOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define MLOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
#define MLOGV(...) BK_LOGV(TAG, ##__VA_ARGS__)

#define DISPLAY_COUNT   3

#define DW_SCREEN   240
#define DH_SCREEN   240

#define DW_FRAME 120
#define DH_FRAME 120

#define DH_FRAME_OFFSET (DH_SCREEN-DH_FRAME)/2
#define DW_FRAME_OFFSET (DW_SCREEN-DW_FRAME)/2

#define TEXT_X  15
#define TEXT_Y  15

#define RED_COLOR       0xF800
#define GREEN_COLOR     0x07E0
#define BLUE_COLOR      0x001F
#define WHITE_COLOR      0xFFFF
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define TIME_LBL_COUNT  4
#define TIME_LBL_PERIOD 30
TimeStamp_TypeDef lbl1;
TimeStamp_TypeDef lbl2;
TimeStamp_TypeDef lbl3;
TimeStamp_TypeDef lbl4;

TimeStamp_TypeDef * lbl_stack[TIME_LBL_COUNT] = {&lbl1, &lbl2, &lbl3, &lbl4};

//uint16_t frameTmp[DW_SCREEN*DH_SCREEN];
int animationCnt = 0;

static frame_buffer_t * disp_stack[DISPLAY_COUNT]; 

//-----------------------------------------------------------------------------
frame_buffer_t * _assign_disp_frame(multi_lcd_id_t ID){
    if(ID < DISPLAY_COUNT){
        return disp_stack[ID];
    }
    else{
        MLOGI("%s: FAIL ID %d\r\n", __func__, ID);
        return NULL;
    }
}
//-----------------------------------------------------------------------------
void lcd_qspi_display_fill_pure_color(frame_buffer_t *frame_buffer, uint16_t color)
{
    uint8_t data[2] = {0};

    data[0] = color >> 8;
    data[1] = color;

    for (int i = 0; i < frame_buffer->size; i+=2)
    {
        frame_buffer->frame[i] = data[0];
        frame_buffer->frame[i + 1] = data[1];
    }
}
//-----------------------------------------------------------------------------
void lsd_some_fill_120x120(uint16_t * pframe_buff, int AnimationCounter){
    for (int i=DH_FRAME_OFFSET; i < DH_FRAME_OFFSET + DH_FRAME; i++) {
        for (int j=DW_FRAME_OFFSET; j < DW_FRAME_OFFSET + DW_FRAME; j++) {
            pframe_buff[i*DW_SCREEN+j] = (uint16_t)((i + AnimationCounter) / 4);
        }
    }
}
//-----------------------------------------------------------------------------
void lsd_qspi_fill_image_120x120(multi_lcd_id_t ID, int x, int y){

    frame_buffer_t *df = _assign_disp_frame(ID);

    if(df == NULL) return;

    int bmpx = 0; 
    int bmpy = 0;
    uint8_t data[2]; 

    uint16_t * pDst = (uint16_t*)df->frame;    

    for (int i = y; i < y + 120; i++){
        for (int j = x; j < x + 120; j++){
            data[0] = epd_bitmap_allArray[ID][bmpy * 120 + bmpx] >> 8;
            data[1] = epd_bitmap_allArray[ID][bmpy * 120 + bmpx];
            pDst[i * DH_SCREEN + j] = *(uint16_t*)data;

            bmpx++;
        }   
        bmpy++;   
        bmpx = 0;  
    } 
    
    //MLOGI("%s: display#%d, width %d, height %d, size %d\r\n ", 
    //    __func__, ID, df->width, df->height, df->size);
}
//-----------------------------------------------------------------------------
static avdk_err_t display_frame_free_cb(void *frame)
{
    frame_buffer_display_free((frame_buffer_t *)frame);

    return AVDK_ERR_OK;
}
//-----------------------------------------------------------------------------
void _init(void){
    uint32_t frame_len = DW_SCREEN * DH_SCREEN * 2;

    for(multi_lcd_id_t i = 0; i < MULTI_LCD_ID_MAX; i++){
        disp_stack[i] = frame_buffer_display_malloc(frame_len);

        disp_stack[i]->fmt = PIXEL_FMT_RGB565;
        disp_stack[i]->width = DW_SCREEN; 
        disp_stack[i]->height = DH_SCREEN;

        if(i == MULTI_LCD_ID_1){
            lcd_qspi_display_fill_pure_color(disp_stack[i], RED_COLOR);
            lsd_qspi_fill_image_120x120(i, DW_FRAME_OFFSET, DH_FRAME_OFFSET);
            draw_text((uint16_t*)disp_stack[i]->frame, "DISP1", TEXT_X, TEXT_Y, WHITE_COLOR);
            MLOGI("%s: display#%d, %p\r\n", __func__, i, disp_stack[i]);
        }
        else if(i == MULTI_LCD_ID_2){
            lcd_qspi_display_fill_pure_color(disp_stack[i], GREEN_COLOR);
            lsd_qspi_fill_image_120x120(i, DW_FRAME_OFFSET, DH_FRAME_OFFSET);
            draw_text((uint16_t*)disp_stack[i]->frame, "DISP2", TEXT_X, TEXT_Y, WHITE_COLOR);
            MLOGI("%s: display#%d, %p\r\n", __func__, i, disp_stack[i]);
        }
        else{
            lcd_qspi_display_fill_pure_color(disp_stack[i], BLUE_COLOR);
            lsd_qspi_fill_image_120x120(i, DW_FRAME_OFFSET, DH_FRAME_OFFSET);
            draw_text((uint16_t*)disp_stack[i]->frame, "DISP3", TEXT_X, TEXT_Y, WHITE_COLOR);
            MLOGI("%s: display#%d, %p\r\n", __func__, i, disp_stack[i]);
        }

        multi_lcd_display_flush(i, disp_stack[i], display_frame_free_cb);
    }
    
    MLOGI("%s: displays inited.\r\n", __func__);
}
//-----------------------------------------------------------------------------
void blur_frame(multi_lcd_id_t ID){

    frame_buffer_t *disp_frame = _assign_disp_frame(ID);

    if(disp_frame != NULL){

        mTimeStamp_Start(lbl_stack[ID]);

        BlurFrameConfig_TypeDef bCfg;
        bCfg.pIn = epd_bitmap_allArray[ID]; //frameTmp;
        bCfg.pOut = (uint16_t*)disp_frame->frame;
        bCfg.coreW = 5;
        bCfg.h = DH_SCREEN;
        bCfg.w = DW_SCREEN;
        bCfg.h_frame = DH_FRAME;
        bCfg.w_frame = DW_FRAME;
        bCfg.h0 = (DH_SCREEN - DH_FRAME)/2;
        bCfg.w0 = (DW_SCREEN - DW_FRAME)/2;

        blur_frame_new(&bCfg);

        mTimeStamp_Stop(lbl_stack[ID]);
    } 
}
//-----------------------------------------------------------------------------
void draw_frame(multi_lcd_id_t ID){

    frame_buffer_t *disp_frame = _assign_disp_frame(ID);   

    if(disp_frame != NULL){

        /*if(lbl_stack[ID]->count % TIME_LBL_PERIOD == 0){
            mTimeStamp_StatisticsAndReset(lbl_stack[ID]);
            draw_text((uint16_t*)disp_stack[ID]->frame, lbl_stack[ID]->msg, TEXT_X, TEXT_Y+10, WHITE_COLOR);
        }*/            

        multi_lcd_display_flush(ID, disp_frame, display_frame_free_cb);
        //MLOGI("%s: display#%d\r\n", __func__, ID);
    }    
}
//-----------------------------------------------------------------------------
void reload_pure_color(void){
    uint16_t color = RED_COLOR;
    for(int i = MULTI_LCD_ID_1; i < MULTI_LCD_ID_MAX; i++){
        if(i == MULTI_LCD_ID_2){
            color = GREEN_COLOR;
        }
        else if(i == MULTI_LCD_ID_3){
            color = BLUE_COLOR;
        }

        lcd_qspi_display_fill_pure_color(disp_stack[i], color);
    }
}
//-----------------------------------------------------------------------------
void reload_images(void){
    for(int i = 0; i < DISPLAY_COUNT; i++){
        lsd_qspi_fill_image_120x120(i, DW_FRAME_OFFSET, DH_FRAME_OFFSET);
    }
    //MLOGI("%s: done\r\n", __func__);
}
//-----------------------------------------------------------------------------
#define DISP_TITLE_LENGTH 5
char disp_title[DISP_TITLE_LENGTH + 1] = {'D', 'I', 'S', 'P', 0, 0};
//-----------------------------------------------------------------------------
//-------------------- PALETTE ------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef MLCD_PALETTE_TEST_RUN

#define IMAGE_W DW_FRAME
#define IMAGE_H DH_FRAME

beken_mutex_t _lock;

static PALETTE _palette;
static uint8_t image_indx[IMAGE_W * IMAGE_H];
static TimeStamp_TypeDef palTmLbl;

//-----------------------------------------------------------------------------
void pal_load_image(const uint16_t * image, PALETTE * palette, uint8_t * image_indexed){
    //MLOGI("%s: enter\r\n", __func__);
    //rtos_lock_mutex(&_lock);
    for(int y = 0; y < IMAGE_H; y++){
        for (int x = 0; x < IMAGE_W; x++){
            image_indexed[y * IMAGE_H + x] = palette_get_color_idx(image[y * IMAGE_H + x], palette);
        }  
        MLOGI("Row %d\r\n", y);      
    }
    //rtos_unlock_mutex(&_lock);
}
//-----------------------------------------------------------------------------

uint16_t frame_tmp[DW_SCREEN * DH_SCREEN];

void pal_load_image_to_frame_buf(multi_lcd_id_t ID, uint8_t * image_indx, PALETTE * palette){    
    
    frame_buffer_t *df = _assign_disp_frame(ID);
    //uint16_t * pDst = (uint16_t*)df->frame; 
    
    //MLOGI("%s: ID %s, image_indx %p, palette %p\r\n", __func__, ID, image_indx, palette);

    uint16_t * pDst = frame_tmp;
    
    if(df != NULL){
        for(int y = 0; y < DW_FRAME; y++){
            for(int x = 0; x < DH_FRAME; x++){
                pDst[(y + DH_FRAME_OFFSET)*DH_SCREEN + x + DW_FRAME_OFFSET] = *(uint16_t*)palette[image_indx[y*DW_FRAME + x]];
            }
        }
    }
    
    //MLOGI("%s: done!\r\n", __func__);
}
//-----------------------------------------------------------------------------
void pal_init(void){

    if(!_lock){
        rtos_init_mutex(&_lock);
    }

    /*uint32_t frame_len = DW_SCREEN * DH_SCREEN * 2;

    for(multi_lcd_id_t i = 0; i < MULTI_LCD_ID_MAX; i++){

        disp_stack[i] = frame_buffer_display_malloc(frame_len);

        disp_stack[i]->fmt = PIXEL_FMT_RGB565;
        disp_stack[i]->width = DW_SCREEN; 
        disp_stack[i]->height = DH_SCREEN;

        if(i == MULTI_LCD_ID_1){
            lcd_qspi_display_fill_pure_color(disp_stack[i], RED_COLOR);
            lsd_qspi_fill_image_120x120(i, DW_FRAME_OFFSET, DH_FRAME_OFFSET);
            draw_text((uint16_t*)disp_stack[i]->frame, "DISP1", TEXT_X, TEXT_Y, WHITE_COLOR);
            MLOGI("%s: display#%d\r\n", __func__, i);
        }
        else if(i == MULTI_LCD_ID_2){
            lcd_qspi_display_fill_pure_color(disp_stack[i], GREEN_COLOR);
            lsd_qspi_fill_image_120x120(i, DW_FRAME_OFFSET, DH_FRAME_OFFSET);
            draw_text((uint16_t*)disp_stack[i]->frame, "DISP2", TEXT_X, TEXT_Y, WHITE_COLOR);
            MLOGI("%s: display#%d\r\n", __func__, i);
        }
        else{
            lcd_qspi_display_fill_pure_color(disp_stack[i], BLUE_COLOR);
            lsd_qspi_fill_image_120x120(i, DW_FRAME_OFFSET, DH_FRAME_OFFSET);
            draw_text((uint16_t*)disp_stack[i]->frame, "DISP3", TEXT_X, TEXT_Y, WHITE_COLOR);
            MLOGI("%s: display#%d\r\n", __func__, i);
        }

       multi_lcd_display_flush(i, disp_stack[i], display_frame_free_cb);
    }*/

    _init();

    palette_generate(&_palette);    

    mTimeStamp_Start(&palTmLbl);

    pal_load_image(epd_bitmap_allArray[0], &_palette, image_indx);

    mTimeStamp_Stop(&palTmLbl);
    mTimeStamp_StatisticsAndReset(&palTmLbl);

    MLOGI("%s: load image done!\r\n", __func__);     
}
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
//--------------------- PALETTE END -------------------------------------------
//-----------------------------------------------------------------------------
void mlcd_test(void){

#ifdef MLCD_TEST_RUN

    multi_lcd_init();

    multi_lcd_backlight_open(MULTI_LCD_BACKLIGHT_CTR_PIN);

    _init();    

    while(1){

        mTimeStamp_Start(&lbl4);

        for(int i = 0; i < DISPLAY_COUNT; i++){

            blur_frame(i);

            draw_frame(i);
        }

        mTimeStamp_Stop(&lbl4);

        if(lbl4.count % TIME_LBL_PERIOD == 0){   
            
            reload_pure_color();
            
            for (size_t i = 0; i < DISPLAY_COUNT; i++)
            {
                mTimeStamp_StatisticsAndReset(lbl_stack[i]);
                disp_title[DISP_TITLE_LENGTH - 1] = '0' + i; 
                draw_text((uint16_t*)disp_stack[i]->frame, disp_title, TEXT_X, TEXT_Y, WHITE_COLOR);
                draw_text((uint16_t*)disp_stack[i]->frame, lbl_stack[i]->msg, TEXT_X, TEXT_Y + 10, WHITE_COLOR);
            }            

            mTimeStamp_StatisticsAndReset(&lbl4);            

            draw_text((uint16_t*)disp_stack[0]->frame, lbl4.msg, TEXT_X, TEXT_Y+20, WHITE_COLOR);
            draw_text((uint16_t*)disp_stack[1]->frame, lbl4.msg, TEXT_X, TEXT_Y+20, WHITE_COLOR);
            draw_text((uint16_t*)disp_stack[2]->frame, lbl4.msg, TEXT_X, TEXT_Y+20, WHITE_COLOR);


            TimeStamp_TypeDef tmLbl;
            for(int i = 0; i < TIME_LBL_PERIOD; i++){
                mTimeStamp_Start(&tmLbl);
                reload_images();
                mTimeStamp_Stop(&tmLbl);
            }

            MLOGI("Copy %d bytes. timeAcc %d, cnt %d. %d copies per second\r\n", 
                sizeof(epd_bitmap_screen1)*DISPLAY_COUNT, tmLbl.timeAcc, tmLbl.count, (1000*tmLbl.count)/tmLbl.timeAcc);// v25

            mTimeStamp_StatisticsAndReset(&tmLbl);      
            
            draw_text((uint16_t*)disp_stack[0]->frame, tmLbl.msg, TEXT_X + 100, TEXT_Y+20, WHITE_COLOR);

            multi_lcd_display_flush(MULTI_LCD_ID_1, disp_stack[MULTI_LCD_ID_1], display_frame_free_cb);
            multi_lcd_display_flush(MULTI_LCD_ID_2, disp_stack[MULTI_LCD_ID_2], display_frame_free_cb);
            multi_lcd_display_flush(MULTI_LCD_ID_3, disp_stack[MULTI_LCD_ID_3], display_frame_free_cb);
        
            rtos_delay_milliseconds(1000);
        }

        
        
    }

#endif

#ifdef MLCD_PALETTE_TEST_RUN

    multi_lcd_init();

    multi_lcd_backlight_open(MULTI_LCD_BACKLIGHT_CTR_PIN);

    pal_init();    

    while(1){        

        mTimeStamp_Start(&palTmLbl);
        for(int k = 0; k < 10; k++){            
            for(int i = MULTI_LCD_ID_1; i < MULTI_LCD_ID_MAX; i++){

                pal_load_image_to_frame_buf(i, image_indx, &_palette);   

                avdk_err_t err = 

                    multi_lcd_display_flush(i, disp_stack[i], display_frame_free_cb);
                
                MLOGI("%s: display#%d, flush err %d\r\n", __func__, i, err);
            }            
        }
        mTimeStamp_Stop(&palTmLbl);

        if(palTmLbl.count % TIME_LBL_PERIOD == 0){            
            MLOGI("%s: %d frames done \r\n", __func__, palTmLbl.count*10*3);
            mTimeStamp_StatisticsAndReset(&palTmLbl);
        }

        //MLOGI("%s: draw screens done\r\n", __func__);
        //rtos_delay_milliseconds(1000);        
    }
    
#endif
}
//-----------------------------------------------------------------------------
