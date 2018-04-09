#include "brdTimer.h"

void BRD_Timer_InitStructDef(TIMER_CntInitTypeDef *TimerInitStruct)
{
  TIMER_CntStructInit (TimerInitStruct);
  TimerInitStruct->TIMER_IniCounter       = 0;                          // Начальное значение таймера
  TimerInitStruct->TIMER_Prescaler        = 0xFFFF;                     // Предделитель частоты
  TimerInitStruct->TIMER_Period           = 0x4C4;                      // Период таймера
  TimerInitStruct->TIMER_CounterMode      = TIMER_CntMode_ClkFixedDir;  // Режим счета
  TimerInitStruct->TIMER_CounterDirection = TIMER_CntDir_Up;            // Направление счета
}


void BRD_Timer_Init(Timer_Obj *TimerObj, TIMER_CntInitTypeDef *TimerInitStruct)
{
  // Включение тактирования таймера TIMER1
  RST_CLK_PCLKcmd (TimerObj->ClockMask, ENABLE);

  // Деинициализация таймера TIMER1
  TIMER_DeInit (TimerObj->TIMERx);

  // Выбор предделителя тактовой частоты таймера TIMER1
  TIMER_BRGInit (TimerObj->TIMERx, TimerObj->ClockBRG);

 // Инициализация таймера TIMER1
  TIMER_CntInit (TimerObj->TIMERx, TimerInitStruct);

  // Выбор источников прерываний
  TIMER_ITConfig (TimerObj->TIMERx, TimerObj->EventIT, ENABLE);

  // Включение таймера TIMER1
  TIMER_Cmd (TimerObj->TIMERx, ENABLE);

  // Назначение приоритета аппаратных прерываний от таймера TIMER1
  NVIC_SetPriority (TimerObj->IRQn, 1);

  // Включение аппаратных прерываний от таймера TIMER1
  NVIC_EnableIRQ (TimerObj->IRQn);
}

//// Обработчик аппаратных прерываний от таймера TIMER1
//void Timer1_IRQHandler (void)
//{
//  // Сброс прерывания от таймера TIMER1
//  TIMER_ClearITPendingBit (MDR_TIMER1, TIMER_STATUS_CNT_ARR);
//}
