#include "cdce913.h"
#include "string.h"
#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

static const cdce913_register_data_type cdce913_150MHZ_config[cdce_cfg_num] = {
    // {0,0x01},
    // {1,0x00},
    {0x02,0xB4},
    {0x03,0x01},
    {0x04,0x02},
    {0x05,0x50},
    // {6,0x40},
    // {7,0x00},
    // {8,0x00},
    // {9,0x00},
    // {10,0x00},
    // {11,0x00},
    // {12,0x00},
    // {13,0x00},
    // {14,0x00},
    // {15,0x00},
    {0x10,0x00},
    {0x11,0x00},
    {0x12,0x00},

    {0x13,0x00},    //frequency
    {0x14,0x6D},
    {0x15,0x02},
    {0x16,0x00},
    {0x17,0x00},
    {0x18,0xBF},
    {0x19,0xA0},
    {0x1A,0x03},
    {0x1B,0x0A},
    {0x1C,0xBF},
    {0x1D,0xA0},
    {0x1E,0x03},
    {0x1F,0x08},
};


static const cdce913_register_data_type cdce913_25MHZ_config[cdce_cfg_num] = {
        // {0,0x01},
    // {1,0x00},
    {0x02,0x34},
    {0x03,0x01},
    {0x04,0x02},
    {0x05,0x50},
    // {6,0x40},
    // {7,0x00},
    // {8,0x00},
    // {9,0x00},
    // {10,0x00},
    // {11,0x00},
    // {12,0x00},
    // {13,0x00},
    // {14,0x00},
    // {15,0x00},
    {0x10,0x00},
    {0x11,0x00},
    {0x12,0x00},

    {0x13,0x00},    //frequency
    {0x14,0xED},
    {0x15,0x02},
    {0x16,0x00},
    {0x17,0x00},
    {0x18,0x66},
    {0x19,0x09},
    {0x1A,0x93},
    {0x1B,0x2C},
    {0x1C,0x66},
    {0x1D,0x09},
    {0x1E,0x93},
    {0x1F,0x2C},
};

static const cdce913_register_data_type cdce913_90MHZ_config[cdce_cfg_num] = {
        // {0,0x01},
    // {1,0x00},
    {0x02,0xB4},
    {0x03,0x02},
    {0x04,0x02},
    {0x05,0x50},
    // {6,0x40},
    // {7,0x00},
    // {8,0x00},
    // {9,0x00},
    // {10,0x00},
    // {11,0x00},
    // {12,0x00},
    // {13,0x00},
    // {14,0x00},
    // {15,0x00},
    {0x10,0x00},
    {0x11,0x00},
    {0x12,0x00},

    {0x13,0x00},    //frequency
    {0x14,0x6D},
    {0x15,0x02},
    {0x16,0x00},
    {0x17,0x00},
    {0x18,0xE5},
    {0x19,0x8C},
    {0x1A,0xC3},
    {0x1B,0x8B},
    {0x1C,0xE5},
    {0x1D,0x8C},
    {0x1E,0xC3},
    {0x1F,0x88},
};


static const cdce913_register_data_type cdce913_120MHZ_config[cdce_cfg_num] = {
    // {0x00, 0x01},
    // {0x01, 0x00},
    {0x02, 0xb4},
    {0x03, 0x01},
    {0x04, 0x02},
    {0x05, 0x50},
    // {0x06, 0x40},
    // {0x07, 0x00},
    // {0x08, 0x00},
    // {0x09, 0x00},
    // {0x0a, 0x00},
    // {0x0b, 0x00},
    // {0x0c, 0x00},
    // {0x0d, 0x00},
    // {0x0e, 0x00},
    // {0x0f, 0x00},
    {0x10, 0x00},
    {0x11, 0x00},
    {0x12, 0x00},
    {0x13, 0x00},
    {0x14, 0x6D},
    {0x15, 0x02},
    {0x16, 0x00},
    {0x17, 0x00},
    {0x18, 0x99},
    {0x19, 0x03},

    {0x1A, 0x32},
    {0x1B, 0x68},
    {0x1C, 0x99},
    {0x1D, 0x03},
    {0x1E, 0x32},
    {0x1F, 0x68},
};

static const cdce913_register_data_type cdce913_98MHZ_config[cdce_cfg_num] = {
    // {0x00, 0x01},
    // {0x01, 0x00},
    {0x02, 0xB4},
    {0x03, 0x02},
    {0x04, 0x02},
    {0x05, 0x50},
    // {0x06, 0x40},
    // {0x07, 0x00},
    // {0x08, 0x00},
    // {0x09, 0x00},
    // {0x0A, 0x00},
    // {0x0B, 0x00},
    // {0x0C, 0x00},
    // {0x0D, 0x00},
    // {0x0E, 0x00},
    // {0x0F, 0x00},
    {0x10, 0x00},
    {0x11, 0x00},
    {0x12, 0x00},
    {0x13, 0x00},
    {0x14, 0x6D},
    {0x15, 0x02},
    {0x16, 0x00},
    {0x17, 0x00},
    {0x18, 0xF5},
    {0x19, 0x05},
    {0x1A, 0xA3},
    {0x1B, 0xEB},
    {0x1C, 0xF5},
    {0x1D, 0x05},
    {0x1E, 0xA3},
    {0x1F, 0xE8},
};


void cdce913_send(cdce913_register_data_type *frame) {
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 3, true, 50);
    frame->Address |= 0x80;
    HAL_I2C_Master_Transmit(&hi2c1, cdce913_IC_ADDRESS, (uint8_t*)frame, 2, 1000);
}

void cdce913_read(cdce913_register_data_type *frame) {
    frame->Address |= 0x80;
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 2, false, 50);
    HAL_I2C_Master_Transmit(&hi2c1, cdce913_IC_ADDRESS, (uint8_t*)frame, 1, 1000);
    // I2C_DRV_MasterReceiveDataBlocking(DES_IIC_INSTANCE, &(frame->DATA), 1, true, 50);
    HAL_I2C_Master_Receive(&hi2c1, cdce913_IC_ADDRESS, &(frame->DATA), 1, 1000);
}

void cdce913_config_150MHZ() {
    cdce913_register_data_type cdce913_150MHZ_config_temp[cdce_cfg_num];
    memcpy(cdce913_150MHZ_config_temp, cdce913_150MHZ_config, sizeof(cdce913_150MHZ_config));

    for (int i = 0; i < cdce_cfg_num; i++) {
        cdce913_send(&cdce913_150MHZ_config_temp[i]);
    }
}

void cdce913_config_25MHZ() {
    cdce913_register_data_type cdce913_25MHZ_config_temp[cdce_cfg_num];
    memcpy(cdce913_25MHZ_config_temp, cdce913_25MHZ_config, sizeof(cdce913_25MHZ_config));

    for (int i = 0; i < cdce_cfg_num; i++) {
        cdce913_send(&cdce913_25MHZ_config_temp[i]);
    }
}

void cdce913_config_90MHZ() {
    cdce913_register_data_type cdce913_90MHZ_config_temp[cdce_cfg_num];
    memcpy(cdce913_90MHZ_config_temp, cdce913_90MHZ_config, sizeof(cdce913_90MHZ_config));

    for (int i = 0; i < cdce_cfg_num; i++) {
        cdce913_send(&cdce913_90MHZ_config_temp[i]);
    }
}

void cdce913_config_120MHZ() {
    cdce913_register_data_type cdce913_120MHZ_config_temp[cdce_cfg_num];
    memcpy(cdce913_120MHZ_config_temp, cdce913_120MHZ_config, sizeof(cdce913_120MHZ_config));

    for (int i = 0; i < cdce_cfg_num; i++) {
        cdce913_send(&cdce913_120MHZ_config_temp[i]);
    }
}

void cdce913_config_98MHZ() {
    cdce913_register_data_type cdce913_98MHZ_config_temp[cdce_cfg_num];
    memcpy(cdce913_98MHZ_config_temp, cdce913_98MHZ_config, sizeof(cdce913_98MHZ_config));

    for (int i = 0; i < cdce_cfg_num; i++) {
        cdce913_send(&cdce913_98MHZ_config_temp[i]);
    }
}


uint8_t cdce913_check_chip() {
   cdce913_register_data_type frame;
   frame.Address = 0x00;
   frame.DATA = 0x00;
   cdce913_read(&frame);
   return frame.DATA == 0x81 || frame.DATA == 0x01; //0x81:3.3V 0x01:1.8v 

}