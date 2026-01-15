#include "dev_power.h"
#include <driver/gpio.h>
#include <driver/gpio_types.h>
#include <gpio_map.h>
#include "gpio_driver.h"


#define TAG "dev_power"
#define LOGE(...) BK_LOGE(TAG, ##__VA_ARGS__)
#define LOGW(...) BK_LOGW(TAG, ##__VA_ARGS__)
#define LOGI(...) BK_LOGI(TAG, ##__VA_ARGS__)
#define LOGD(...) BK_LOGD(TAG, ##__VA_ARGS__)
#define LOGV(...) BK_LOGV(TAG, ##__VA_ARGS__)


#define DEV_PIN_LDO33_EN (GPIO_33)

bk_err_t dev_power_ldo33_en(void)
{
    bk_err_t ret = BK_OK;
    gpio_config_t mode = {0};
    gpio_id_t gpio_id = 0;

    gpio_id = DEV_PIN_LDO33_EN;
    BK_LOG_ON_ERR(gpio_dev_unmap(gpio_id));
    mode.io_mode = GPIO_OUTPUT_ENABLE;
    mode.pull_mode = GPIO_PULL_DISABLE;
    BK_LOG_ON_ERR(bk_gpio_set_config(gpio_id, &mode));
    BK_LOG_ON_ERR(bk_gpio_set_output_high(gpio_id));

    LOGD("%s, %d, ldo33_en[%d]ok!\n", __func__, __LINE__, gpio_id);

    return ret;
}
