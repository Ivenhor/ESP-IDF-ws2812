#include "SPI_ws2812.h"
#include "5x5_font.h"
#define LED_MAX_NBER_LEDS 256*2
#define MAX_LED 256
#define LED_DMA_BUFFER_SIZE ((LED_MAX_NBER_LEDS * 16 * (24/4)))+1
#define LED_PIN GPIO_NUM_19
typedef struct {
	spi_host_device_t host;
	spi_device_handle_t spi;
	int dma_chan;
	spi_device_interface_config_t devcfg;
	spi_bus_config_t buscfg;
} SPI_settings_t;

uint16_t* ledDMAbuffer;

uint32_t table[LED_MAX_NBER_LEDS];


CRGB leds[MAX_LED]; 
//-----------------------------------------------------
static SPI_settings_t SPI_settings = {
		.host = HSPI_HOST,
		.dma_chan = 2,
		.buscfg = {
				.miso_io_num = -1,
				.mosi_io_num = LED_PIN,
				.sclk_io_num = -1,
				.quadwp_io_num = -1,
				.quadhd_io_num = -1,
				.max_transfer_sz = LED_DMA_BUFFER_SIZE
		},
		.devcfg = { .clock_speed_hz = 3.2 * 1000 * 1000, //Clock out at 3.2 MHz
				.mode = 0, //SPI mode 0
				.spics_io_num = -1, //CS pin
				.queue_size = 1, //Not sure if needed
				.command_bits = 0,
				.address_bits = 0
		}
};

//-----------------------------------------------------
void initSPIws2812()
{
	esp_err_t err;

	err = spi_bus_initialize(SPI_settings.host, &SPI_settings.buscfg, SPI_settings.dma_chan);
	ESP_ERROR_CHECK(err);

	//Attach the Accel to the SPI bus
	err = spi_bus_add_device(SPI_settings.host, &SPI_settings.devcfg, &SPI_settings.spi);
	ESP_ERROR_CHECK(err);

	ledDMAbuffer = heap_caps_malloc(LED_DMA_BUFFER_SIZE, MALLOC_CAP_DMA); // Critical to be DMA memory.
}
//-----------------------------------------------------
void fillCol(uint32_t col)
{
	for(int i=0; i < LED_MAX_NBER_LEDS;i++)
	{
		table[i] = col;
		//table[i+1] = col;
	}
//	for(int i =0 ; i < 8 ;i++)
//		table[i] = 0x00ff00;
}
//-----------------------------------------------------
void fillBuffer(uint32_t* bufLed, int Count)
{
	for(int i =0 ; i < Count ;i++)
	{
		table[i] = bufLed[i];
		//table[i+1] = bufLed[i];
	}
}
//-----------------------------------------------------
int pixel_solver(uint16_t x, uint16_t y)
{
	if (x%2==0)
	{
		return (x*8)+y;
	}
	return(x*8)+(7-y);
}
//-----------------------------------------------------
void draw_pixel(uint16_t x,uint16_t y,int r, int g, int b)
{
    CRGB i ={.r=r,.g=g,.b=b};
    leds[pixel_solver(x,y)]=i;
	fillBuffer((uint32_t*)&leds,MAX_LED);

}
//-----------------------------------------------------
void reset_led()
{
    for(int i = 0 ; i < 256 ; i++)
	{
		CRGB c = {.r=0,.g=0,.b=0};
		leds[i] = c;
        
	}
    fillBuffer((uint32_t*)&leds,MAX_LED);
}
//-----------------------------------------------------
void led_strip_update()
{
	uint16_t LedBitPattern[16] = {
	0x8888,
	0x8C88,
	0xC888,
	0xCC88,
	0x888C,
	0x8C8C,
	0xC88C,
	0xCC8C,
	0x88C8,
	0x8CC8,
	0xC8C8,
	0xCCC8,
	0x88CC,
	0x8CCC,
	0xC8CC,
	0xCCCC
	};
	uint32_t i;
	esp_err_t err;

	memset(ledDMAbuffer, 0, LED_DMA_BUFFER_SIZE);
	int n = 0;
	for (i = 0; i < LED_MAX_NBER_LEDS; i++) {
		uint32_t temp = table[i];// Data you want to write to each LEDs, I'm my case it's 95 RGB x 3 color

		//R
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp >>12)];
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp)>>8];

		//G
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp >>4)];
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp)];

		//B
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp >>20)];
		ledDMAbuffer[n++] = LedBitPattern[0x0f & (temp)>>16];

	}

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = LED_DMA_BUFFER_SIZE * 8; //length is in bits
	t.tx_buffer = ledDMAbuffer;

	err = spi_device_transmit(SPI_settings.spi, &t);
	ESP_ERROR_CHECK(err);
}


//-----------------------------------------------------

uint16_t effStep = 0;
//uint16_t shift=0;
//--------------------------------------------------
void draw_char(char Character, uint16_t x, uint16_t y, uint16_t r, uint16_t g, uint16_t b)
{
		uint8_t 	function_char;
		uint8_t 	i,j;

		function_char = Character;

    if (function_char < ' ') {
        Character = 0;
    } else {
        function_char -= 32;
		}

		char temp[CHAR_WIDTH];
		for(uint8_t k = 0; k<CHAR_WIDTH; k++)
		{
		temp[k] = font[function_char][k];
		}



    // Draw pixels

    for (j=0; j<CHAR_WIDTH; j++) {
        for (i=0; i<CHAR_HEIGHT; i++) {
            if (temp[j] & (1<<i)) {

              draw_pixel(x+j, y+i, r, g, b);

            }
        }
    }

    //WS2812_Send();

}
//--------------------------------------------------
void draw_text(const char* Text, uint16_t x, uint16_t y, uint16_t r, uint16_t g, uint16_t b)
{
    while (*Text) {
    	draw_char(*Text++, x, y, r, g, b);
        x += CHAR_WIDTH;
    }
    led_strip_update();
}
//--------------------------------------------------
void erase_letter(const char* Text, uint16_t shift, uint16_t x, uint16_t r, uint16_t g, uint16_t b)
{
	Text=Text+shift;
	while (*Text)
	{
		draw_char(*Text++, x, 0, r, g, b);
		x += CHAR_WIDTH;
	}
}
//--------------------------------------------------
uint16_t text_size(const char* Text)
{
    size_t Size = strlen(Text);
    return Size;

}
//--------------------------------------------------
void draw_scroll_text(const char* Text, uint16_t r, uint16_t g, uint16_t b)
{
    uint16_t shift=0;
	for (uint16_t x = DISPLAY_WIDTH; x!=0; x--)
	{
		if(x==1)
		{
			shift++;
			x=CHAR_WIDTH;
		}
		erase_letter(Text, shift, x, r, g, b);
		if(shift>=text_size(Text))break;
		//vTaskDelay(150 / portTICK_PERIOD_MS);
		led_strip_update();
		reset_led();

	}

}
//--------------------------------------------------

int random_number(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}
//--------------------------------------------------
void test ()
{
	for (uint8_t i=0; i<DISPLAY_HEIGHT; i++)
		  {
			for(uint16_t j=0, x=1;j<DISPLAY_WIDTH||x<DISPLAY_WIDTH;j++,x++)
			{
				draw_pixel(j, i, random_number(0,255), random_number(0,255), random_number(0,255));
				led_strip_update();
			}
		  }
}
//--------------------------------------------------
uint8_t rainbow_effect_right() {

  float factor1, factor2;
  uint16_t ind;
  for(uint16_t j=0;j<DISPLAY_WIDTH;j++) {
    ind = 14 - (int16_t)(effStep - j * 1.75) % 14;
    switch((int)((ind % 14) / 4.666666666666667)) {
      case 0: factor1 = 1.0 - ((float)(ind % 14 - 0 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 0) % 14) / 4.666666666666667;
              for(uint8_t i=0;i<DISPLAY_HEIGHT;i++)
              {
            	  draw_pixel(j, i, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
              }
              break;

      case 1: factor1 = 1.0 - ((float)(ind % 14 - 1 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 4.666666666666667) % 14) / 4.666666666666667;
              for(uint8_t i=0;i<DISPLAY_HEIGHT;i++)
              {
            	  draw_pixel(j, i, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
              }
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 14 - 2 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 9.333333333333334) % 14) / 4.666666666666667;
              for(uint8_t i=0;i<DISPLAY_HEIGHT;i++)
              {
            	  draw_pixel(j, i, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
              }
              break;

    }
  }
  led_strip_update();
  if(effStep >= 14) {effStep = 0; return 0x03; }
  else effStep++;
  return 0x01;
}
//--------------------------------------------------
uint8_t rainbow_effect_left()
{
  float factor1, factor2;
  uint16_t ind;
  for(uint16_t j=0;j<DISPLAY_WIDTH;j++) {
    ind = effStep + j * 1.625;
    switch((int)((ind % 13) / 4.333333333333333)) {
      case 0: factor1 = 1.0 - ((float)(ind % 13 - 0 * 4.333333333333333) / 4.333333333333333);
              factor2 = (float)((int)(ind - 0) % 13) / 4.333333333333333;
              for(uint8_t i=0;i<DISPLAY_HEIGHT;i++)
              {
            	  draw_pixel(j, i, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
              }
              break;
      case 1: factor1 = 1.0 - ((float)(ind % 13 - 1 * 4.333333333333333) / 4.333333333333333);
              factor2 = (float)((int)(ind - 4.333333333333333) % 13) / 4.333333333333333;
              for(uint8_t i=0;i<DISPLAY_HEIGHT;i++)
              {
              	draw_pixel(j, i, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
              }
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 13 - 2 * 4.333333333333333) / 4.333333333333333);
              factor2 = (float)((int)(ind - 8.666666666666666) % 13) / 4.333333333333333;
              for(uint8_t i=0;i<DISPLAY_HEIGHT;i++)
              {
            	  draw_pixel(j, i, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
              }
              break;
    }
  }
  led_strip_update();
  if(effStep >= 13) {effStep=0; return 0x03; }
  else effStep++;
  return 0x01;
}
//--------------------------------------------------
uint8_t rainbow_text(const char* Text, uint16_t x, uint16_t y)
{
  float factor1, factor2;
  uint16_t ind;
  while (*Text) {
    ind = 14 - (int16_t)(effStep - x * 1.75) % 14;
    switch((int)((ind % 14) / 4.666666666666667)) {
      case 0: factor1 = 1.0 - ((float)(ind % 14 - 0 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 0) % 14) / 4.666666666666667;
           	  draw_char(*Text++, x, y, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
           	  x += CHAR_WIDTH;
              break;
      case 1: factor1 = 1.0 - ((float)(ind % 14 - 1 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 4.666666666666667) % 14) / 4.666666666666667;
           	  draw_char(*Text++, x, y, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
           	  x += CHAR_WIDTH;
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 14 - 2 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 9.333333333333334) % 14) / 4.666666666666667;
           	  draw_char(*Text++, x, y, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
           	  x += CHAR_WIDTH;
              break;

    }
  }
  led_strip_update();
  if(effStep >= 14) {effStep = 0; return 0x03; }
  else effStep++;
  return 0x01;
}
//--------------------------------------------------
uint8_t rainbow_erase_letter(const char* Text, uint16_t shift, uint16_t x)
{
  float factor1, factor2;
  uint16_t ind;
  Text=Text+shift;
  while (*Text) {
    ind = 14 - (int16_t)(effStep - x * 1.75) % 14;
    switch((int)((ind % 14) / 4.666666666666667)) {
      case 0: factor1 = 1.0 - ((float)(ind % 14 - 0 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 0) % 14) / 4.666666666666667;
           	  draw_char(*Text++, x, 0, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2);
           	  x += CHAR_WIDTH;
              break;
      case 1: factor1 = 1.0 - ((float)(ind % 14 - 1 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 4.666666666666667) % 14) / 4.666666666666667;
           	  draw_char(*Text++, x, 0, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2, 0 * factor1 + 255 * factor2);
           	  x += CHAR_WIDTH;
              break;
      case 2: factor1 = 1.0 - ((float)(ind % 14 - 2 * 4.666666666666667) / 4.666666666666667);
              factor2 = (float)((int)(ind - 9.333333333333334) % 14) / 4.666666666666667;
           	  draw_char(*Text++, x, 0, 0 * factor1 + 255 * factor2, 0 * factor1 + 0 * factor2, 255 * factor1 + 0 * factor2);
           	  x += CHAR_WIDTH;
              break;

    }
  }
  led_strip_update();
  if(effStep >= 14) {effStep = 0; return 0x03; }
  else effStep++;
  return 0x01;
}
//--------------------------------------------------
void rainbow_scroll_text(const char* Text)
{
    uint16_t shift =0;
	for (uint16_t x = DISPLAY_WIDTH; x!=0; x--)
	{
		if(x==1)
		{
			shift++;
			x=CHAR_WIDTH;
		}
		rainbow_erase_letter(Text, shift, x);
		if(shift>=text_size(Text))break;
		vTaskDelay(100 / portTICK_PERIOD_MS);
		led_strip_update();
        //fillCol(0);
		reset_led();

	}
}
