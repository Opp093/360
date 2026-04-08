#include "MAX96755.h"

#include "stm32f1xx_hal.h"
#include "main.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

static const max_96752_register_data_type s_96752_init_config[] = {
	{0x0002,0xC3},	//关闭反向音频通道 GPIO6(pin27) GPIO7(pin28) GPIO8(pin29)开启,lock pin indicate video output
	
	{0x0213,0x63},	//上拉输出 ID : 3
	{0x0212,0x03},	//配置GPIO6 : CRQ


	{0x0216,0x21},	//推挽
	{0x0217,0x01},	//ID : 1
	{0x0215,0x04},	//配置GPIO7 : BL_PWM



	{0x020D,0x22},	//推挽
	{0x020E,0x02},	//ID : 2
	{0x020C,0x04},	//GPIO4 : BL_EN

	// {0x01D0, 0xBB},//0xBB
	// {0x01D1, 0xAA},//0xAA
	// {0x01D2, 0x99},//0x99
	// {0x01D3, 0x88},//0x88
	// {0x01D4, 0xCC},//0xCC


//	//开发测试板PIN定义，正式板需根据实际更改

//	{0x0140,0x20},		//关闭音频正向通道 GPIO11,GPIO12,GPIO13开启

//	{0x0221,0x84},		//LCD_STB
//	{0x0222,0x22},
//	{0x0223,0x02},		//ID:2

//	{0x0224,0x84},		//BL_EN	GPIO12
//	{0x0225,0x22},		//pulldown
//	{0x0226,0x02},		//ID:2

//	{0x0227,0x84},		//BL_PWM GPIO13
//	{0x0228,0x21},			//
//	{0x0229,0x01},		//ID:1

	{0xFFFF, 0xFF},
	
};



static const max_96752_register_data_type s_96755_init_config[] = {

	{0x02D6, 0x03},		//BL_EN	GPIO8
	{0x02D7, 0xA2},		//ID:2

	{0x02D9, 0x03},		//BL_PWM GPIO9
	{0x02DA, 0x21},		//ID:1

//	{0X02E2, 0x83},		//LCD_RESET GPIO12 
//	{0x02E3, 0xA3},		//ID:3
	{0x02d3, 0x04},				//GPIO7
	{0x02d5, 0x03},
	{0xFFFF, 0xFF},

};

const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_1920_color_bar[CONFIG_NUM_755_VIDEO_color_bar] = {
	{0x0053, 0x00, 0x03},		//设置x,y,z,u视频通道流id
	{0x0057, 0x01, 0x03},		
	{0x005B, 0x02, 0x03},		
	{0x005F, 0x03, 0x03},		
	{0x01C8, 0x00, 0x04},		//翻转DE
	{0x01C8, 0x08, 0x08},		//不反转HS
	{0x01C8, 0x10, 0x10},		//不反转VS
	{0x01CA, 0x00, 0xFF},		//VS延时[13:16]
	{0x01CB, 0x00, 0xFF},		//VS延时[15:8]
	{0x01CC, 0x00, 0xFF},		//VS延时[7:0]
	{0x01CD, 0x00, 0xFF},		//VS高周期[23:16]
	{0x01CE, 0x10, 0xFF},		//VS高周期[15:8]
	{0x01CF, 0x00, 0xFF},		//VS低周期[7:0]
	{0x01D0, 0x22, 0xFF},		//VS边到第一个HS上升沿的周期[23:16]
	{0x01D1, 0xC8, 0xFF},		//VS边到第一个HS上升沿的周期[15:8]
	{0x01D2, 0x00, 0xFF},		//VS边到第一个HS上升沿的周期[7:0]
	{0x01D3, 0x00, 0xFF},		//HS高周期[15:8]
	{0x01D4, 0x00, 0xFF},		//HS高周期[7:0]
	{0x01D5, 0x00, 0xFF},		//HS低周期[15:8]
	{0x01D6, 0x00, 0xFF},		//HS低周期[7:0]
	{0x01D7, 0x0C, 0xFF},		//每帧HS同步[15:8]
	{0x01D8, 0x07, 0xFF},		//每帧HS同步[7:0]
	{0x01D9, 0xF4, 0xFF},		//VS边到第一DE上升沿的周期[23:16]
	{0x01DA, 0x04, 0xFF},		//VS边到第一DE上升沿的周期[15:8]
	{0x01DB, 0x5B, 0xFF},		//VS边到第一DE上升沿的周期[7:0]
	{0x01DC, 0x00, 0xFF},		//DE高周期[15:8]
	{0x01DD, 0x38, 0xFF},		//DE高周期[7:0]
	{0x01DE, 0x1C, 0xFF},		//DE低周期[15:8]
	{0x01DF, 0x07, 0xFF},		//DE低周期[7:0]
	{0x01E0, 0x80, 0xFF},		//每帧有效line[15:8]
	{0x01E1, 0x00, 0xFF},		//每帧有效line[7:0]
	{0x01E2, 0x80, 0xFF},		//检测板模式
	{0x01E3, 0x04, 0xFF},		
	{0x01E4, 0x38, 0xFF},		
	{0x01E5, 0x02, 0x03},		
	{0x01E6, 0x02, 0xFF},//自增的个数
	{0x0007, 0x01, 0x01},
	{0x01C8, 0xE3, 0xE3},								
			//设置x,y,z,u视频通道流id
	
	
	
			//翻转DE
			//不反转HS
			//不反转VS
			//VS延时[13:16]
			//VS延时[15:8]
			//VS延时[7:0]
			//VS高周期[23:16]
			//VS高周期[15:8]
			//VS高周期[7:0]
			//VS低周期[23:16]
			//VS低周期[15:8]
			//VS低周期[7:0]
			//VS边到第一个HS上升沿的周期[23:16]
			//VS边到第一个HS上升沿的周期[15:8]
			//VS边到第一个HS上升沿的周期[7:0]
			//HS高周期[15:8]
			//HS高周期[7:0]
			//HS低周期[15:8]
			//HS低周期[7:0]
			//每帧HS同步[15:8]
			//每帧HS同步[7:0]
			//VS边到第一DE上升沿的周期[23:16]
			//VS边到第一DE上升沿的周期[15:8]
			//VS边到第一DE上升沿的周期[7:0]
			//DE高周期[15:8]
			//DE高周期[7:0]
			//DE低周期[15:8]
			//DE低周期[7:0]
			//每帧有效line[15:8]
			//每帧有效line[7:0]
			//检测板模式
			// {0x0007, 0x01, 0x01},		//pclk源来自MFP3
			//free run mode
};

const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_1920_check_board[CONFIG_NUM_755_VIDEO_checkboard] = {
	{0x0053, 0x00, 0x03},
	{0x0057, 0x01, 0x03},
	{0x005B, 0x02, 0x03},
	{0x005F, 0x03, 0x03},
	{0x01C8, 0x00, 0x04},
	{0x01C8, 0x08, 0x08},
	{0x01C8, 0x10, 0x10},
	{0x01CA, 0x00, 0xFF},
	{0x01CB, 0x00, 0xFF},
	{0x01CC, 0x00, 0xFF},
	{0x01CD, 0x00, 0xFF},
	{0x01CE, 0x10, 0xFF},
	{0x01CF, 0x00, 0xFF},
	{0x01D0, 0x22, 0xFF},
	{0x01D1, 0xC8, 0xFF},
	{0x01D2, 0x00, 0xFF},
	{0x01D3, 0x00, 0xFF},
	{0x01D4, 0x00, 0xFF},
	{0x01D5, 0x00, 0xFF},
	{0x01D6, 0x00, 0xFF},
	{0x01D7, 0x0C, 0xFF},
	{0x01D8, 0x07, 0xFF},
	{0x01D9, 0xF4, 0xFF},
	{0x01DA, 0x04, 0xFF},
	{0x01DB, 0x5B, 0xFF},
	{0x01DC, 0x00, 0xFF},
	{0x01DD, 0x28, 0xFF},
	{0x01DE, 0x10, 0xFF},
	{0x01DF, 0x07, 0xFF},
	{0x01E0, 0x80, 0xFF},
	{0x01E1, 0x00, 0xFF},
	{0x01E2, 0x80, 0xFF},
	{0x01E3, 0x04, 0xFF},
	{0x01E4, 0x38, 0xFF},
	{0x01E5, 0x01, 0x03},
	{0x01E7, 0xFF, 0xFF},
	{0x01E8, 0xFF, 0xFF},
	{0x01E9, 0xFF, 0xFF},
	{0x01EA, 0x00, 0xFF},
	{0x01EB, 0x00, 0xFF},
	{0x01EC, 0x00, 0xFF},
	{0x01ED, 0xF0, 0xFF},
	{0x01EE, 0xF0, 0xFF},
	{0x01EF, 0x78, 0xFF},
	{0x0007, 0x01, 0x01},
	{0x01C8, 0xE3, 0xE3},
};

const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_880_color_bar[CONFIG_NUM_755_VIDEO_color_bar] = {
	{0x0053, 0x00, 0x03},			//设置x,y,z,u视频通道流id
	{0x0057, 0x01, 0x03},
	{0x005B, 0x02, 0x03},
	{0x005F, 0x03, 0x03},			
	{0x01C8, 0x00, 0x04},			//翻转DE
	{0x01C8, 0x08, 0x08},			//不反转HS
	{0x01C8, 0x10, 0x10},			//不反转VS
	{0x01CA, 0x00, 0xFF},			//VS延时[13:16]
	{0x01CB, 0x00, 0xFF},			//VS延时[15:8]
	{0x01CC, 0x00, 0xFF},			//VS延时[7:0]
	{0x01CD, 0x00, 0xFF},			//VS高周期[23:16]					1760
	{0x01CE, 0x06, 0xFF},			//VS高周期[15:8]	
	{0x01CF, 0xE0, 0xFF},			//VS高周期[7:0]
	{0x01D0, 0x06, 0xFF},			//VS低周期[23:16]					432080
	{0x01D1, 0x97, 0xFF},			//VS低周期[15:8]
	{0x01D2, 0xD0, 0xFF},			//VS低周期[7:0]
	{0x01D3, 0x00, 0xFF},			//VS边到第一个HS上升沿的周期[23:16]
	{0x01D4, 0x00, 0xFF},			//VS边到第一个HS上升沿的周期[15:8]
	{0x01D5, 0x00, 0xFF},			//VS边到第一个HS上升沿的周期[7:0]
	{0x01D6, 0x00, 0xFF},			//HS高周期[15:8]					12		HS sync
	{0x01D7, 0x0C, 0xFF},			//HS高周期[7:0]
	{0x01D8, 0x03, 0xFF},			//HS低周期[15:8]					868
	{0x01D9, 0x64, 0xFF},			//HS低周期[7:0]
	{0x01DA, 0x01, 0xFF},			//每帧HS同步[15:8]					493
	{0x01DB, 0xED, 0xFF},			//每帧HS同步[7:0]
	{0x01DC, 0x00, 0xFF},			//VS边到第一DE上升沿的周期[23:16]	 15300
	{0x01DD, 0x14, 0xFF},			//VS边到第一DE上升沿的周期[15:8]
	{0x01DE, 0xB4, 0xFF},			//VS边到第一DE上升沿的周期[7:0]
	{0x01DF, 0x03, 0xFF},			//DE高周期[15:8]					800
	{0x01E0, 0x20, 0xFF},			//DE高周期[7:0]
	{0x01E1, 0x00, 0xFF},			//DE低周期[15:8]					80
	{0x01E2, 0x50, 0xFF},			//DE低周期[7:0]
	{0x01E3, 0x01, 0xFF},			//每帧有效line[15:8]				480
	{0x01E4, 0xE0, 0xFF},			//每帧有效line[7:0]
	{0x01E5, 0x02, 0x03},			//color bar模式
	{0x01E6, 0x04, 0xFF},			//自增
	{0x0007, 0x01, 0x01},			//MFP3 -> pclk
	{0x01C8, 0xE3, 0xE3},			//free run mode
};

const max_96755_register_RMW_CFG_TYPE s_96755_video_RMW_config_880_check_board[CONFIG_NUM_755_VIDEO_checkboard] = {
	{0x0053, 0x00, 0x03},
	{0x0057, 0x01, 0x03},
	{0x005B, 0x02, 0x03},
	{0x005F, 0x03, 0x03},
	{0x01C8, 0x00, 0x04},
	{0x01C8, 0x08, 0x08},
	{0x01C8, 0x10, 0x10},
	{0x01CA, 0x00, 0xFF},
	{0x01CB, 0x00, 0xFF},
	{0x01CC, 0x00, 0xFF},
	{0x01CD, 0x00, 0xFF},
	{0x01CE, 0x06, 0xFF},
	{0x01CF, 0xE0, 0xFF},
	{0x01D0, 0x06, 0xFF},
	{0x01D1, 0x97, 0xFF},
	{0x01D2, 0xD0, 0xFF},
	{0x01D3, 0x00, 0xFF},
	{0x01D4, 0x00, 0xFF},
	{0x01D5, 0x00, 0xFF},
	{0x01D6, 0x00, 0xFF},
	{0x01D7, 0x0C, 0xFF},
	{0x01D8, 0x03, 0xFF},
	{0x01D9, 0x64, 0xFF},
	{0x01DA, 0x01, 0xFF},
	{0x01DB, 0xED, 0xFF},
	{0x01DC, 0x00, 0xFF},
	{0x01DD, 0x14, 0xFF},
	{0x01DE, 0xB4, 0xFF},
	{0x01DF, 0x03, 0xFF},
	{0x01E0, 0x20, 0xFF},
	{0x01E1, 0x00, 0xFF},
	{0x01E2, 0x50, 0xFF},
	{0x01E3, 0x01, 0xFF},
	{0x01E4, 0xE0, 0xFF},
	{0x01E5, 0x01, 0x03},
	{0x01E7, 0xFF, 0xFF},
	{0x01E8, 0xFF, 0xFF},
	{0x01E9, 0xFF, 0xFF},
	{0x01EA, 0x00, 0xFF},
	{0x01EB, 0x00, 0xFF},
	{0x01EC, 0x00, 0xFF},
	{0x01ED, 0xF0, 0xFF},
	{0x01EE, 0xF0, 0xFF},
	{0x01EF, 0x78, 0xFF},
	{0x0007, 0x01, 0x01},
	{0x01C8, 0xE3, 0xE3},
};





void max96755_send(max_96752_register_data_type const *frame) {
	uint8_t data[3];
	data[0] = ((uint8_t*)frame)[1];
	data[1] = ((uint8_t*)frame)[0];
	data[2] = ((uint8_t*)frame)[2];
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 3, true, 50);
    HAL_I2C_Master_Transmit(&hi2c1, MAX96755_ADDRESS, data, 3, 1000);
}

void max96752_send(max_96752_register_data_type const *frame) {
	uint8_t data[3];
	data[0] = ((uint8_t*)frame)[1];
	data[1] = ((uint8_t*)frame)[0];
	data[2] = ((uint8_t*)frame)[2];
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 3, true, 50);
    HAL_I2C_Master_Transmit(&hi2c1, MAX96752_ADDRESS, data, 3, 1000);
}

void max96755_read(max_96752_register_data_type *frame) {
	uint8_t data[2];
	data[0] = ((uint8_t*)frame)[1];
	data[1] = ((uint8_t*)frame)[0];
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 2, false, 50);
    HAL_I2C_Master_Transmit(&hi2c1, MAX96755_ADDRESS, data, 2, 1000);
    // I2C_DRV_MasterReceiveDataBlocking(DES_IIC_INSTANCE, &(frame->DATA), 1, true, 50);
    HAL_I2C_Master_Receive(&hi2c1, MAX96755_ADDRESS, &(frame->DATA), 1, 1000);
}

void max96752_read(max_96752_register_data_type *frame) {
	uint8_t data[2];
	data[0] = ((uint8_t*)frame)[1];
	data[1] = ((uint8_t*)frame)[0];
    // I2C_DRV_MasterSendDataBlocking(DES_IIC_INSTANCE, data, 2, false, 50);
    HAL_I2C_Master_Transmit(&hi2c1, MAX96752_ADDRESS, data, 2, 1000);
    // I2C_DRV_MasterReceiveDataBlocking(DES_IIC_INSTANCE, &(frame->DATA), 1, true, 50);
    HAL_I2C_Master_Receive(&hi2c1, MAX96752_ADDRESS, &(frame->DATA), 1, 1000);
}

void max96755_init(){
    HAL_GPIO_WritePin(MAX96755_PDEN_GPIO_Port, MAX96755_PDEN_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
}

uint8_t max96755_check_chip() {
    max_96752_register_data_type frame;
    frame.Address = 0x000D;
    frame.DATA = 0x00;
    max96755_read(&frame);
    return frame.DATA == 0x9B; //SER的ID寄存器default是0x9B
}

uint8_t max96752_check_chip() {
    max_96752_register_data_type frame;
    frame.Address = 0x000D;
    frame.DATA = 0x00;
    max96752_read(&frame);
    return frame.DATA == 0x82; //des的ID寄存器default是0x82
}

void max96752_init_config() {
	uint8_t index = 0;
	while(s_96752_init_config[index].Address != 0xFFFF) {
		max96752_send(&s_96752_init_config[index++]);
	}
}

void max96755_init_config() {
	uint8_t index = 0;
	while(s_96755_init_config[index].Address != 0xFFFF) {
		max96755_send(&s_96755_init_config[index++]);
	}
}

void max96755_video_RMW_config(max_96755_register_RMW_CFG_TYPE cfg[], uint8_t cfg_num) {
	for(uint8_t i = 0; i < cfg_num; i++) {
		max_96755_register_RMW_CFG_TYPE frame;
		frame.Address = cfg[i].Address;
		if(cfg[i].mask == 0xFF) {
			max96755_send((max_96752_register_data_type*)&cfg[i]);
		}else {
			max96755_read((max_96752_register_data_type*)&frame);
			frame.DATA = (frame.DATA & ~(cfg[i].mask)) | cfg[i].DATA;
			max96755_send((max_96752_register_data_type*)&frame);
		}
	}
}

void max96755_apply_screen_preset(uint8_t res, uint8_t lvds, uint8_t pattern) {
	const max_96755_register_RMW_CFG_TYPE *tbl = NULL;
	uint8_t n = 0;

	if (res == 0) {
		if (pattern == 0) {
			tbl = s_96755_video_RMW_config_1920_color_bar;
			n = CONFIG_NUM_755_VIDEO_color_bar;
		} else {
			tbl = s_96755_video_RMW_config_1920_check_board;
			n = CONFIG_NUM_755_VIDEO_checkboard;
		}
	} else {
		if (pattern == 0) {
			tbl = s_96755_video_RMW_config_880_color_bar;
			n = CONFIG_NUM_755_VIDEO_color_bar;
		} else {
			tbl = s_96755_video_RMW_config_880_check_board;
			n = CONFIG_NUM_755_VIDEO_checkboard;
		}
	}

	(void)lvds;
	if (tbl != NULL && n > 0) {
		max96755_video_RMW_config((max_96755_register_RMW_CFG_TYPE *)tbl, n);
	}
}

/* 默认与板级 CDCE913 约 98MHz 像素时钟一致 */
#define MAX96755_PCLK_DEFAULT_HZ 98000000UL

static void timing_patch_byte(max_96755_register_RMW_CFG_TYPE *cfg, uint8_t n, uint16_t addr, uint8_t d)
{
	for (uint8_t i = 0; i < n; i++) {
		if (cfg[i].Address == addr && cfg[i].mask == 0xFF) {
			cfg[i].DATA = d;
			return;
		}
	}
}

static void timing_patch_u16(max_96755_register_RMW_CFG_TYPE *cfg, uint8_t n, uint16_t addr, uint16_t v)
{
	timing_patch_byte(cfg, n, addr, (uint8_t)(v >> 8));
	timing_patch_byte(cfg, n, (uint16_t)(addr + 1U), (uint8_t)(v & 0xFF));
}

static void timing_patch_u24(max_96755_register_RMW_CFG_TYPE *cfg, uint8_t n, uint16_t addr, uint32_t v)
{
	if (v > 0xFFFFFFUL) {
		v = 0xFFFFFFUL;
	}
	timing_patch_byte(cfg, n, addr, (uint8_t)((v >> 16) & 0xFF));
	timing_patch_byte(cfg, n, (uint16_t)(addr + 1U), (uint8_t)((v >> 8) & 0xFF));
	timing_patch_byte(cfg, n, (uint16_t)(addr + 2U), (uint8_t)(v & 0xFF));
}

static void max96755_apply_user_timing_core(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
					    uint32_t pclk_hz, uint8_t lvds, uint8_t pattern)
{
	uint8_t pat = (uint8_t)(pattern & 1U);
	uint8_t ncfg = pat ? CONFIG_NUM_755_VIDEO_checkboard : CONFIG_NUM_755_VIDEO_color_bar;
	static max_96755_register_RMW_CFG_TYPE work[CONFIG_NUM_755_VIDEO_checkboard];

	(void)lvds;

	if (h_active < 320U || h_active > 4095U || v_active < 200U || v_active > 2160U) {
		return;
	}
	if (refresh_fps_x10 < 300U || refresh_fps_x10 > 1200U) {
		return;
	}
	if (pclk_hz < 1000000UL || pclk_hz > 200000000UL) {
		return;
	}

	{
		const max_96755_register_RMW_CFG_TYPE *src =
		    pat ? s_96755_video_RMW_config_880_check_board : s_96755_video_RMW_config_880_color_bar;
		memcpy(work, src, (size_t)ncfg * sizeof(work[0]));
	}

	{
		uint32_t frame_pix = (pclk_hz * 10UL) / (uint32_t)refresh_fps_x10;
		uint32_t v_tot = (uint32_t)v_active + (uint32_t)v_active / 8U + 30U;
		uint32_t h_tot;
		uint32_t h_blank;

		if (v_tot < (uint32_t)v_active + 20U) {
			v_tot = (uint32_t)v_active + 20U;
		}
		h_tot = frame_pix / v_tot;
		if (h_tot < (uint32_t)h_active + 48U) {
			h_tot = (uint32_t)h_active + 48U;
			v_tot = frame_pix / h_tot;
			if (v_tot < (uint32_t)v_active + 20U) {
				v_tot = (uint32_t)v_active + 20U;
			}
			h_tot = frame_pix / v_tot;
		}
		h_blank = h_tot - (uint32_t)h_active;
		if (h_blank < 24U) {
			h_tot += (24U - h_blank);
			h_blank = h_tot - (uint32_t)h_active;
			v_tot = frame_pix / h_tot;
		}

		{
			uint16_t hs_h = (uint16_t)((h_tot * 12UL) / 880UL);
			if (hs_h < 8U) {
				hs_h = 8U;
			}
			if ((uint32_t)hs_h + 8UL >= h_tot) {
				hs_h = (uint16_t)(h_tot / 2U);
			}
			{
				uint16_t hs_l = (uint16_t)(h_tot - (uint32_t)hs_h);
				timing_patch_u16(work, ncfg, 0x01D6, hs_h);
				timing_patch_u16(work, ncfg, 0x01D8, hs_l);
			}
		}

		timing_patch_u16(work, ncfg, 0x01DF, h_active);
		timing_patch_u16(work, ncfg, 0x01E1, (uint16_t)h_blank);
		timing_patch_u16(work, ncfg, 0x01E3, v_active);

		{
			const uint32_t ref_vs_high = 0x0006E0UL;
			const uint32_t ref_vs_low = 0x0697D0UL;
			const uint32_t ref_vs2de = 0x0014B4UL;
			uint32_t vs_high;
			uint32_t vs_low;
			uint32_t vs2de;
			uint32_t hs_pf;

			vs_high = (uint32_t)(((uint64_t)ref_vs_high * (uint64_t)v_tot * 600ULL)
					     / ((uint64_t)525 * (uint64_t)refresh_fps_x10));
			vs_low = (uint32_t)(((uint64_t)ref_vs_low * (uint64_t)v_tot * 600ULL)
					    / ((uint64_t)525 * (uint64_t)refresh_fps_x10));
			vs2de = (uint32_t)(((uint64_t)ref_vs2de * (uint64_t)v_tot * (uint64_t)h_active * 600ULL)
					   / ((uint64_t)525 * (uint64_t)800 * (uint64_t)refresh_fps_x10));
			hs_pf = (uint32_t)(((uint64_t)493UL * (uint64_t)v_tot * 600ULL)
					   / ((uint64_t)525 * (uint64_t)refresh_fps_x10));

			timing_patch_u24(work, ncfg, 0x01CD, vs_high);
			timing_patch_u24(work, ncfg, 0x01D0, vs_low);
			timing_patch_u24(work, ncfg, 0x01DC, vs2de);
			timing_patch_u16(work, ncfg, 0x01DA, (uint16_t)(hs_pf > 0xFFFFUL ? 0xFFFFU : hs_pf));
		}
	}

	if (pat != 0U) {
		uint8_t i;
		for (i = 0; i < ncfg; i++) {
			if (work[i].Address >= 0x01E7U && work[i].Address <= 0x01EFU && work[i].mask == 0xFF) {
				uint32_t nv = (uint32_t)(((uint64_t)work[i].DATA * (uint64_t)v_active) / 480ULL);
				if (nv > 255U) {
					nv = 255U;
				}
				work[i].DATA = (uint8_t)nv;
			}
		}
	}

	max96755_video_RMW_config(work, ncfg);
}

static void max96755_apply_user_timing_core_full(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
						 uint32_t pclk_hz, uint16_t hbp, uint16_t hfp, uint16_t vbp,
						 uint16_t vfp, uint8_t lvds, uint8_t pattern)
{
	uint8_t pat = (uint8_t)(pattern & 1U);
	uint8_t ncfg = pat ? CONFIG_NUM_755_VIDEO_checkboard : CONFIG_NUM_755_VIDEO_color_bar;
	static max_96755_register_RMW_CFG_TYPE work[CONFIG_NUM_755_VIDEO_checkboard];

	(void)lvds;
	if (h_active < 320U || h_active > 4095U || v_active < 200U || v_active > 2160U) {
		return;
	}
	if (refresh_fps_x10 < 300U || refresh_fps_x10 > 1200U) {
		return;
	}
	if (pclk_hz < 1000000UL || pclk_hz > 200000000UL) {
		return;
	}
	if (hbp > 4095U || hfp > 4095U || vbp > 4095U || vfp > 4095U) {
		return;
	}

	{
		const max_96755_register_RMW_CFG_TYPE *src =
		    pat ? s_96755_video_RMW_config_880_check_board : s_96755_video_RMW_config_880_color_bar;
		memcpy(work, src, (size_t)ncfg * sizeof(work[0]));
	}

	{
		uint32_t frame_pix = (pclk_hz * 10UL) / (uint32_t)refresh_fps_x10;
		uint32_t h_blank = (uint32_t)hbp + (uint32_t)hfp;
		uint32_t h_tot = (uint32_t)h_active + h_blank;
		uint32_t v_blank = (uint32_t)vbp + (uint32_t)vfp;
		uint32_t v_tot = (uint32_t)v_active + v_blank + 1U;
		uint32_t hs_h = h_tot / 32U;
		uint32_t hs_l;
		uint32_t hs_pf;
		uint32_t vs_high;
		uint32_t vs_low;
		uint32_t vs2de;

		if (h_blank < 8U) h_blank = 8U;
		h_tot = (uint32_t)h_active + h_blank;
		if (v_blank < 2U) v_blank = 2U;
		v_tot = (uint32_t)v_active + v_blank + 1U;
		if (h_tot * v_tot > frame_pix) {
			v_tot = frame_pix / h_tot;
			if (v_tot < (uint32_t)v_active + 3U) {
				v_tot = (uint32_t)v_active + 3U;
			}
		}

		if (hs_h < 8U) hs_h = 8U;
		if (hs_h >= h_tot) hs_h = h_tot / 2U;
		hs_l = h_tot - hs_h;
		hs_pf = (v_tot > 0xFFFFUL) ? 0xFFFFUL : v_tot;
		vs_high = hs_h * (vbp > 0 ? vbp : 1U);
		vs_low = hs_l * (v_tot > (vbp > 0 ? vbp : 1U) ? (v_tot - (vbp > 0 ? vbp : 1U)) : 1U);
		vs2de = (uint32_t)vbp * h_tot + hfp;

		timing_patch_u16(work, ncfg, 0x01D6, (uint16_t)hs_h);
		timing_patch_u16(work, ncfg, 0x01D8, (uint16_t)hs_l);
		timing_patch_u16(work, ncfg, 0x01DA, (uint16_t)hs_pf);
		timing_patch_u24(work, ncfg, 0x01CD, vs_high);
		timing_patch_u24(work, ncfg, 0x01D0, vs_low);
		timing_patch_u24(work, ncfg, 0x01DC, vs2de);
		timing_patch_u16(work, ncfg, 0x01DF, h_active);
		timing_patch_u16(work, ncfg, 0x01E1, (uint16_t)h_blank);
		timing_patch_u16(work, ncfg, 0x01E3, v_active);
	}

	if (pat != 0U) {
		for (uint8_t i = 0; i < ncfg; i++) {
			if (work[i].Address >= 0x01E7U && work[i].Address <= 0x01EFU && work[i].mask == 0xFF) {
				uint32_t nv = (uint32_t)(((uint64_t)work[i].DATA * (uint64_t)v_active) / 480ULL);
				if (nv > 255U) nv = 255U;
				work[i].DATA = (uint8_t)nv;
			}
		}
	}
	max96755_video_RMW_config(work, ncfg);
}

void max96755_apply_user_timing(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
				uint8_t lvds, uint8_t pattern)
{
	max96755_apply_user_timing_core(h_active, v_active, refresh_fps_x10, MAX96755_PCLK_DEFAULT_HZ, lvds, pattern);
}

void max96755_apply_user_timing_pclk(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
				     uint32_t pclk_hz, uint8_t lvds, uint8_t pattern)
{
	max96755_apply_user_timing_core(h_active, v_active, refresh_fps_x10, pclk_hz, lvds, pattern);
}

void max96755_apply_user_timing_full(uint16_t h_active, uint16_t v_active, uint16_t refresh_fps_x10,
				     uint32_t pclk_hz, uint16_t hbp, uint16_t hfp, uint16_t vbp, uint16_t vfp,
				     uint8_t lvds, uint8_t pattern)
{
	max96755_apply_user_timing_core_full(h_active, v_active, refresh_fps_x10, pclk_hz, hbp, hfp, vbp, vfp, lvds, pattern);
}


