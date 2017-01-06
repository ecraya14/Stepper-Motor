#include <MKL25Z4.H>
#include "../include/pit.h"
#include "../include/gpio.h"  // only needed for demo application
#include "../include/stepperMotor.h"

extern MotorId m1 ; 
/* -------------------------------------
    Configure a timer channel with interrupts
   ------------------------------------- */
void configurePIT(int channel) {
  // enable clock to PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK ;
	
	// Set MDIS = 0 module enabled
	//     FRZ = 0, timers run in debug
	PIT->MCR &= ~PIT_MCR_MDIS_MASK ;
	
	// Clear any outstanding interrupts
	// Set TIE = 1 to enable interupts
	// Do not start timer
	PIT->CHANNEL[channel].TFLG = PIT_TFLG_TIF_MASK ;
	PIT->CHANNEL[channel].TCTRL |= PIT_TCTRL_TIE_MASK ;	
	
	//
		/* Enable Interrupts */
	NVIC_SetPriority(PIT_IRQn, 128); // 0, 64, 128 or 192
	NVIC_ClearPendingIRQ(PIT_IRQn);  // clear any pending interrupts
	NVIC_EnableIRQ(PIT_IRQn);
}

/* -------------------------------------
    Start the timer on the given channel
   ------------------------------------- */
void startTimer(int channel) {
	PIT->CHANNEL[channel].TCTRL |= PIT_TCTRL_TEN_MASK ;	
}

/* -------------------------------------
    Stop the timer on the given channel
   ------------------------------------- */
void stopTimer(int channel) {
	PIT->CHANNEL[channel].TCTRL &= ~PIT_TCTRL_TEN_MASK ;	
}

/* -------------------------------------
   Set the timer value
     If the timer is running, the new value is used
     after the next timeout

   Unit are number of cycle of 24MHz clock
   ------------------------------------- */
void setTimer(int channel, uint32_t timeout) {
	PIT->CHANNEL[channel].LDVAL = timeout ;
}

/* -------------------------------------
    Timer interrupt handler

    Check each channel to see if caused interrupt
    Write 1 to TIF to reset interrupt flag
   ------------------------------------- */
void PIT_IRQHandler(void) {
	// clear pending interrupts
	NVIC_ClearPendingIRQ(PIT_IRQn);

	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
		// clear TIF
		PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK ;
		
		//channel 0 interrupt
        //causes the motor to step if it has outstanding steps,
        //if no outstanding steps, then nothing happens, harmless. 
        updateMotor(m1);
        
		
	}

	if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
		// clear TIF
		PIT->CHANNEL[1].TFLG = PIT_TFLG_TIF_MASK ;

		// add code here for channel 1 interrupt
		
	}
}
