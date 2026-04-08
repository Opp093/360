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
/** res: 0=1920x1080 timing, 1=800x480 timing; lvds: 0=dual, 1=single (same tables until HW-specific RMW added); pattern: 0=color bar, 1=checkerboard */
void max96755_apply_screen_preset(uint8_t res, uint8_t lvds, uint8_t pattern);
/**
 * 按水平有效像素、垂直有效行、刷新率(0.1Hz)生成内建图案时序并写入 MAX96755。
 * 以 800x480(880 行模板)为参考做比例与帧长推算；板级 PCLK 按约 98MHz 估算帧总长(用于空白分配)。
 * pattern: 0=彩条, 1=棋盘格。lvds 预留(当前与时序同发,可后续加专用 RMW)。
 */
void max96755_apply_user_timing(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
				uint8_t lvds, uint8_t pattern);
/**
 * 同 max96755_apply_user_timing，但由上位机指定像素时钟 PCLK（Hz）。
 * pclk_hz 用于依据 refresh_fps_x10 反推 H_total / V_total 的分配。
 */
void max96755_apply_user_timing_pclk(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
				     uint32_t pclk_hz, uint8_t lvds, uint8_t pattern);
void max96755_apply_user_timing_full(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
				     uint32_t pclk_hz, uint16_t hbp, uint16_t hfp, uint16_t vbp, uint16_t vfp,
				     uint8_t lvds, uint8_t pattern);

//-------------------------------------------------------------------
// public vars
//-------------------------------------------------------------------
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_1920_color_bar[CONFIG_NUM_755_VIDEO_color_bar];
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_880_color_bar[CONFIG_NUM_755_VIDEO_color_bar];
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_880_check_board[CONFIG_NUM_755_VIDEO_checkboard];
extern const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_1920_check_board[CONFIG_NUM_755_VIDEO_checkboard];

#endif