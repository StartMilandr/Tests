#include "brdDef.h"
#include "brdClock.h"
#include "brdLed.h"
#include "brdEthernet.h"
#include "brdUtils.h"
#include "brdBtn.h"
#include "brdBKP.h"

//--------      Параметры подключения - не активировать оба сразу  ---------

// Ожидание Autonegotiation или Link в линии при установке связи
//#define USE_AUTONEG

// Тестирование обмена в одном контроллере, без пары (режим КЗ в блоке МАС, Phy не используется)
#define USE_LOOP_BACK

#ifdef USE_LOOP_BACK  
  #define LOOP_BACK_SELECT    ENABLE;
#else
  #define LOOP_BACK_SELECT    DISABLE;
#endif 

//--------      SELECT PAIR  ---------
//  IS_STARTER  - Должен быть только один в паре - он запускает обмен
//  LINK_TO_VE8 - Тот в паре, кто подключен к 1986ВЕ8Т - пропускается ожидание Autonegotiation или Link в линии

#if   defined (USE_MDR1986VE1T)
  #define ETHERNET_X        MDR_ETHERNET1
  #define PLL_MUX           RST_CLK_CPU_PLLmul16 // Clock Max 128 MHz 

  #define IS_STARTER
  //#define LINK_TO_VE8
  
#elif defined (USE_MDR1986VE3)
  #define ETHERNET_X        MDR_ETHERNET1
  #define PLL_MUX           RST_CLK_CPU_PLLmul10 // Clock Max 80 MHz 
  
  #define USE_VE3_BUG_PTR_FIX   //  Выравнивание указателей позволяет правильно принимать след. пакет! Для линейного и FIFO, мешает  в Auto 

  //#define IS_STARTER
  //#define LINK_TO_VE8      
  
#elif defined (USE_BOARD_VE_8)
  #define ETHERNET_X        MDR_ETH0  
  #define PLL_MUX           10       // Clock Max 100 MHz   
  
  //#define IS_STARTER  
#endif

#if defined (USE_LOOP_BACK) && !defined (IS_STARTER)
  #define IS_STARTER            //  В режиме КЗ - сам себе стартер
#endif


#ifdef LINK_TO_VE8
  #define PHY_MODE_SELECT     ETH_PHY_MODE_10BaseT_Full_Duplex;    // VE8 не умеет быстрее
#elif defined (USE_AUTONEG)
  #define PHY_MODE_SELECT     ETH_PHY_MODE_AutoNegotiation;
#else
  #define PHY_MODE_SELECT     ETH_PHY_MODE_100BaseT_Full_Duplex;
#endif

uint32_t doFrameSend;

//--------      ETHERNET SETTINGS  ---------
uint8_t  MAC1[] = {0x1c, 0x1b, 0x0d, 0x49, 0xe2, 0x14};
uint8_t  MAC2[] = {0x1c, 0x34, 0x56, 0x78, 0x9a, 0xbc};

#ifdef IS_STARTER
  #define MAC_DEST    MAC1
  #define MAC_SRC     MAC2
#else
  #define MAC_DEST    MAC2
  #define MAC_SRC     MAC1
#endif

#define FRAME_LEN_MIN   64
#define FRAME_LEN_MAX   1514

#define REPEAT_FRAME_LEN_COUNT 5

uint32_t FrameLen = FRAME_LEN_MIN;
uint32_t RepeatFrameLenTX;

#define  BUFF_MODE_COUNT   3
uint32_t BuffModes[BUFF_MODE_COUNT]   = {ETH_BUFFER_MODE_LINEAR, ETH_BUFFER_MODE_AUTOMATIC_CHANGE_POINTERS, ETH_BUFFER_MODE_FIFO};
uint32_t BuffModeLED[BUFF_MODE_COUNT] = {BRD_LED_1, BRD_LED_2, BRD_LED_3};
uint32_t BuffModeIndex = 0;

uint32_t GetNextBuffModeInd(uint32_t buffModeInd);
void ValidateBuffModeInd(uint32_t *buffModeInd);

void ETH_Init_Run(uint32_t buffMode);

//--------      LED Status  ---------
#define LED_TX            BRD_LED_1
#define LED_RX            BRD_LED_2
#define LED_RX_IND_ERR    BRD_LED_3
#define LED_LEN_ERR       BRD_LED_4
#define LED_DATA_ERR      BRD_LED_5
#define LED_PTR_ERR       BRD_LED_6

#define LED_ALL_ERR (LED_RX_IND_ERR | LED_LEN_ERR | LED_DATA_ERR | LED_PTR_ERR)
#define LED_ALL     (LED_TX | LED_RX | LED_ALL_ERR)


#define LED_FRAME_PERIOD  1000

int32_t LED_IndexTX;
int32_t LED_IndexRX;

void LED_CheckSwitch(uint32_t ledSel, int32_t * ledIndex);

//--------      ETHERNET Status  ---------
ETH_InitTypeDef ETH_InitStruct;

uint32_t FrameIndexTX;
uint32_t FrameIndexRD;

uint8_t *ptrDataTX;

uint32_t CheckBuffPTR(void);
uint32_t CheckFrameData(void);
uint32_t CheckFrameIndex(uint32_t frameIndTX);
void FillFrameTXData(uint32_t frameIndex, uint32_t frameLength);


uint32_t Process_ModeSelect(void);

int main()
{
  ETH_StatusPacketReceptionTypeDef StatusRX;
  
  //        CLOCK
  BRD_Clock_Init_HSE_PLL(PLL_MUX);
  
  //        LEDs and BTNs
  BRD_LEDs_Init();
  BRD_BTNs_Init();
  
  //  BKP
  BRD_BKP_Init();
  BuffModeIndex = BRD_BKP_ReadReg(brd_BKP_R0);
  ValidateBuffModeInd(&BuffModeIndex);
  //  Show active BuffMode and wait start
  BRD_LED_Set(LED_ALL, 0);
  BRD_LED_Set(BuffModeLED[BuffModeIndex], 1);
  while (!Process_ModeSelect());

  //        ETHERNET
  BRD_ETH_ClockInit();

  //  Настройки и переопределение
  BRD_ETH_StructInitDef(&ETH_InitStruct, MAC_SRC);
  ETH_InitStruct.ETH_Loopback_Mode = LOOP_BACK_SELECT;
#ifndef USE_BOARD_VE_8
  ETH_InitStruct.ETH_PHY_Mode = PHY_MODE_SELECT;  
#endif

  //  Инициализация и запуск Ethernet - для ВЕ8 работает только вручную
  ETH_Init_Run(BuffModes[BuffModeIndex]);
  //  для отладки
  //ETH_Init_Run(ETH_BUFFER_MODE_LINEAR);
  //ETH_Init_Run(ETH_BUFFER_MODE_AUTOMATIC_CHANGE_POINTERS);
  //ETH_Init_Run(ETH_BUFFER_MODE_FIFO);

  while(1)
  { 
    //  ------------  Стирание LED ошибок -------------
    if (BRD_Is_BntAct_Right())
    {
      while (BRD_Is_BntAct_Right()){};    
      BRD_LED_Set(LED_ALL_ERR, 0);
    };
        
    //  ------------  Receive Frame -------------
    if (BRD_ETH_TryReadFrame(ETHERNET_X, &StatusRX))
    { 
      //  Проверка длины пакета
      if (StatusRX.Fields.Length != (FrameLen + FRAME_CRC_SIZE))
        BRD_LED_Set(LED_LEN_ERR, 1);

      //  Проверка индекса фрейма
      if (!CheckFrameIndex(FrameIndexTX - 1))
        BRD_LED_Set(LED_RX_IND_ERR, 1);
        
      //  Проверка данных фрейма
      if (CheckFrameData())
        BRD_LED_Set(LED_DATA_ERR, 1);

      //  Проверка указателей Head и Tail
      if (CheckBuffPTR())
      {  
        //  Увеличиваем длину фрейма через REPEAT_FRAME_LEN_COUNT посылок
        RepeatFrameLenTX++;
        if (RepeatFrameLenTX >= REPEAT_FRAME_LEN_COUNT)
        {  
          RepeatFrameLenTX = 0;
          
          FrameLen++;
          if (FrameLen > FRAME_LEN_MAX)
            FrameLen = FRAME_LEN_MIN;
        }
      }
      else
        BRD_LED_Set(LED_PTR_ERR, 1);
      
      // Индикация приема
      LED_CheckSwitch(LED_RX, &LED_IndexRX);
        
  
      //  Считаем принятые фреймы и переключаем светодиод
      FrameIndexRD++;
  
      //  Сброс статуса
      ETHERNET_X->ETH_STAT = 0;
      
      //  Запуск передачи следующего фрейма
      doFrameSend = 1;
    }  
    
    //  ------------  Send Frame ---------
    if (doFrameSend)
    {
      FillFrameTXData(FrameIndexTX, FrameLen);

      //  Check Buff_TX and out
			//while (ETH_GetFlagStatus(ETHERNET_X, ETH_MAC_FLAG_X_HALF) == SET); // не требуется, т.к. передача по 1 фрейму.
			ETH_SendFrame(ETHERNET_X, (uint32_t *) FrameTX, FrameLen);
     
      //  Считаем отправленные фреймы
      FrameIndexTX++;
      LED_CheckSwitch(LED_TX, &LED_IndexTX);
        
      //  Выкл. отправки фреймов
      doFrameSend = 0;
    }  
  }
}  

//  Переключение светодиодов по периоду
void LED_CheckSwitch(uint32_t ledSel, int32_t * ledIndex)
{
  (*ledIndex)--;
  if ((*ledIndex) < 0)
  {  
    (*ledIndex) = LED_FRAME_PERIOD;
    BRD_LED_Switch(ledSel);    
  }  
}

void ETH_Init_Run(uint32_t buffMode)
{
  //  Режим работы буферов
  ETH_InitStruct.ETH_Buffer_Mode = buffMode;
 
  BRD_ETH_Init(ETHERNET_X, &ETH_InitStruct); 
  // BRD_ETH_InitIRQ(ETHERNET_X, ETH_MAC_IT_MISSED_F);  // Непрерывная генерация прерывания в 1986ВЕ1Т rev4. В rev6 не наблюдается
  BRD_ETH_Start(ETHERNET_X);

  // Show Wait Status
  BRD_LED_Set(LED_ALL, 1);
  
  //  Ожидание установления соединения, кроме ВЕ8Т
#if !defined(LINK_TO_VE8) && !defined(USE_LOOP_BACK)
  //  Wait
  #ifdef USE_AUTONEG  
    BRD_ETH_WaitAutoneg_Completed(ETHERNET_X);
  #else  
    BRD_ETH_WaitLink(ETHERNET_X);
  #endif
  
#endif

  Delay(10000000); // Wait for Led

  //  Сброс LED статуса
  BRD_LED_Set(LED_ALL, 0);
  //  Обнуление счетчиков фреймов
  LED_IndexTX = 0;
  LED_IndexRX = 0;
  FrameIndexTX = 0;
  FrameIndexRD = 0;
  RepeatFrameLenTX = 0;
  
  FrameLen = FRAME_LEN_MIN;

#ifdef IS_STARTER
  doFrameSend = 1;    //  Стартер - Запускает обмен: Посылает фрейм к "Приемнику" и ждет от него ответный  
#else
  doFrameSend = 0;    //  Приемник - Отвечает: Ждет фрейм от "Стартера" и посылается ему ответный
#endif
}

uint32_t Process_ModeSelect(void)
{
  uint32_t completed = 0;
  
  //  Select Buff Mode by Inc
  if (BRD_Is_BntAct_Left())
  {
    while (BRD_Is_BntAct_Left()){};

    BuffModeIndex = GetNextBuffModeInd(BuffModeIndex);
    BRD_LED_Set(LED_ALL, 0);
    BRD_LED_Set(BuffModeLED[BuffModeIndex], 1);
  };
  
  //  Apply Buff Mode and Reset
  if (BRD_Is_BntAct_Up())
  {
    while (BRD_Is_BntAct_Up()){};
      
    BRD_BKP_WriteReg(brd_BKP_R0, BuffModeIndex);
    completed = 1;
  };       

  return completed; 
}  

uint32_t CheckFrameIndex(uint32_t frameIndTX)
{
  uint8_t *ptr8;  
  uint32_t frameIndRX;
  
  ptr8 = (uint8_t*) (FrameRX) + FRAME_HEAD_SIZE;
  frameIndRX = (uint32_t) (ptr8[0] | (ptr8[1] << 8) | (ptr8[2] << 16) | (ptr8[3] << 24));
  return frameIndRX == frameIndTX;
}  

uint32_t CheckFrameData(void)
{
  uint16_t dataCount;
  uint32_t errDataCount;
  uint16_t i;
  uint8_t *ptr8;
  
  //  Проверка данных фрейма
  ptr8 = (uint8_t*) (FrameRX) + FRAME_HEAD_SIZE - FRAME_LEN_SIZE;
  dataCount = (uint32_t) (ptr8[1]);

  ptr8 = (uint8_t*) (FrameRX) + FRAME_HEAD_SIZE;    
  errDataCount = 0;
  for (i = 0; i < dataCount; ++i)
  {
    if (ptr8[i] != ptrDataTX[i])
      errDataCount++;
  }
  
  return errDataCount;
}

void FillFrameTXData(uint32_t frameIndex, uint32_t frameLength)
{
  uint16_t dataCount;
  uint16_t i;
  
  // Запись управляющего слова, МАС и Length фрейма
  ptrDataTX = BRD_ETH_Init_FrameTX(MAC_DEST, MAC_SRC, frameLength, &dataCount);
  
  //  Запись индекса фрейма
  ptrDataTX[0] =  frameIndex & 0xFF;
  ptrDataTX[1] = (frameIndex >> 8)  & 0xFF;
  ptrDataTX[2] = (frameIndex >> 16) & 0xFF;
  ptrDataTX[3] = (frameIndex >> 24) & 0xFF;
  
  //  Запись данных для отправки
  for (i = 4; i < dataCount; ++i)
  {
    ptrDataTX[i] = i ;
  }
  // Маркер конца пакета
  ptrDataTX[dataCount - 2] = 0xFF;
  ptrDataTX[dataCount - 1] = frameLength;
}

uint32_t CheckBuffPTR(void)
{
  uint32_t ErrCnt = 0;
  
  if (ETHERNET_X->ETH_R_Head != ETHERNET_X->ETH_R_Tail)
  {
#ifdef USE_VE3_BUG_PTR_FIX
    ETHERNET_X->ETH_R_Head = ETHERNET_X->ETH_R_Tail;
#endif    
    ErrCnt++;
  }
  if (ETHERNET_X->ETH_X_Head != ETHERNET_X->ETH_X_Tail)
  {
    //    ETHERNET_X->ETH_X_Tail = ETHERNET_X->ETH_X_Head; // Перекрывается обработкой ETH_R_Head 
    ErrCnt++;
  }
  
  return !ErrCnt;
}  

void ValidateBuffModeInd(uint32_t *buffModeInd)
{
  if (*buffModeInd >= BUFF_MODE_COUNT)
    *buffModeInd = 0;
}

uint32_t GetNextBuffModeInd(uint32_t buffModeInd)
{
  buffModeInd++;
  if (buffModeInd >= BUFF_MODE_COUNT)
    buffModeInd = 0;
  
  return buffModeInd;
}

