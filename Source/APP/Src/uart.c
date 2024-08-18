#include "uart.h"
#include <rtthread.h>
#include "log.h"

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

#define uart1_rt_buf_size    ( 256 )
static uint8_t uart1_dma_rt_buf[uart1_rt_buf_size] = {0};

void usart_process_data(const void* data, size_t len);



void MX_USART1_UART_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_USART1_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();

	__HAL_RCC_DMA1_CLK_ENABLE();
	/**USART1 GPIO Configuration
	PA9     ------> USART1_TX
	PA10     ------> USART1_RX
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* USART1 DMA Init */
	/* USART1_RX Init */
	hdma_usart1_rx.Instance = DMA1_Channel5;
	hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
	hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_usart1_rx);

	__HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

	/* USART1_TX Init */
	hdma_usart1_tx.Instance = DMA1_Channel4;
	hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_usart1_tx.Init.Mode = DMA_NORMAL;
	hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_usart1_tx);

	__HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

	//串口初始化
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);
    
	__HAL_DMA_ENABLE_IT(&hdma_usart1_tx, DMA_IT_TC);
     //开启空闲接收中断
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

	__HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_TC);
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);

	HAL_UART_Receive_DMA(&huart1, (uint8_t *)&uart1_dma_rt_buf, uart1_rt_buf_size);
}
/**
 * @brief   Check for new data received with DMA
 *
 *
 */

void usart_rx_check(void)
{
    static uint16_t old_pos;
    uint16_t pos;

    /* Calculate current position in buffer and check for new data available */
    pos = uart1_rt_buf_size - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
    if(pos != old_pos){                       /* Check change in received data */
        if(pos > old_pos){                    /* Current position is over previous one */
            /*
             * Processing is done in "linear" mode.
             *
             * Application processing is fast with single data block,
             * length is simply calculated by subtracting pointers
             *
             * [   0   ]
             * [   1   ] <- old_pos |------------------------------------|
             * [   2   ]            |                                    |
             * [   3   ]            | Single block (len = pos - old_pos) |
             * [   4   ]            |                                    |
             * [   5   ]            |------------------------------------|
             * [   6   ] <- pos
             * [   7   ]
             * [ N - 1 ]
             */
            usart_process_data(&uart1_dma_rt_buf[old_pos], pos - old_pos);
        }else{
            /*
             * Processing is done in "overflow" mode..
             *
             * Application must process data twice,
             * since there are 2 linear memory blocks to handle
             *
             * [   0   ]            |---------------------------------|
             * [   1   ]            | Second block (len = pos)        |
             * [   2   ]            |---------------------------------|
             * [   3   ] <- pos
             * [   4   ] <- old_pos |---------------------------------|
             * [   5   ]            |                                 |
             * [   6   ]            | First block (len = N - old_pos) |
             * [   7   ]            |                                 |
             * [ N - 1 ]            |---------------------------------|
             */
            usart_process_data(&uart1_dma_rt_buf[old_pos], uart1_rt_buf_size - old_pos);
            if(pos > 0){
                usart_process_data(&uart1_dma_rt_buf[0], pos);
            }
        }
        old_pos = pos;                          /* Save current position as old for next transfers */
    }
}

void usart_process_data(const void* data, size_t len)
{
//    char buf[] = {0xAA, 0x13, 0x51, 0xBB, '\n'};
    log_printf("receive:%s\n", data);
}

/**
  * @brief This function handles DMA1 channel4 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
//    if(__HAL_DMA_GET_FLAG(&hdma_usart1_tx, DMA_FLAG_TC4) != RESET){
//        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_FLAG_TC4);        //清除DMA传输完成标志
//        HAL_UART_DMAStop(&huart1);		                         //传输完成以后关闭串口DMA,缺了这一句会死机
//    }
//    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_FLAG_TC4);
//    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_FLAG_HT4);

//    __HAL_UNLOCK(&hdma_usart1_tx);
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

/**
  * @brief This function handles DMA1 channel5 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
    rt_interrupt_enter();
    if(__HAL_DMA_GET_FLAG(&hdma_usart1_rx, DMA_FLAG_HT5) != RESET){
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, DMA_FLAG_HT5);
         usart_rx_check();
    }
    if(__HAL_DMA_GET_FLAG(&hdma_usart1_rx, DMA_FLAG_TC5) != RESET){
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, DMA_FLAG_TC5);
         usart_rx_check();
    }
    rt_interrupt_leave();
}


/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
    rt_interrupt_enter();
    //当触发了串口空闲中断
    if((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)){
        __HAL_UART_CLEAR_IDLEFLAG(&huart1); //清除空闲标志
        usart_rx_check();
    } 
    if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != RESET)){
        __HAL_UART_DISABLE_IT(&huart1, UART_IT_TC);
    }

    rt_interrupt_leave();
}