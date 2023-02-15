#include <stdio.h>
#include "SPI_ws2812.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//#include "GFX.h"

//TaskHandle_t LED_Handle = NULL;
/*
void LED ()
{

        rainbow_effect_right();
        //vTaskDelay(100 / portTICK_PERIOD_MS);

}
*/
void app_main(void)
{
	initSPIws2812();
    //xTaskCreate(LED, "show led", 4096, NULL, 1, NULL);
    
    while(1)
    {
        
    for (uint8_t i=0; i<60; i++)
	{
	  rainbow_effect_right();	  
	}
	for (uint8_t i=0; i<60; i++)
	{ 
	  rainbow_effect_left();  
	}
	reset_led();
	for (uint8_t i=0; i<60; i++)
	{
		rainbow_text("test",5,0);
	} 
    reset_led();
	draw_scroll_text("hello world", 0, 100, 0);
	rainbow_scroll_text("Help me please");
    }
    
    
}