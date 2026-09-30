// LTDC
// © 2020-2025 Patrick Lafarguette

#ifndef LTDC_H_
#define LTDC_H_

#if LV_USE_GPU_STM32_DMA2D

#include <Arduino.h>

class DisplayController {
public:
	DisplayController();
	virtual ~DisplayController();
	void init();

	bool begin();

	uint16_t width();
	uint16_t height();

	uint16_t* getBuffer();

private:
	uint16_t* _buffer = NULL;

	LTDC_HandleTypeDef _handle;
};

#else

#include <Adafruit_GFX.h>

class DisplayController: public Adafruit_GFX {
public:
    DisplayController();
    virtual ~DisplayController();
    void init();

    bool begin();

    void drawPixel(int16_t x, int16_t y, uint16_t color);

    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    void drawBuffer(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t *buffer);

    uint16_t* getBuffer();

private:
    uint16_t *_buffer = NULL;

    LTDC_HandleTypeDef _handle;
};

#endif

#endif /* LTDC_H_ */
