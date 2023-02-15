#ifndef MAIN_SPI_WS2812_H_
#define MAIN_SPI_WS2812_H_
#include <stdio.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define DISPLAY_HEIGHT 8
#define DISPLAY_WIDTH 32

#define PI 3.14159265

typedef struct CRGB {
	union {
		struct {
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t g;
                uint8_t green;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
		uint8_t raw[3];
		uint32_t num;
	};
}CRGB;


void initSPIws2812();
void fillCol(uint32_t col);
void fillBuffer(uint32_t* bufLed, int Count);
void led_strip_update();
void draw_pixel(uint16_t x,uint16_t y,int r, int g, int b);
void reset_led();

void draw_char(char Character, uint16_t X, uint16_t Y, uint16_t r, uint16_t g, uint16_t b);
void draw_text(const char* Text, uint16_t X, uint16_t Y, uint16_t r, uint16_t g, uint16_t b);
void draw_scroll_text(const char* Text, uint16_t r, uint16_t g, uint16_t b);
void test ();
uint8_t rainbow_effect_right();
uint8_t rainbow_effect_left();
uint8_t rainbow_text(const char* Text, uint16_t x, uint16_t y);
void rainbow_scroll_text(const char* Text);


#endif /* MAIN_SPI_WS2812_H_ */
