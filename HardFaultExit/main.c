#include <stdint.h>
#include <MDR32F9Qx_config.h>

//  Если не использовать MDR32F9Qx_config.h, то это
//#include <MDR32Fx.h>
//#include <core_cm3.h>
//#include <core_cmFunc.h>
//#include <core_cmInstr.h>

//extern void HardFault_Handler_C(uint32_t stack[]);

void HardFault_TrapDivByZero(void)
{
  volatile uint32_t *confctrl = (uint32_t *) 0xE000ED14;

  *confctrl |= (1<<4);
}

uint32_t RiseDivZero(void)
{
  uint32_t b = 0;
  return 10 / b;
}

int main(void)
{
  volatile uint32_t result;
  
  HardFault_TrapDivByZero();
   
  //  Call Exception
  result = RiseDivZero();
  
  //  MainLoop
  while (1);
}

enum { r0, r1, r2, r3, r12, lr, pc, psr};

void HardFault_Handler_C(uint32_t stack[])
{
   stack[pc] = stack[pc] + 4;
  
//   printf("r0  = 0x%08x\n", stack[r0]);
//   printf("r1  = 0x%08x\n", stack[r1]);
//   printf("r2  = 0x%08x\n", stack[r2]);
//   printf("r3  = 0x%08x\n", stack[r3]);
//   printf("r12 = 0x%08x\n", stack[r12]);
//   printf("lr  = 0x%08x\n", stack[lr]);
//   printf("pc  = 0x%08x\n", stack[pc]);
//   printf("psr = 0x%08x\n", stack[psr]);
}

//----------- Не сработало --------
//register uint32_t R0         __ASM("r0");
//register uint32_t LR         __ASM("lr");

//void HardFault_Handler(void)
//{
//  __asm volatile
//  {
//    MOV r0, r1
//    
//    TST LR, #4
//    ITE EQ
//    MRSEQ r0, MSP
//    MRSNE r0, PSP
//    BL HardFault_Handler_C
//  }
//}
