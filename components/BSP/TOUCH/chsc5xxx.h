#ifndef __CHSC5XXX_H
#define __CHSC5XXX_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "touch.h"
#include "iic.h"
#include "xl9555.h"


/* 触摸屏复位 */
#define CT_RST(x)       do { x ?                               \
                            xl9555_pin_write(CTP_RST_IO, 1):   \
                            xl9555_pin_write(CTP_RST_IO, 0);   \
                        } while(0)

#define CHSC5432_ADDR                        0x2E        /* 7位地址->请看《Application Note for CTPM_CHSC5xxx》 */

/* CHSC5XXX 寄存器  */
#define CHSC5XXX_CTRL_REG                    0x2000002C  /* 触摸事件 */
#define CHSC5XXX_PID_REG                     0x20000080  /* 读取ID */

/* 函数声明 */
uint8_t chsc5xxx_init(void);
uint8_t chsc5xxx_scan(uint8_t mode);
esp_err_t chsc5xxx_wr_reg(uint32_t reg, uint8_t *buf, unsigned int len);
esp_err_t chsc5xxx_rd_reg(uint32_t reg, uint8_t *buf, unsigned int len);

#endif
