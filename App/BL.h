#ifndef __BL_H__
#define __BL_H__

#include "stm32f1xx.h"

#define BL_IC_ADDRESS 0xB2

typedef struct {
    uint8_t Address;
	uint8_t DATA;
}BL_register_data_type;


uint8_t BL_check_chip();
void BL_enable();
void BL_send(BL_register_data_type const *frame);
void BL_read(BL_register_data_type *frame);

#endif