#include <stdio.h>

#include "iic.h"
#include "xl9555.h"
#include "spi.h"
#include "lcd.h"
#include "gw_log.h"

void ui_init()
{
    iic_init();
    xl9555_init();
    spi2_init();
    spilcd_init();
    spilcd_clear(WHITE);
}

void ui_task(void *pvParameter)
{
    ui_init();
    static uint8_t count = 0;
    char buf[128];
    while (1)
    {
        sprintf(buf, "ui count %d\n",count++);
        spilcd_show_string(10,10,100,50,16,buf,BLACK);
        vTaskDelay(1000);
    }
}