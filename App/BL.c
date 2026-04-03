#include "BL.h"

#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

void BL_send(BL_register_data_type const *frame) {
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 3, true, 50);
    HAL_I2C_Master_Transmit(&hi2c1, BL_IC_ADDRESS, (uint8_t*)frame, 2, 1000);
}

void BL_read(BL_register_data_type *frame) {
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 2, false, 50);
    HAL_I2C_Master_Transmit(&hi2c1, BL_IC_ADDRESS, (uint8_t*)frame, 1, 1000);
    // I2C_DRV_MasterReceiveDataBlocking(DES_IIC_INSTANCE, &(frame->DATA), 1, true, 50);
    HAL_I2C_Master_Receive(&hi2c1, BL_IC_ADDRESS, &(frame->DATA), 1, 1000);
}

uint8_t BL_check_chip() {
    BL_register_data_type frame;
    frame.Address = 0x00;
    frame.DATA = 0x00;
    BL_read(&frame);
    return frame.DATA == 0x24; 
}

void BL_enable() {
    BL_register_data_type frame;
    frame.Address = 0x02;
    frame.DATA = 0x3B;                  //开启背光
    BL_send(&frame);
}