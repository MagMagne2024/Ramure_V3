// SDRAM
// © 2020-2025 Patrick Lafarguette

#ifndef SDRAM_H_
#define SDRAM_H_

#include <Arduino.h>

class SDRAM {
public:
    SDRAM();

    bool begin();

private:
    SDRAM_HandleTypeDef _hsdram;
    FMC_SDRAM_TimingTypeDef _timing;
    FMC_SDRAM_CommandTypeDef _command;
};

#endif /* SDRAM_H_ */
