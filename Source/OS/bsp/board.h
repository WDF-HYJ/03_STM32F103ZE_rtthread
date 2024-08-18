#ifndef __BOARD_H
#define __BOARD_H

/********** 包含头文件 *************/
//stm32 固件库头文件
#include "stm32f1xx.h"

/********** 函数声明 *************/

void rt_hw_board_init(void);
void SysTick_Handler(void);

#endif