#ifndef _BRD_TIMER_H
#define _BRD_TIMER_H

#include "brdDef.h"

#ifdef USE_BOARD_VE_8 
  #include  <mdr32f8_clkctrl.h>
  #include  <mdr32f8_tim.h>

  // Timer
  #define MDR_TIMER_TypeDef     MDR_TMR_TypeDef
  #define MDR_TIMER1            MDR_TMR1
  //  Clock
  #define RST_CLK_PCLKcmd       CLKCTRL_PER0_CLKcmd
  #define RST_CLK_PCLK_TIMER1   CLKCTRL_PER0_CLK_MDR_TMR1_EN
  
  //  IT Event
  #define Timer1_IRQn           TMR1_IRQn
  
#else
  #include <MDR32F9Qx_port.h>
  #include <MDR32F9Qx_rst_clk.h>
  #include <MDR32F9Qx_timer.h>
#endif


typedef struct
{
  MDR_TIMER_TypeDef*  TIMERx;
  uint32_t            ClockMask;
  uint32_t            ClockBRG;
  uint32_t            EventIT;
  IRQn_Type           IRQn;
  
} Timer_Obj;

void BRD_Timer_InitStructDef(TIMER_CntInitTypeDef *TimerInitStruct);
void BRD_Timer_Init(Timer_Obj *TimerObj, TIMER_CntInitTypeDef *TimerInitStruct);


#endif //_BRD_TIMER_H
