#ifndef __MAX96755_H__
#define __MAX96755_H__

#include "stm32f1xx.h"

//-------------------------------------------------------------------
// Private define
//-------------------------------------------------------------------
#define MAX96752_ADDRESS    0x50
#define MAX96752_PORT_IS_A  1
// #define CONFIG_NUM_752      (29+5 - 18)
// #define CONFIG_NUM_755      4
#define CONFIG_NUM_755_VIDEO_color_bar      38 
#define CONFIG_NUM_755_VIDEO_checkboard     46 

#define MAX96755_ADDRESS    0x80



//-------------------------------------------------------------------
// typedef
//-------------------------------------------------------------------
typedef struct {
    uint16_t Address;
	uint8_t DATA;
}max_96752_register_data_type;

typedef struct {
    uint16_t Address;
	uint8_t DATA;
    uint8_t mask;
}max_96755_register_RMW_CFG_TYPE;       //带读写改的类型


//-------------------------------------------------------------------
// public functions
//-------------------------------------------------------------------
void max96752_init();
uint8_t max96752_check_chip();
void max96752_read(max_96752_register_data_type *frame);
void max96752_send(max_96752_register_data_type const *frame);
void max96752_init_config();

void max96755_init();
uint8_t max96755_check_chip();
void max96755_read(max_96752_register_data_type *frame);
void max96755_send(max_96752_register_data_type const *frame);
void max96755_init_config();

void max96755_video_RMW_config(max_96755_register_RMW_CFG_TYPE cfg[], uint8_t cfg_num);

//-------------------------------------------------------------------
// public vars
//-------------------------------------------------------------------
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_1920_color_bar[CONFIG_NUM_755_VIDEO_color_bar];
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_880_color_bar[CONFIG_NUM_755_VIDEO_color_bar];
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_880_check_board[CONFIG_NUM_755_VIDEO_checkboard];
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_1920_check_board[CONFIG_NUM_755_VIDEO_checkboard];

#endif