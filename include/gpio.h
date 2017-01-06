#include <stdbool.h>

// Definitions for GPIO
#ifndef GPIO_H
#define GPIO_H


#define MASK(x) (1UL << (x))

// Freedom KL25Z LEDs
#define RED_LED_POS (18)		// on port B
#define GREEN_LED_POS (19)	// on port B
#define BLUE_LED_POS (1)		// on port D

// Button is on port D, pin 6
#define BUTTON_POS (6)

#define BUTTON2_POS (7)

// Outputs for stepper motor, on port E
#define MOTOR_IN1 (30) // phase A+
#define MOTOR_IN2 (29) // phase A-
#define MOTOR_IN3 (23) // phase B+
#define MOTOR_IN4 (22) // phase B-

// Function prototypes
void configureGPIOinput(void) ;       // Initialise button
void configureGPIOoutput(void) ;      // Initialise output
bool runIsPressed(void) ;
bool stopIsPressed(void);

#endif
