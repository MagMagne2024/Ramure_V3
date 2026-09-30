// LTDC
// © 2020-2025 Patrick Lafarguette

#include <LTDC.h>

struct Settings {
    uint16_t width;
    uint16_t height;
    uint16_t horizontalSync;
    uint16_t horizontalFrontPorch;
    uint16_t horizontalBackPorch;
    uint16_t verticalSync;
    uint16_t verticalFrontPorch;
    uint16_t verticalBackPorch;
};

#if ARDUINO_DISCO_F746NG

// ROCKTECH RK043FN48H-CT672B
// 4.3” RGB 480×272 color LCD-TFT with capacitive touch screen
// Drive IC OTA5180A
// CTP IC FT5336GQQ
Settings settings = {
    .width = 480,
    .height = 272,
    .horizontalSync = 41,
    .horizontalFrontPorch = 32,
    .horizontalBackPorch = 13,
    .verticalSync = 10,
    .verticalFrontPorch = 2,
    .verticalBackPorch = 10,
};

void DisplayController::init() {
    static RCC_PeriphCLKInitTypeDef periph_clk_init_struct;
    periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    periph_clk_init_struct.PLLSAI.PLLSAIN = 192;
    periph_clk_init_struct.PLLSAI.PLLSAIR = 5;
    periph_clk_init_struct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
    HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);

    // Enable GPIOs clock
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_ENABLE();

    // LTDC pins configuration
    GPIO_InitTypeDef gpio_init_structure;

    // GPIOE configuration
    gpio_init_structure.Pin = GPIO_PIN_4;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FAST;
    gpio_init_structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOE, &gpio_init_structure);

    // GPIOG configuration
    gpio_init_structure.Pin = GPIO_PIN_12;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Alternate = GPIO_AF9_LTDC;
    HAL_GPIO_Init(GPIOG, &gpio_init_structure);

    // GPIOI LTDC alternate configuration
    gpio_init_structure.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_13
            | GPIO_PIN_14 | GPIO_PIN_15;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOI, &gpio_init_structure);

    // GPIOJ configuration
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
            | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8
            | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14
            | GPIO_PIN_15;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

    // GPIOK configuration
    gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4
            | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOK, &gpio_init_structure);

    // Display enable
    pinMode(PI12, OUTPUT);
    digitalWrite(PI12, HIGH);

    // Back light enable
    pinMode(PK3, OUTPUT);
    digitalWrite(PK3, HIGH);
}

#endif // ARDUINO_DISCO_F746NG

#if LV_USE_GPU_STM32_DMA2D

DisplayController::DisplayController() {
}

DisplayController::~DisplayController() {
}

bool DisplayController::begin() {
    _buffer = (uint16_t *)0xC0000000;
    memset(_buffer, 0xFF, settings.width * settings.height * sizeof(uint16_t));

    // LCD clock configuration
    _handle.Init.HorizontalSync = (settings.horizontalSync - 1);
    _handle.Init.VerticalSync = (settings.verticalSync - 1);
    _handle.Init.AccumulatedHBP = (settings.horizontalSync + settings.horizontalBackPorch - 1);
    _handle.Init.AccumulatedVBP = (settings.verticalSync + settings.verticalBackPorch - 1);
    _handle.Init.AccumulatedActiveH = (settings.height + settings.verticalSync + settings.verticalBackPorch - 1);
    _handle.Init.AccumulatedActiveW = (settings.width + settings.horizontalSync + settings.horizontalBackPorch - 1);
    _handle.Init.TotalHeigh = (settings.height + settings.verticalSync + settings.verticalBackPorch + settings.verticalFrontPorch - 1);
    _handle.Init.TotalWidth = (settings.width + settings.horizontalSync + settings.horizontalBackPorch + settings.horizontalFrontPorch - 1);

    // Initialize the LCD pixel width and pixel height
    _handle.LayerCfg->ImageWidth = settings.width;
    _handle.LayerCfg->ImageHeight = settings.height;

    // Background value
    _handle.Init.Backcolor.Blue = 0;
    _handle.Init.Backcolor.Green = 0;
    _handle.Init.Backcolor.Red = 0;
    // Polarity
    _handle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    _handle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    _handle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    _handle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    _handle.Instance = LTDC;
    __HAL_RCC_LTDC_CLK_ENABLE();
    init();

    HAL_LTDC_Init(&_handle);

    // Layer initialization
    LTDC_LayerCfgTypeDef layer;
    layer.WindowX0 = 0;
    layer.WindowX1 = settings.width;
    layer.WindowY0 = 0;
    layer.WindowY1 = settings.height;
    layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layer.FBStartAdress = (uint32_t)_buffer;
    layer.Alpha = 255;
    layer.Alpha0 = 0;
    layer.Backcolor.Blue = 0;
    layer.Backcolor.Green = 0;
    layer.Backcolor.Red = 0;
    layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    layer.ImageWidth = settings.width;
    layer.ImageHeight = settings.height;
    HAL_LTDC_ConfigLayer(&_handle, &layer, 0);

    return true;
}

uint16_t DisplayController::width() {
    return settings.width;
}
uint16_t DisplayController::height() {
    return settings.height;
}

uint16_t*  DisplayController::getBuffer() {
    return _buffer;
}

#else

DisplayController::DisplayController() :
        Adafruit_GFX(settings.width, settings.height) {
}

DisplayController::~DisplayController() {
}

bool DisplayController::begin() {
    _buffer = (uint16_t*) 0xC0000000;
    memset(_buffer, 0xFF, settings.width * settings.height * sizeof(uint16_t));
//  // Buffer
//  if (_buffer) {
//      free(_buffer);
//  }
//  size_t size = _width * _height * sizeof(uint16_t);
//    _buffer = (uint16_t*)malloc(size);
//    if (_buffer) {
//      memset(_buffer, 0xFF, size);
//    } else {
//      return false;
//    }

    // LCD clock configuration
    _handle.Init.HorizontalSync = (settings.horizontalSync - 1);
    _handle.Init.VerticalSync = (settings.verticalSync - 1);
    _handle.Init.AccumulatedHBP = (settings.horizontalSync + settings.horizontalBackPorch - 1);
    _handle.Init.AccumulatedVBP = (settings.verticalSync + settings.verticalBackPorch - 1);
    _handle.Init.AccumulatedActiveH = (settings.height + settings.verticalSync + settings.verticalBackPorch - 1);
    _handle.Init.AccumulatedActiveW = (settings.width + settings.horizontalSync + settings.horizontalBackPorch - 1);
    _handle.Init.TotalHeigh = (settings.height + settings.verticalSync + settings.verticalBackPorch + settings.verticalFrontPorch - 1);
    _handle.Init.TotalWidth = (settings.width + settings.horizontalSync + settings.horizontalBackPorch + settings.horizontalFrontPorch - 1);

    // Initialize the LCD pixel width and pixel height
    _handle.LayerCfg->ImageWidth = _width;
    _handle.LayerCfg->ImageHeight = _height;

    // Background value
    _handle.Init.Backcolor.Blue = 0;
    _handle.Init.Backcolor.Green = 0;
    _handle.Init.Backcolor.Red = 0;
    // Polarity
    _handle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    _handle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    _handle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    _handle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    _handle.Instance = LTDC;
    __HAL_RCC_LTDC_CLK_ENABLE();
    init();

    HAL_LTDC_Init(&_handle);

    // Layer initialization
    LTDC_LayerCfgTypeDef layer;
    layer.WindowX0 = 0;
    layer.WindowX1 = _width;
    layer.WindowY0 = 0;
    layer.WindowY1 = _height;
    layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layer.FBStartAdress = (uint32_t) _buffer;
    layer.Alpha = 255;
    layer.Alpha0 = 0;
    layer.Backcolor.Blue = 0;
    layer.Backcolor.Green = 0;
    layer.Backcolor.Red = 0;
    layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    layer.ImageWidth = _width;
    layer.ImageHeight = _height;
    HAL_LTDC_ConfigLayer(&_handle, &layer, 0);

    return true;
}

void DisplayController::drawPixel(int16_t x, int16_t y, uint16_t color) {
//  if (x >= 0 && x < _width && y >= 0 && y < _height) {
//      _buffer[x + y * _width] = color;
//  }

    if (x >= 0 && x < _width && y >= 0 && y < _height) {
        _buffer[y * _width + x] = color;
    }
}

void DisplayController::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
//  if (x >= _width || y < 0 || y >= _height) {
//      return;
//  }
//  for (int i = max((int) x, 0); i < min(x + w, (int)_width); i++) {
//      _buffer[i + y * _width] = color;
//  }

    uint16_t *to = _buffer + y * _width + x;
    for (int16_t index = 0; index < w; ++index) {
        *to++ = color;
    }
}

void DisplayController::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
//  if (x < 0 || x >= _width || y >= _height) {
//      return;
//  }
//  for (int i = max((int) y, 0); i < min(y + h, (int)_height); i++) {
//      _buffer[x + i * _width] = color;
//  }

    uint16_t *to = _buffer + y * _width + x;
    for (int16_t index = 0; index < h; ++index) {
        *to = color;
        to += _width;
    }
}

void DisplayController::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
//  if (x >= _width || y >= _height) {
//      return;
//  }
//  for (int j = max((int) y, 0); j < min(y + h, H); j++) {
//      for (int i = max((int) x, 0); i < min(x + w, _width); i++) {
//          _buffer[i + j * _width] = color;
//      }
//  }

    uint16_t* to = _buffer + y * _width + x;
    for (int16_t index = 0; index < h; ++index) {
        for (int16_t index = 0; index < w; ++index) {
            *to++ = color;
        }
        to += _width - w;
    }
}

void DisplayController::drawBuffer(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *buffer) {
    uint16_t *from = buffer;
    uint16_t *to = _buffer + y * _width + x;
    for (int16_t index = 0; index < h; ++index) {
        memcpy(to, from, w * sizeof(uint16_t));
        to += _width;
        from += w;
    }
}

uint16_t*  DisplayController::getBuffer() {
    return _buffer;
}

#endif
