// FT5336
// © 2020-2025 Patrick Lafarguette

#ifndef FT5336_H_
#define FT5336_H_

// FT5336
// http://www.mouser.com/ds/2/291/NHD-4.3-480272EF-ATXL-CTP-28216.pdf
// https://www.newhavendisplay.com/specs/NHD-4.3-480272EF-ATXL-CTP.pdf
// https://github.com/STMicroelectronics/STM32CubeF7/tree/master/Drivers/BSP/Components/ft5336

#include <Wire.h>

class TSPoint {
public:
    TSPoint();
    TSPoint(int16_t x, int16_t y, int16_t z);

    bool operator ==(TSPoint);
    bool operator !=(TSPoint);

    int16_t x, y, z;
};

class FT5336 {
public:
    FT5336();

    bool begin();

    TSPoint getPoint();

private:
    TwoWire _wire;
    uint8_t read(uint8_t address);
};

#endif /* FT5336_H_ */
