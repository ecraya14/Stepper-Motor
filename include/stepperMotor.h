/* -------------------------------
    Stepper Motor header file

    Version 1
   ------------------------------- */

#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H
#include <MKL25Z4.H>
#include <stdbool.h>

#define STEP1 (0) 
#define STEP2 (1) 
#define STEP3 (2) 
#define STEP4 (3) 

#define NORMAL (1) 
#define REVERSE (0) 

#define STEPSTOPPED (0) 
#define STEPALWAYS  (1)  
#define STEPCOUNT (2)  

typedef struct {
	GPIO_Type *port ;     // pointer to port
	uint16_t bitAp ;      // phase A+ bit number
	uint16_t bitAm ;      // phase A- bit number
	uint16_t bitBp ;      // phase B+ bit number
	uint16_t bitBm ;      // phase B- bit number
	uint16_t step ;       // Current position in stepping sequence
  volatile int32_t cumulSteps ;  // Cumulative steps, counting clockwise +ve
	volatile int32_t targetSteps ; // Target cumulative steps
	volatile uint16_t stepMode ;   // Mode: always, counting, stopped
	volatile bool clockwise ;      // Direction, clockwise <-> true 
	volatile bool skip ;           // provides concurrency protection
} motorType ;
// Data use
//    Direction: only if mode == always
//    Target steps: only if mode == counting 
//    Counting mode => cumul != target


// Motor is identified by pointer to its motorType
typedef motorType* MotorId ;

// Protoypes for interface functions
void initMotor(MotorId m) ;   
void moveSteps(MotorId m, uint16_t steps, bool clck) ;
   // steps == zero => continuous movement
void stopMotor(MotorId m) ;
int32_t getSteps(MotorId m) ;
bool isMoving(MotorId m) ;

// Prototype for periodic update of motor
void updateMotor(MotorId m) ;
	
#endif
