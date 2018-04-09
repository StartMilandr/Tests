//#include "brdClock.h"
#include "brdLed.h"
#include "brdBtn.h"
#include "brdADC.h"
#include "brdADC_Select.h"
#include "brdUtils.h"
#include "brdLCD.h"

#define ADC_LED_PERIOD    10000

#define ADC_PRESC_CLR_MASK     (~ADC_CLK_div_32768)
#define ADC_PRESC_COUNT         15

uint32_t ADC_Presc_Value[ADC_PRESC_COUNT] = {
  ADC_CLK_div_2 ,   ADC_CLK_div_4,    ADC_CLK_div_8,    ADC_CLK_div_16 ,   ADC_CLK_div_32,
  ADC_CLK_div_64,   ADC_CLK_div_128,  ADC_CLK_div_256,  ADC_CLK_div_512,   ADC_CLK_div_1024,
  ADC_CLK_div_2048, ADC_CLK_div_4096, ADC_CLK_div_8192, ADC_CLK_div_16384, ADC_CLK_div_32768
  };

void ADC1_Init_withPresc(uint32_t prescIndex);
void ADC1_ChangePresc(uint32_t prescIndex);
void LCD_ShowDelay(uint32_t presc, uint32_t delay);

int main(void)
{
  volatile uint32_t ADC_Value;
  volatile uint32_t ADC_WaitCnt;  
  uint32_t ADC_Count;
  
  uint32_t ADC_PrecInd   = 0;
  uint32_t ADC_LedPeriod = ADC_LED_PERIOD;

  //  Jtag Catch protection
  Delay(5000000);
  
  //  CPU Clock
  //BRD_Clock_Init_HSE_PLL(10);
  
  //  Led and Buttons Init
  BRD_LEDs_Init();
  BRD_BTNs_Init();
  BRD_LCD_Init();
      
  //  ADCs Init
  ADC1_Init_withPresc(ADC_PrecInd);
	
	while (1)
	{   
    ADC_WaitCnt = 0;
        
    MDR_ADC->ADC1_CFG |= ADC1_CFG_REG_GO;
    while (!(MDR_ADC->ADC1_STATUS & ADC1_FLAG_END_OF_CONVERSION))
    {
      ADC_WaitCnt++;
    };
    
    ADC_Value = ADC1_GetResult() & 0xFFF;
    ADC_Count++;
    
    //  Led Switch
    if (ADC_Count > ADC_LedPeriod)
    {
      ADC_Count = 0;
      BRD_LED_Switch(BRD_LED_1);
      
      LCD_ShowDelay(ADC_PrecInd, ADC_WaitCnt);
    }

    // Inc Prec
    if (BRD_Is_BntAct_Up())
    {
      while (BRD_Is_BntAct_Up());
      
      // Set New prescaller
      ++ADC_PrecInd;
      if (ADC_PrecInd >= ADC_PRESC_COUNT)
        ADC_PrecInd = 0;
      
      ADC_LedPeriod = ADC_LED_PERIOD >> ADC_PrecInd;
      ADC1_ChangePresc(ADC_PrecInd);
    }
    
    // Decc Prec
    if (BRD_Is_BntAct_Down())
    {
      while (BRD_Is_BntAct_Down());
      
      // Set New prescaller
      if (ADC_PrecInd == 0)
        ADC_PrecInd = ADC_PRESC_COUNT - 1;
      else
        --ADC_PrecInd;
      
      ADC_LedPeriod = ADC_LED_PERIOD >> ADC_PrecInd;
      ADC1_ChangePresc(ADC_PrecInd);
    }    
    
	}
}

void ADC1_ChangePresc(uint32_t prescIndex)
{
  
  uint32_t regCFG1 = MDR_ADC->ADC1_CFG;
  regCFG1 &= ADC_PRESC_CLR_MASK;
  MDR_ADC->ADC1_CFG = regCFG1 | ADC_Presc_Value[prescIndex];   
}

void ADC1_Init_withPresc(uint32_t prescIndex)
{
  ADC_InitTypeDef  ADCInitStruct;
  ADCx_InitTypeDef ADCxInitStruct;  

  BRD_ADCs_InitStruct(&ADCInitStruct);
  BRD_ADCs_Init(&ADCInitStruct);
  
  //  ADC1 Init
  BRD_ADCx_InitStruct(&ADCxInitStruct);

  ADCxInitStruct.ADC_Prescaler = ADC_Presc_Value[prescIndex];   
  BRD_ADC1_Init(&ADCxInitStruct);  
}

void LCD_ShowDelay(uint32_t presc, uint32_t delay)
{
  static char message[64];
  
  sprintf(message , "PSC=%d  T=%d ", presc, delay);
  LCD_PutString (message, 3);
}


