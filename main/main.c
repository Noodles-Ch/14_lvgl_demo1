#include "nvs_flash.h"
#include "iic.h"
#include "xl9555.h"
#include "lvgl_demo.h"

#define CONFIG_ESP_MAIN_TASK_STACK_SIZE 8192
i2c_obj_t i2c0_master;

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
void app_main(void)
{
    esp_err_t ret;
    
    ret = nvs_flash_init();             /* 初始化NVS */

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    i2c0_master = iic_init(I2C_NUM_0);  /* 初始化IIC0 */
    xl9555_init(i2c0_master);           /* IO扩展芯片初始化 */

    lvgl_demo();                        /* 运行LVGL例程 */
}
  