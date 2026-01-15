#include "multi_lcd.h"
#include "components/bk_display.h"
#include "gpio_driver.h"
#include <driver/gpio.h>
#include "lcd_panel_devices.h"
#include "driver/pwr_clk.h"
#if (CONFIG_LCD_QSPI_SW_CS > 0)
#include "bk_display_ctlr.h"
#endif


#define TAG "multi_lcd"
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
#define LOGV(...) BK_LOGV(TAG, ##__VA_ARGS__)


#define MULTI_LCD_DEVICE    (&lcd_device_jd9853a)
#define MULTI_LCD_QSPI_ID   (QSPI_ID_0)
#define MULTI_LCD_RESET_PIN (GPIO_49)


typedef struct __multi_lcd_info
{
    beken_mutex_t lock;
    bk_display_ctlr_handle_t lcd_display_handle;
}multi_lcd_info_info_t;


static multi_lcd_info_info_t g_multi_lcd_info = {0};

static bk_display_qspi_ctlr_config_t g_qspi_ctlr_config = {
    .lcd_device = MULTI_LCD_DEVICE,
    .qspi_id = MULTI_LCD_QSPI_ID,
    .reset_pin = MULTI_LCD_RESET_PIN,
    .te_pin = 0,
};

#if (CONFIG_LCD_QSPI_SW_CS > 0)
static gpio_id_t g_qspi_cs_pin[CONFIG_LCD_QSPI_SW_CS_NUM] = {GPIO_23, GPIO_51, GPIO_29};

#if (CONFIG_LCD_QSPI_TE > 0)
static gpio_id_t g_qspi_te_pin[CONFIG_LCD_QSPI_TE_NUM] = {GPIO_34, GPIO_52, GPIO_48};
#endif
#endif


#if (MULTI_LCD_BACKLIGHT_CTR_EN > 0)
static void lcd_backlight_open(uint8_t bl_io)
{
    gpio_dev_unmap(bl_io);
    BK_LOG_ON_ERR(bk_gpio_enable_output(bl_io));
    BK_LOG_ON_ERR(bk_gpio_pull_up(bl_io));
    bk_gpio_set_output_high(bl_io);
}

static void lcd_backlight_close(uint8_t bl_io)
{
    BK_LOG_ON_ERR(bk_gpio_pull_down(bl_io));
    bk_gpio_set_output_low(bl_io);
}
#endif

avdk_err_t multi_lcd_lock(void)
{
    avdk_err_t ret = AVDK_ERR_INVAL;
    
    if (!g_multi_lcd_info.lock)
    {
        LOGE("%s, %d, lock not init!\n", __func__, __LINE__);
        return ret;
    }

    ret = rtos_lock_mutex(&g_multi_lcd_info.lock);
    if (ret)
    {
        LOGE("%s, %d, rtos_lock_mutex fail[%d]!\n", __func__, __LINE__, ret);
    }

    return ret;
}

avdk_err_t multi_lcd_unlock(void)
{
    avdk_err_t ret = AVDK_ERR_INVAL;
    
    if (!g_multi_lcd_info.lock)
    {
        LOGE("%s, %d, lock not init!\n", __func__, __LINE__);
        return ret;
    }

    ret = rtos_unlock_mutex(&g_multi_lcd_info.lock);
    if (ret)
    {
        LOGE("%s, %d, rtos_unlock_mutex fail[%d]!\n", __func__, __LINE__, ret);
    }

    return ret;
}

static avdk_err_t multi_lcd_common_qspi_lcd_init(void)
{
    avdk_err_t ret = AVDK_ERR_OK;

    if (g_multi_lcd_info.lcd_display_handle)
    {
        LOGW("%s, %d, already init!\n", __func__, __LINE__);
        return ret;
    }

#if (MULTI_LCD_LDO_CTR_EN > 0)
    bk_pm_module_vote_ctrl_external_ldo(GPIO_CTRL_LDO_MODULE_LCD, MULTI_LCD_LDO_CTR_PIN, GPIO_OUTPUT_STATE_HIGH);
#endif

    ret = bk_display_qspi_new(&g_multi_lcd_info.lcd_display_handle, &g_qspi_ctlr_config);
    if (ret)
    {
        LOGE("%s, %d, bk_display_qspi_new fail[%d]!\n", __func__, __LINE__, ret);
        return ret;
    }

#if (CONFIG_LCD_QSPI_SW_CS > 0)
    bool cs_mode = true;
    bk_display_ioctl(g_multi_lcd_info.lcd_display_handle, DISPLAY_QSPI_SET_CS_MODE, (void *)(&cs_mode));

    uint32_t cs_init_cmd = DISPLAY_QSPI_INIT_CS_GROUP_PIN_1;
    gpio_id_t cs_gpio_id;
    for (int32_t index=0; index<sizeof(g_qspi_cs_pin)/sizeof(gpio_id_t); ++index, ++cs_init_cmd)
    {
        cs_gpio_id = g_qspi_cs_pin[index];
        bk_display_ioctl(g_multi_lcd_info.lcd_display_handle, cs_init_cmd, (void *)(&cs_gpio_id));
    }

#if (CONFIG_LCD_QSPI_TE > 0)
    uint32_t te_init_cmd = DISPLAY_QSPI_INIT_TE_GROUP_PIN_1;
    gpio_id_t te_gpio_id;

    for (int32_t index=0; index<sizeof(g_qspi_te_pin)/sizeof(gpio_id_t); ++index, ++te_init_cmd)
    {
        te_gpio_id = g_qspi_te_pin[index];
        bk_display_ioctl(g_multi_lcd_info.lcd_display_handle, te_init_cmd, (void *)(&te_gpio_id));
    }
#endif
#endif

    ret = bk_display_open(g_multi_lcd_info.lcd_display_handle);
    if (ret)
    {
        LOGE("%s, %d, bk_display_open fail[%d]!\n", __func__, __LINE__, ret);
        return ret;
    }

#if (CONFIG_IMAGE_PLAY_BACKLIGHT_INDEPENDENT_CTR > 0)

#else
#if (MULTI_LCD_BACKLIGHT_CTR_EN > 0)
    lcd_backlight_open(MULTI_LCD_BACKLIGHT_CTR_PIN);
#endif
#endif

    return ret;
}

static avdk_err_t multi_lcd_common_qspi_lcd_deinit(void)
{
    avdk_err_t ret = AVDK_ERR_OK;

    if (!g_multi_lcd_info.lcd_display_handle)
    {
        LOGW("%s, %d, already deinit!\n", __func__, __LINE__);
        return ret;
    }

    ret = bk_display_close(g_multi_lcd_info.lcd_display_handle);
    if (ret)
    {
        LOGE("%s, %d, bk_display_close fail[%d]!\n", __func__, __LINE__, ret);
        return ret;
    }

#if (MULTI_LCD_BACKLIGHT_CTR_EN > 0)
    lcd_backlight_close(MULTI_LCD_BACKLIGHT_CTR_PIN);
#endif

    ret = bk_display_delete(g_multi_lcd_info.lcd_display_handle);
    if (ret)
    {
        LOGE("%s, %d, bk_display_delete fail[%d]!\n", __func__, __LINE__, ret);
        return ret;
    }
    g_multi_lcd_info.lcd_display_handle = NULL;

#if (MULTI_LCD_LDO_CTR_EN > 0)
    bk_pm_module_vote_ctrl_external_ldo(GPIO_CTRL_LDO_MODULE_LCD, MULTI_LCD_LDO_CTR_PIN, GPIO_OUTPUT_STATE_LOW);
#endif

    return ret;
}

avdk_err_t multi_lcd_init(void)
{
    avdk_err_t ret = AVDK_ERR_OK;

    if (g_multi_lcd_info.lock)
    {
        LOGW("%s, %d, already init!\n", __func__, __LINE__);
        return ret;
    }

    // lock.
    ret = rtos_init_mutex(&g_multi_lcd_info.lock);
    if (ret)
    {
        LOGE("%s, %d, mutex init fail[%d]!\n", __func__, __LINE__, ret);
        return ret;
    }

    multi_lcd_lock();
    ret = multi_lcd_common_qspi_lcd_init();
    if (ret)
    {
        LOGE("%s, %d, mutex multi_lcd_common_qspi_lcd_init fail[%d]!\n", __func__, __LINE__, ret);
        multi_lcd_unlock();
        return ret;
    }
    multi_lcd_unlock();

    LOGD("%s, %d, ok!\n", __func__, __LINE__);

    return ret;
}

avdk_err_t multi_lcd_deinit(void)
{
    avdk_err_t ret = AVDK_ERR_OK;

    if (!g_multi_lcd_info.lock)
    {
        LOGW("%s, %d, already deinit!\n", __func__, __LINE__);
        return ret;
    }

    multi_lcd_lock();
    ret = multi_lcd_common_qspi_lcd_deinit();
    if (ret)
    {
        LOGE("%s, %d, mutex multi_lcd_common_qspi_lcd_deinit fail[%d]!\n", __func__, __LINE__, ret);
        multi_lcd_unlock();
        return ret;
    }
    multi_lcd_unlock();

    if (g_multi_lcd_info.lock)
    {
        ret = rtos_deinit_mutex(&g_multi_lcd_info.lock);
        if (ret)
        {
            LOGE("%s, %d, rtos_deinit_mutex fail[%d]!\n", __func__, __LINE__, ret);
        }
        g_multi_lcd_info.lock = NULL;
    }

    LOGD("%s, %d, ok!\n", __func__, __LINE__);

    return ret;
}

avdk_err_t multi_lcd_display_flush(multi_lcd_id_t lcd_id, frame_buffer_t *frame, bk_err_t (*free_t)(void *args))
{
    avdk_err_t ret = AVDK_ERR_INVAL;

    if (lcd_id >= MULTI_LCD_ID_MAX)
    {
        LOGE("%s, %d, sn is out of range!\n", __func__, __LINE__);
        return ret;
    }

    if (frame == NULL)
    {
        LOGE("%s, %d, pointer to frame is null!\n", __func__, __LINE__);
        return ret;
    }

    if (free_t == NULL)
    {
        LOGE("%s, %d, pointer to free_t is null!\n", __func__, __LINE__);
        return ret;
    }

    if (g_multi_lcd_info.lcd_display_handle == NULL)
    {
        LOGE("%s, %d, pointer to free_t is null!\n", __func__, __LINE__);
        return ret;
    }

#if (CONFIG_LCD_QSPI_SW_CS > 0)
    bool cs_mode = false;
    bk_display_ioctl(g_multi_lcd_info.lcd_display_handle, DISPLAY_QSPI_SET_CS_MODE, (void *)(&cs_mode));

    uint32_t cs_cmd = DISPLAY_QSPI_SET_CS_PIN;
    uint32_t cs_id = lcd_id;
    bk_display_ioctl(g_multi_lcd_info.lcd_display_handle, cs_cmd, (void *)(&cs_id));
#endif

    ret = bk_display_flush(g_multi_lcd_info.lcd_display_handle, frame, free_t);
    if (ret)
    {
        LOGE("%s, %d, bk_display_flush fail[%d]!\n", __func__, __LINE__, ret);
        return ret;
    }

    return ret;
}

avdk_err_t multi_lcd_backlight_open(uint8_t bl_io)
{
#if (MULTI_LCD_BACKLIGHT_CTR_EN > 0)
    multi_lcd_lock();

    lcd_backlight_open(bl_io);

    multi_lcd_unlock();
#endif

    return AVDK_ERR_OK;
}

avdk_err_t multi_lcd_backlight_close(uint8_t bl_io)
{
#if (MULTI_LCD_BACKLIGHT_CTR_EN > 0)
    multi_lcd_lock();
    
    lcd_backlight_close(bl_io);

    multi_lcd_unlock();
#endif

    return AVDK_ERR_OK;
}
