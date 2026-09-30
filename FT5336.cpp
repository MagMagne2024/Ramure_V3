// FT5336
// © 2020-2025 Patrick Lafarguette

#include <FT5336.h>
#include <Wire.h>

TSPoint::TSPoint() {
    x = y = z = 0;
}

TSPoint::TSPoint(int16_t x, int16_t y, int16_t z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

bool TSPoint::operator ==(TSPoint point) {
    return ((point.x == x) && (point.y == y) && (point.z == z));
}

bool TSPoint::operator !=(TSPoint point) {
    return ((point.x != x) || (point.y != y) || (point.z != z));
}

FT5336::FT5336() {
}

#define FT5336_REGISTER_CHIPID 0xA8
#define FT5336_REGISTER_STATUS 0x02
#define FT5336_REGISTER_TOUCH0 0x03

TSPoint FT5336::getPoint() {
    TSPoint point = { 0, 0, 0 };

    uint8_t count = read(FT5336_REGISTER_STATUS) & 0x0F;

    if (count <= 5) {
        for (unsigned int touch = 0; touch < count; ++touch) {
            unsigned int index = FT5336_REGISTER_TOUCH0 + touch * 6;
            point.y = (read(index++) & 0x0F) << 8;
            point.y |= read(index++);
            point.x = (read(index++) & 0x0F) << 8;
            point.x |= read(index++);
            point.z = 500;
        }
    }
    return point;
}

bool FT5336::begin() {
    _wire.setSDA(PH8);
    _wire.setSCL(PH7);
    _wire.begin();

    HAL_Delay(200);

    return read(FT5336_REGISTER_CHIPID) == 0x51;
}

uint8_t FT5336::read(uint8_t address) {
    _wire.beginTransmission(0x38);
    _wire.write(address);
    _wire.endTransmission();
    _wire.requestFrom(0x38, 1);
    return _wire.read();
}
