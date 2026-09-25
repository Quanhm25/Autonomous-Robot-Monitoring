#include "sensor/bh1750.h"
#include "i2c1.h"

void BH1750_Init(void)
{
    uint8_t cmd = BH1750_CMD_POWER_ON;
    I2C1_WriteBytes(BH1750_ADDR, &cmd, 1);

    cmd = BH1750_CMD_CONT_H;
    I2C1_WriteBytes(BH1750_ADDR, &cmd, 1);
}

uint8_t BH1750_ReadLux(float *lux)
{
    uint8_t buf[2];
    if(!I2C1_ReadBytes(BH1750_ADDR, buf, 2)) {
    	return 0;
    }
    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    *lux = raw / 1.2f;

    return 1;
}
