#ifndef __MULTI_LCD_H_
#define __MULTI_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <components/system.h>
#include <os/os.h>
#include <components/avdk_utils/avdk_error.h>
#include "frame_buffer.h"
//#include "palette.h"


#define MULTI_LCD_LDO_CTR_EN (0)
#if (MULTI_LCD_LDO_CTR_EN > 0)
    #define MULTI_LCD_LDO_CTR_PIN (GPIO_13)
#endif

#define MULTI_LCD_BACKLIGHT_CTR_EN (1)
#if (MULTI_LCD_BACKLIGHT_CTR_EN > 0)
    #define MULTI_LCD_BACKLIGHT_CTR_PIN (GPIO_6)
#endif

typedef enum __multi_lcd_id
{
    MULTI_LCD_ID_1 = 0,
    MULTI_LCD_ID_2,
    MULTI_LCD_ID_3,
    MULTI_LCD_ID_MAX
}multi_lcd_id_t;


avdk_err_t multi_lcd_init(void);
avdk_err_t multi_lcd_deinit(void);
avdk_err_t multi_lcd_display_flush(multi_lcd_id_t lcd_id, frame_buffer_t *frame, bk_err_t (*free_t)(void *args));
avdk_err_t multi_lcd_backlight_open(uint8_t bl_io);
avdk_err_t multi_lcd_backlight_close(uint8_t bl_io);


#ifdef __cplusplus
}
#endif

#endif /*__MULTI_LCD_H_*/
