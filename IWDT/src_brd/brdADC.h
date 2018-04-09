#ifndef _BRD_ADC_H
#define _BRD_ADC_H

#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_adc.h>

//  General ADC Init
void BRD_ADC_PortInit(uint32_t Port_ClockMask, MDR_PORT_TypeDef* PORTx, uint32_t Port_PinsSel);
void BRD_ADCs_InitStruct(ADC_InitTypeDef  *ADCInitStruct);
void BRD_ADCs_Init(ADC_InitTypeDef  *ADCInitStruct);

//  Custom ADCx Init
void BRD_ADCx_InitStruct(ADCx_InitTypeDef *ADCxInitStruct);
void BRD_ADC1_Init(ADCx_InitTypeDef *ADCxInitStruct);
void BRD_ADC1_InitIRQ(uint32_t ADC_IT);
void BRD_ADC1_InitIRQ_EndConv(void);




#endif //_BRD_ADC_H
