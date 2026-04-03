#ifndef __TASK_H__
#define __TASK_H__

void EN_VDD2();
void sys_init();

void check_test();

void EN_LCD_RESET();

void EN_BL();
void DISEN_BL();

typedef enum {
    IC_CHECK_SUCCESS,
    IC_CHECK_FAIL
}IC_CHECK_STATE;


typedef struct {
    IC_CHECK_STATE max755_state;
    IC_CHECK_STATE max752_state;
    IC_CHECK_STATE BL_state;
}sys_check_state_type;


#endif