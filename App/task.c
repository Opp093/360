#include "task.h"

#include "stm32f1xx_hal.h"
#include "main.h"

#include "MAX96755.h"
#include "BL.h"
#include "cdce913.h"


sys_check_state_type sys_check_state;

void EN_VDD2() {
    while(HAL_GPIO_ReadPin(VDD1_PG_GPIO_Port, VDD1_PG_Pin) != GPIO_PIN_SET)
        ;
    HAL_Delay(10);
    HAL_GPIO_WritePin(VDD2_EN_GPIO_Port, VDD2_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(10);   
}

void EN_BL() {
    HAL_GPIO_WritePin(BL_EN_GPIO_Port, BL_EN_Pin, GPIO_PIN_SET);
}

void DISEN_BL() {
    HAL_GPIO_WritePin(BL_EN_GPIO_Port, BL_EN_Pin, GPIO_PIN_RESET);
}

void EN_LCD_RESET() {
    HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
}

void sys_init() {
    EN_VDD2();
    max96755_init();

    if(max96755_check_chip()) {
        sys_check_state.max755_state = IC_CHECK_SUCCESS;
        max96755_init_config();
    } else {
        sys_check_state.max755_state = IC_CHECK_FAIL;
    }
	
	HAL_Delay(50);
    if(max96752_check_chip()) {
        //TODO: config 86752
        sys_check_state.max752_state = IC_CHECK_SUCCESS;
        max96752_init_config();
    } else {
        sys_check_state.max752_state = IC_CHECK_FAIL;
    }
	

    //EN_LCD_RESET();

    HAL_Delay(50);

	// HAL_Delay(50);
    // if(BL_check_chip()) {
	// 	max_96752_register_data_type t;
	// 	t.Address = 0x0007;
    //     sys_check_state.BL_state = IC_CHECK_SUCCESS;
    //     BL_enable();        
    // } else {
    //     sys_check_state.BL_state = IC_CHECK_FAIL;
    // }

    if(cdce913_check_chip()) {
        cdce913_config_98MHZ();
    }


}

void check_test() {
    if(max96755_check_chip()) {
        sys_check_state.max755_state = IC_CHECK_SUCCESS;
        max_96752_register_data_type t[3];
        t[0].Address = 0x0007;
        t[1].Address = 0x01C8;
		t[2].Address = 0x0308;
        max96755_read(&t[0]);
		max96755_read(&t[1]);
		max96755_read(&t[2]);
    } else {
        sys_check_state.max755_state = IC_CHECK_FAIL;
    }


while(1) {
    HAL_Delay(2000);
    max96755_video_RMW_config((max_96755_register_RMW_CFG_TYPE *)s_96755_video_RMW_config_1920_check_board, CONFIG_NUM_755_VIDEO_checkboard);
    // DISEN_BL();
    HAL_Delay(2000);
	max96755_video_RMW_config((max_96755_register_RMW_CFG_TYPE *)s_96755_video_RMW_config_1920_color_bar, CONFIG_NUM_755_VIDEO_color_bar);
    // EN_BL();
		
    // if(HAL_GPIO_ReadPin(GPIOC,BL_EN_Pin) == GPIO_PIN_SET){
    //     HAL_GPIO_WritePin(GPIOC,BL_EN_Pin ,GPIO_PIN_RESET);
    //     HAL_GPIO_WritePin(GPIOC,BL_PWM_Pin ,GPIO_PIN_RESET);
    // }
    // else{
    //     HAL_GPIO_WritePin(GPIOC,BL_EN_Pin ,GPIO_PIN_SET);
    //     HAL_Delay(30);
			
	// 	if(BL_check_chip()) {
	// 		max_96752_register_data_type t;
	// 		t.Address = 0x0007;
    //         sys_check_state.BL_state = IC_CHECK_SUCCESS;
    //         BL_enable();        
	// 	} else {
    //         sys_check_state.BL_state = IC_CHECK_FAIL;
    //     }

    //     HAL_GPIO_WritePin(GPIOC,BL_PWM_Pin ,GPIO_PIN_SET);
    // }
}

}
