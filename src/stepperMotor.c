/* -------------------------------
    Stepper Motor API Implementation

    Version 1
   ------------------------------- */

#include <MKL25Z4.H>
#include "../include/gpio.h"
#include "../include/stepperMotor.h"
#include <stdbool.h>
#include <stdlib.h>

// local prototype
void setStep(MotorId m) ;


/*  ------------------------
    Initialse motor
		
		The motorType port and pin fields must be completed before
		  this is called.
		The clock must be enable to the GPIO port
		
		Initialise the GPIO as output and set pin mux
		Initialse step count; motor stopped
    ------------------------ */
void initMotor(MotorId m) {
	
	// Use the PT to find the PORT
	PORT_Type *pcrport ;
	if (m->port == PTA) {
		pcrport = PORTA ;
	} else if (m->port == PTB) {
		pcrport = PORTB ;
	} else if (m->port == PTC) {
		pcrport = PORTC ;
	} else if (m->port == PTD) {
		pcrport = PORTD ;
	} else {
		pcrport = PORTE ;
	}

  // Make the 4 pins GPIO
	pcrport->PCR[m->bitAp] &= ~PORT_PCR_MUX_MASK;          
	pcrport->PCR[m->bitAp] |= PORT_PCR_MUX(1);          
	pcrport->PCR[m->bitAm] &= ~PORT_PCR_MUX_MASK;          
	pcrport->PCR[m->bitAm] |= PORT_PCR_MUX(1);          
	pcrport->PCR[m->bitBp] &= ~PORT_PCR_MUX_MASK;          
	pcrport->PCR[m->bitBp] |= PORT_PCR_MUX(1);          
	pcrport->PCR[m->bitBm] &= ~PORT_PCR_MUX_MASK;          
	pcrport->PCR[m->bitBm] |= PORT_PCR_MUX(1);          
	
	// Set ports to outputs
	(m->port)->PDDR |= MASK(m->bitAp) | MASK(m->bitAm) | MASK(m->bitBp) |  MASK(m->bitBm) ;

	// Initialise other fields - motor should not move
	m->cumulSteps = 0 ;
	m->step = STEP1 ;
	m->targetSteps = 0 ;
	m->stepMode = STEPSTOPPED ;
	
	// set phases
	setStep(m) ;
}

// Do the next step: set phases for current m->step
void setStep(MotorId m) {
	GPIO_Type *PT = m->port ;
	switch (m->step) {
		case STEP1 :
	    // A==REVERSE && B==NORMAL
			PT->PSOR = MASK(m->bitAm) | MASK(m->bitBp) ;
			PT->PCOR = MASK(m->bitAp) | MASK(m->bitBm) ;
			break ;
		case STEP2 :
	    // A==REVERSE && B==REVERSE
			PT->PSOR = MASK(m->bitAm) | MASK(m->bitBm) ;
			PT->PCOR = MASK(m->bitAp) | MASK(m->bitBp) ;
		  break ;
		case STEP3 :
	    // A==NORMAL && B==REVERSE)
			PT->PSOR = MASK(m->bitAp) | MASK(m->bitBm) ;
			PT->PCOR = MASK(m->bitAm) | MASK(m->bitBp) ;
			break ;
	  case STEP4 :
	    // A==NORMAL && B==NORMAL) 
			PT->PSOR = MASK(m->bitAp) | MASK(m->bitBp) ;
			PT->PCOR = MASK(m->bitAm) | MASK(m->bitBm) ;
			break ;
	}			
}

/* ------------------------
   Stop the motor

   If update in progress then either mode not changed 
   or set to stopped
   ------------------------ */
void stopMotor(MotorId m) {
	m->stepMode = STEPSTOPPED ;
} 

/* ------------------------
   Check if motor moving

   If update in progress the result may be out of date
   ------------------------ */
bool isMoving(MotorId m) {
	return m->stepMode != STEPSTOPPED ;
} 

/* ------------------------
    moveSteps

      Move continuously if steps is zero
      Otherwise more target number of steps.

    May be called while motor running
     - set and clear skip flag

     Mode old -> New       Action
     -----------------------------------
     count --> count     add new steps
     count --> always    set direction
     always --> count    target = cumul +/- steps
     always --> always   set direction
      
   ------------------------ */
void moveSteps(MotorId m, uint16_t steps, bool clck) {

  // Block updates - update may skip
  m->skip = true ;
	
	// Going into always mode - any current mode
	if (steps == 0) {
		m->clockwise = clck ;
		m->stepMode = STEPALWAYS ;
		m->skip = false ;
		return ;
	}
	
	// Going into counting mode
	// Could be counting, always or stopped
	switch (m->stepMode) {
		case STEPCOUNT:
			if (!clck) {
				m->targetSteps += steps ;		
			} else {
				m->targetSteps -= steps ;
			}
			break ;
		case STEPSTOPPED:
		case STEPALWAYS:
			if (!clck) {
  		  m->targetSteps = m->cumulSteps + steps ;		
  		} else {
    		m->targetSteps = m->cumulSteps - steps ;
			}
			break ;
	}
	m->stepMode = STEPCOUNT ;
	m->skip = false ;
	return ;
}
		

/*  ------------------------
    Get current step count

    Counts +ve clockwise / -ve anticlockwise from zero 
    at initialisation

    If update in progress result may be out of date
    ------------------------ */
int32_t getSteps(MotorId m) {
	return m->cumulSteps ;
}

/*  ------------------------
    updateMotor state: can be called from an ISR

    Do nothing is stopped
    Otherwise
      Step in correct direction
      Incrmement step count
      If in step counting mode, stop if reached target 

    Concurreny
    ----------
      * We assume this function runs atomically
        but it may interleave with another function updating the 
        the motorType
      * skip if skip flag set: no need to set it as we run atomically
    ------------------------ */
void nextClockwise(MotorId m) {
	m->step = (m->step + 1) & 0x0003 ;
	setStep(m) ;
	m->cumulSteps++ ;
}

void nextAntiCwise(MotorId m) {
	m->step = (m->step + 3) & 0x0003 ; // 0->3, 1->0, 2->1, 3->2 
	setStep(m) ;
	m->cumulSteps-- ;
}

void updateMotor(MotorId m) {
	
	// Do nothing if stopped
	if (m->stepMode == STEPSTOPPED) return ;
	
	// Do nothing if a modification in progress
	if (m->skip) return ;

  // branch on moving modes	
	switch (m->stepMode){
		case STEPALWAYS :
			if (m->clockwise) {
				nextClockwise(m) ;
			} else {
				nextAntiCwise(m) ;
			}			
			break ;
		case STEPCOUNT :
			if (m->cumulSteps < m->targetSteps) {
				// add a step clockwise
				nextClockwise(m) ;
			} else if (m->cumulSteps > m->targetSteps) {
				// substract a step anti-clockwise
				nextAntiCwise(m) ;
			} 
  		// check if target reached 
			if (m->cumulSteps == m->targetSteps) {
				m->stepMode = STEPSTOPPED ;
			}
			break ;
	}				
}

