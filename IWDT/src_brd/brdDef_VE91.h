#ifndef _BRD_DEF_VE91
#define _BRD_DEF_VE91

#ifdef USE_BOARD_VE_91

#include "brdDef_VE9x.h"

//  Buttons Definition
    #define BRD_BTN_PORT_SEL       MDR_PORTC
    #define BRD_BTN_PIN_SEL        PORT_Pin_10

    #define BRD_BTN_PORT_UP        MDR_PORTC
    #define BRD_BTN_PIN_UP         PORT_Pin_11
	
    #define BTN_PORT_RIGHT         MDR_PORTC
    #define BTN_BTN_PIN_RIGHT      PORT_Pin_14
	
    #define BTN_PORT_DOWN          MDR_PORTC
    #define BTN_BTN_PIN_DOWN       PORT_Pin_12
	
    #define BTN_PORT_LEFT          MDR_PORTC
    #define BTN_BTN_PIN_LEFT       PORT_Pin_13

    // for Initialization
    #define BRD_BTNs_PORT_CLK      RST_CLK_PCLK_PORTC
    #define BRD_BTNs_PORT_MASK     MDR_PORTC
    #define BRD_BTNs_PIN_MASK      BRD_BTN_PIN_SEL | BRD_BTN_PIN_UP | BTN_BTN_PIN_RIGHT | BTN_BTN_PIN_DOWN | BTN_BTN_PIN_LEFT

    //  for Is_BtnAct_...
    #define BRD_BTNs_PUSH_TO_GND 

//  LEDs Definition
    #define BRD_LED_1 	           PORT_Pin_10
    #define BRD_LED_2 	           PORT_Pin_11
    #define BRD_LED_3 	           PORT_Pin_12
    #define BRD_LED_4 	           PORT_Pin_13

    #define BRD_LED_PORT_CLK       RST_CLK_PCLK_PORTD
    #define BRD_LED_PORT           MDR_PORTD    
    #define BRD_LED_Pins           BRD_LED_1 | BRD_LED_2 | BRD_LED_3 | BRD_LED_4
    

#else
   Please, select board in brdSelect.h!

#endif 

#endif // _BRD_DEF_VE91
