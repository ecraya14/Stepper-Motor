#ifndef PIT_H
#define PIT_H
#include <MKL25Z4.H>
#include "../include/stepperMotor.h"


// prototypes
void configurePIT(int channel) ;
void startTimer(int channel) ;
void stopTimer(int channel) ;
void setTimer(int channel, uint32_t timeout) ;

#endif
