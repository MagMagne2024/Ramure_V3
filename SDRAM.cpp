// SDRAM
// © 2020-2025 Patrick Lafarguette

#include <SDRAM.h>

#if ARDUINO_DISCO_F746NG

// MT48LC4M32B2B5-6A

#define SDRAM_BANK_ADDR                 ((uint32_t)0xC0000000)

#define SDRAM_MEMORY_WIDTH               FMC_SDRAM_MEM_BUS_WIDTH_16

#define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/**
  * @brief SDRAM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param hsram: SDRAM handle pointer
  * @retval None
  */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef* hsdram) {
    UNUSED(hsdram);
    GPIO_InitTypeDef  GPIO_Init_Structure;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull = GPIO_PULLUP;
    GPIO_Init_Structure.Speed = GPIO_SPEED_FAST;
    GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;


    /* GPIOC configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

    /* GPIOD configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9
            | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    /* GPIOE configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 | GPIO_PIN_8
            | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
            | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    /* GPIOF configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
            | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
            | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

    /* GPIOG configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5
            | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);

    /* GPIOH configuration */
    GPIO_Init_Structure.Pin = GPIO_PIN_3 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOH, &GPIO_Init_Structure);
}

/**
  * @brief  Perform the SDRAM external memory initialization sequence
  * @param  hsdram: SDRAM handle
  * @param  Command: Pointer to SDRAM command structure
  * @retval None
  */
static void BSP_SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *command) {
    __IO uint32_t tmpmrd = 0;
    /* Step 3:  Configure a clock configuration enable command */
    command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command->AutoRefreshNumber = 1;
    command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, SDRAM_TIMEOUT);

    /* Step 4: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 5: Configure a PALL (precharge all) command */
    command->CommandMode = FMC_SDRAM_CMD_PALL;
    command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command->AutoRefreshNumber = 1;
    command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, SDRAM_TIMEOUT);

    /* Step 6 : Configure a Auto-Refresh command */
    command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command->AutoRefreshNumber = 8;
    command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, SDRAM_TIMEOUT);

    /* Step 7: Program the external memory mode register */
    tmpmrd = (uint32_t) SDRAM_MODEREG_BURST_LENGTH_1
            | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | SDRAM_MODEREG_CAS_LATENCY_2
            | SDRAM_MODEREG_OPERATING_MODE_STANDARD
            | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    command->AutoRefreshNumber = 1;
    command->ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, command, SDRAM_TIMEOUT);

    /* Step 8: Set the refresh rate counter */
    /* (15.62 us x Freq) - 20 */
    /* Set the device refresh counter */
    hsdram->Instance->SDRTR |= ((uint32_t) ((1292) << 1));
}

#endif

SDRAM::SDRAM() {
}

bool SDRAM::begin() {
    /*##-1- Configure the SDRAM device #########################################*/
    /* SDRAM device configuration */
    _hsdram.Instance = FMC_SDRAM_DEVICE;

    _timing.LoadToActiveDelay = 2;
    _timing.ExitSelfRefreshDelay = 6;
    _timing.SelfRefreshTime = 4;
    _timing.RowCycleDelay = 6;
    _timing.WriteRecoveryTime = 2;
    _timing.RPDelay = 2;
    _timing.RCDDelay = 2;

    _hsdram.Init.SDBank = FMC_SDRAM_BANK1;
    _hsdram.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
    _hsdram.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
    _hsdram.Init.MemoryDataWidth = SDRAM_MEMORY_WIDTH;
    _hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    _hsdram.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
    _hsdram.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    _hsdram.Init.SDClockPeriod = SDCLOCK_PERIOD;
    _hsdram.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    _hsdram.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;

    /* Initialize the SDRAM controller */
    if (HAL_SDRAM_Init(&_hsdram, &_timing) != HAL_OK) {
        /* Initialization Error */
        return false;
    }

    /* Program the SDRAM external device */
    BSP_SDRAM_Initialization_Sequence(&_hsdram, &_command);

    return true;
}
