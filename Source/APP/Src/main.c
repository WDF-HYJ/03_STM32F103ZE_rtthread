/* C标准库头文件 */
#include <stdint.h>
/* OS头文件 */
#include "board.h"
#include "rtthread.h"
/* MCU库头文件 */
#include "stm32f1xx_it.h"
#include "stm32f1xx.h"
/* 应用头文件 */
#include "uart.h"

/***********************************************************************
 *                              宏定义
 ***********************************************************************/

/***********************************************************************
 *                           全局变量定义
 ***********************************************************************/
 
 /***********************************************************************
 *                             函数声明
 ***********************************************************************/
void MX_GPIO_Init(void);

/***********************************************************************
 *                             函数定义
 ***********************************************************************/
extern UART_HandleTypeDef huart1;
void LED_thread(void* parameter)
{  
    uint8_t ch[3] = {'a', 'b', 'c'};
    while(1)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET); 	
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_SET);
		rt_thread_delay(1000);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);   	
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_RESET);
		rt_thread_delay(1000);
       // HAL_UART_Transmit_DMA(&huart1, ch, 3);

	}
}
/**
 * @brief   初始化相关   \n
 *  
 * @param   无
 * @return  无
 */
int init_all(void)
{
    MX_GPIO_Init();
	MX_USART1_UART_Init();
	return 0;
}
INIT_DEVICE_EXPORT(init_all);

int main(void)
{  
   	rt_thread_t LED_tid = rt_thread_create("led1", LED_thread/*thread*/, RT_NULL, 512/*stack*/, 3/*priority*/, 20/*tick*/); 
    rt_thread_startup(LED_tid);
	return -1;
}
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

} 



