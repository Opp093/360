#ifndef __CDCE913_H__
#define __CDCE913_H__

#include "stm32f1xx.h"


#define cdce913_IC_ADDRESS  0xCA
#define cdce_cfg_num        20

typedef struct {
    uint8_t Address;
    uint8_t DATA;
}cdce913_register_data_type;

void cdce913_send(cdce913_register_data_type *frame);
void cdce913_read(cdce913_register_data_type *frame);
uint8_t cdce913_check_chip();
void cdce913_config_150MHZ();
void cdce913_config_25MHZ();
void cdce913_config_90MHZ();
void cdce913_config_120MHZ();
void cdce913_config_98MHZ();

#endif