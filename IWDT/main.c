#include "brdLed.h"
#include "brdBtn.h"
#include "brdBKP.h"
#include "brdIWDT.h"
#include "brdIWDT_Select.h"
#include "brdUtils.h"

#define LED_ODD     BRD_LED_1
#define LED_EVEN    BRD_LED_2

int main(void)
{
  //  Jtag Catch protection
  Delay(5000000);
  
  BRD_BKP_Init();
  BRD_LEDs_Init();
	BRD_BTNs_Init();

  //  Led swicth
	if (BRD_BKP_ReadReg(brd_BKP_R0) == 0)
	{
		BRD_BKP_WriteReg(brd_BKP_R0, 1);
		BRD_LED_Set(LED_ODD, 1);
	}
	else
	{
		BRD_BKP_WriteReg(brd_BKP_R0, 0);
		BRD_LED_Set(LED_EVEN, 1);
	}

  //  IWDT Run
	BRD_IWDT_Init(&brdIWDT_2sec);
	
	while (1)
	{
    // Push to restart by IWDT
	  if (!BRD_Is_BntAct_Select())
		{
		  IWDG_ReloadCounter();
		}
	}		
}
