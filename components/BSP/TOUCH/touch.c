#include "touch.h"


_m_tp_dev tp_dev =
{{
    tp_init,
    0,
    0,
    0,
    0,
    0,
}};

/**
 * @brief       触摸屏初始化
 * @param       无
 * @retval      0,触摸屏初始化成功
 *              1,触摸屏有问题
 */
uint8_t tp_init(void)
{
    tp_dev.touchtype = 0;                       /* 默认设置(电阻屏 & 竖屏) */
    tp_dev.touchtype |= lcd_dev.dir & 0X01;     /* 根据LCD判定是横屏还是竖屏 */

    chsc5xxx_init();
    tp_dev.scan = chsc5xxx_scan;                /* 扫描函数指向CHSC5xxx触摸屏扫描 */
    tp_dev.touchtype |= 0X80;
    return 0;
}
