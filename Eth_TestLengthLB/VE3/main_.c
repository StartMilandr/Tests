#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>

#include "brdClock.h"
#include "brdLed.h"
#include "brdEthernet.h"
#include "brdDef.h"

#include "brdUtils.h"

//--------      WORK SETTINGS  ---------
//#define TX_PERMANENT

//--------      LED Status  ---------
#define LED_TX            BRD_LED_1
#define LED_RX            BRD_LED_2
#define LED_RX_IND_ERR    BRD_LED_3

#define LED_FRAME_PERIOD  500

int32_t LED_IndexTX = 0;
int32_t LED_IndexRX = 0;

void LED_CheckSwitch(uint32_t ledSel, int32_t * ledIndex);

//--------      ETHERNET Status  ---------
#define ETHERNET_X        MDR_ETHERNET1

uint32_t FrameIndexWR = 0;
uint32_t FrameIndexRD = 0;



int main()
{
  ETH_InitTypeDef ETH_InitStruct;
  ETH_StatusPacketReceptionTypeDef StatusRX;
  
  uint32_t doFrameSend;
  volatile uint32_t inpFrameInd;
  uint8_t *ptrDataTX;
  uint16_t dataCount;
  uint16_t i;
  uint8_t *ptr8;
  
  //  Clock 128 MHz
  BRD_Clock_Init_HSE_PLL(RST_CLK_CPU_PLLmul10);
  
  //  LEDs
  BRD_LEDs_Init();
  
  //  Ethernet
  BRD_ETH_StructInitDef(&ETH_InitStruct, MAC_SRC);
  BRD_ETH_Init(ETHERNET_X, &ETH_InitStruct); 
  BRD_ETH_Start(ETHERNET_X);
  
  //  Wait autonegotiation
  BRD_LED_Set(LED_TX | LED_RX | LED_RX_IND_ERR, 1);  
  BRD_ETH_WaitAutoneg_Completed();
  
  //Delay(10000000);
  
  //  Clear Status
  BRD_LED_Set(LED_TX | LED_RX | LED_RX_IND_ERR, 0);
  
  doFrameSend = 0;
  while(1)
  {
    //  ------------  Receive Frame -------------
    if (BRD_ETH_TryReadFrame(ETHERNET_X, &StatusRX))
    {
      ptr8 = (uint8_t*) (FrameRX) + FRAME_HEAD_SIZE;
      inpFrameInd = (uint32_t) (ptr8[0] | (ptr8[1] << 8) | (ptr8[2] << 16) | (ptr8[3] << 24));
      
      if (inpFrameInd != FrameIndexRD)
        BRD_LED_Set(LED_RX_IND_ERR, 1);
      
      //  Count Input frames
      FrameIndexRD++;
      LED_CheckSwitch(LED_RX, &LED_IndexRX);
      
      //Delay(10000);
      
      doFrameSend = 1;
    }  
    
    //  ------------  Send Frame ---------
    if (doFrameSend)
    {
      // Fill Frame
      ptrDataTX = BRD_ETH_Init_FrameTX(MAC_DEST, MAC_SRC, FRAME_LEN, &dataCount);

      ptrDataTX[0] =  FrameIndexWR & 0xFF;
      ptrDataTX[1] = (FrameIndexWR >> 8)  & 0xFF;
      ptrDataTX[2] = (FrameIndexWR >> 16) & 0xFF;
      ptrDataTX[3] = (FrameIndexWR >> 24) & 0xFF;
      
//      for (i = 4; i < dataCount; ++i)
//        {
//          ptrDataTX[i] = i;
//        }
      
      //  Check Buff_TX and out
			//while (ETH_GetFlagStatus(ETHERNET_X, ETH_MAC_FLAG_X_HALF) == SET);
			ETH_SendFrame(ETHERNET_X, (uint32_t *) FrameTX, FRAME_LEN);
            
      // Count output frames
      FrameIndexWR++;
      LED_CheckSwitch(LED_TX, &LED_IndexTX);
        
#ifndef TX_PERMANENT        
      doFrameSend = 0;
#endif
    }
  
  }  

}  

void LED_CheckSwitch(uint32_t ledSel, int32_t * ledIndex)
{
  (*ledIndex)--;
  if ((*ledIndex) < 0)
  {  
    (*ledIndex) = LED_FRAME_PERIOD;
    BRD_LED_Switch(ledSel);    
  }  
}  


