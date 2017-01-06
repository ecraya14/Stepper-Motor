#include <MKL25Z4.H>
// Function prototypes for cycle timing using SysTick

void Init_SysTick(uint32_t ticksPerSec) ;
void waitSysTickCounter(int ticks) ;
