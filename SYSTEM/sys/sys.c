#include "sys.h"  

// THUMB instructions do not support inline assembly
// Use the following method to execute the WFI assembly instruction

void WFI_SET(void)
{
	__ASM volatile("wfi");
}
// Disable all interrupts (but does not include fault and NMI interrupts)
void INTX_DISABLE(void)
{
	__ASM volatile("cpsid i");
	__ASM volatile ("BX LR");

}
// Enable all interrupts
void INTX_ENABLE(void) {
	__ASM volatile ("CPSIE I");
	__ASM volatile ("BX LR");
}

// Set the stack top address
// addr: stack top address
void MSR_MSP(u32 addr)
{
    __ASM volatile("MSR MSP, r0");    //set Main Stack value
    __ASM volatile("BX r14");
}




