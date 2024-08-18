/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include "board.h"
 
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>

#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))

// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);

// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  
    
    return 0;
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

/**
 * This function will initial your board.
 */
static void SystemClock_Config(void);
void rt_hw_board_init()
{
    HAL_Init();
    /* System Clock Update */
    SystemClock_Config();
    
    /* System Tick Configuration */
    _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}
/*
 * @brief   时钟系统配置函数
 * @param   无
 * @return  无
 */
static void SystemClock_Config(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_OscInitTypeDef RCC_OscInitStructure; 
    RCC_ClkInitTypeDef RCC_ClkInitStructure;
    
    RCC_OscInitStructure.OscillatorType = RCC_OSCILLATORTYPE_HSE;    	/* 时钟源为HSE */
    RCC_OscInitStructure.HSEState       = RCC_HSE_ON;                   /* 打开HSE */
	RCC_OscInitStructure.HSEPredivValue = RCC_HSE_PREDIV_DIV1;		    /* HSE预分频 */
    RCC_OscInitStructure.PLL.PLLState   = RCC_PLL_ON;					/* 打开PLL */
    RCC_OscInitStructure.PLL.PLLSource  = RCC_PLLSOURCE_HSE;			/* PLL时钟源选择HSE */
    RCC_OscInitStructure.PLL.PLLMUL     = RCC_PLL_MUL9; 				/* 主PLL倍频因子 */
    ret=HAL_RCC_OscConfig(&RCC_OscInitStructure);                       /* 初始化 */
	
    if(ret!=HAL_OK) while(1);
    
    /* 选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2 */
    RCC_ClkInitStructure.ClockType      = (RCC_CLOCKTYPE_SYSCLK | 
                                           RCC_CLOCKTYPE_HCLK | 
                                           RCC_CLOCKTYPE_PCLK1 | 
                                           RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStructure.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;		//设置系统时钟时钟源为PLL
    RCC_ClkInitStructure.AHBCLKDivider  = RCC_SYSCLK_DIV1;				//AHB分频系数为1
    RCC_ClkInitStructure.APB1CLKDivider = RCC_HCLK_DIV2; 				//APB1分频系数为2
    RCC_ClkInitStructure.APB2CLKDivider = RCC_HCLK_DIV1; 				//APB2分频系数为1
    ret=HAL_RCC_ClockConfig(&RCC_ClkInitStructure,FLASH_LATENCY_2);	    //同时设置FLASH延时周期为2WS，也就是3个CPU周期。
		
    if(ret!=HAL_OK) while(1);
}
