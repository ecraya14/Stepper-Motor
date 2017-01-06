#include <MKL25Z4.H>
//#include "../include/gpio.h"
//#include <stdbool.h>
//#include <stdint.h>

/*----------------------------------------------------------------------------
 * Configure SysTick to interrupt a given number times every second
 *---------------------------------------------------------------------------- */ 
void Init_SysTick(uint32_t ticksPerSec) {
   uint32_t r = 0 ; 
   r = SysTick_Config(SystemCoreClock / ticksPerSec) ;

   // Check return code for errors 
   if (r != 0) { 
       // Error Handling - program gets stuck here
       while(1); 
   }
}

/*----------------------------------------------------------------------------
 * Handle the SysTick interrupt
 *    Decrement the SysTick counter if > 0
 *---------------------------------------------------------------------------- */ 
volatile uint32_t SysTickCounter ;

void SysTick_Handler(void) {
	// Add code here to handle the SysTick counter
	if (SysTickCounter > 0) SysTickCounter -- ;
}

/*----------------------------------------------------------------------------
 * Wait using the SysTick interrupt
*    ticks: number of ms to wait
 *---------------------------------------------------------------------------- */ 
void waitSysTickCounter(int ticks) {
	while (SysTickCounter > 0) ;
	SysTickCounter = ticks ;
}
