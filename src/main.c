/* ------------------------------------------
       Stepper Motor
	 Show red
	  On each press of the button, the motor advances to the next move(8 in total), before returning to move 1.
   Stop - on button 2 press
   If button 2 pressed when motor not moving,Run motor quickly to starting position
	using minimum number of steps. 

	 The following GPIO pins are used for the motor
	    Motor Cnnctn   Port E Pin
			-----------------------------
         IN1           pin 30       (phase A+)
         IN2           pin 29       (phase A-)
         IN3           pin 23       (phase B+)
         IN4           pin 22       (phase B-)

	 -------------------------------------------- */

#include <MKL25Z4.H>
#include "../include/gpio.h"
#include "../include/stepperMotor.h"
#include "../include/SysTick.h"
#include "../include/pit.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define STATESTART (0)
#define MOVE (1)
#define MOVECOMPLETED (2)
#define RESET (3)

#define BUTTONOPEN (0)
#define BUTTONCLOSED (1)
#define BUTTONBOUNCE (2)

int sys_state = STATESTART ;
/*----------------------------------------------------------------------------
  Motor Configuration

 *----------------------------------------------------------------------------*/
motorType mcb ;   // motor control block
MotorId m1 ;      // motor id
void configureMotor() {
	m1 = & mcb ;
	m1->port = PTE ;
  m1->bitAp = MOTOR_IN1 ;
  m1->bitAm = MOTOR_IN2 ;
  m1->bitBp = MOTOR_IN3 ;
  m1->bitBm = MOTOR_IN4 ;

	// Enable clock to port E
	SIM->SCGC5 |=  SIM_SCGC5_PORTE_MASK; /* enable clock for port E */
	
	// Initialise motor data and set to state 1
  initMotor(m1) ; // motor initially stopped, with step 1 powered
}

/*----------------------------------------------------------------------------
  Poll the input

 Detect changes in the switch state.
    runIsPressed (or stopIsPressed) and not closed --> new press;
     ~isPressed (or stopIsPressed) and closed -> not closed
*----------------------------------------------------------------------------*/
int b_state = BUTTONOPEN ;
int runPressed = 0 ;
int stopPressed = 0 ;
int bounceCounter = 0 ;

void buttonTask(void) {
    
    if (bounceCounter > 0) bounceCounter -- ;
    
    switch (b_state) {
        case BUTTONOPEN :
            if (runIsPressed()) {
                runPressed = 1 ;  // create a 'pressed' event
                b_state = BUTTONCLOSED ;
            }
            else if (stopIsPressed()) {
                stopPressed = 1;
                b_state = BUTTONCLOSED ;
            }
            break ;
        case BUTTONCLOSED :
            if (!runIsPressed() || !stopIsPressed()) {
		runPressed = 0;
                b_state = BUTTONBOUNCE ;
                bounceCounter = 50 ;
            }
            break ;
        case BUTTONBOUNCE :
            if (runIsPressed() || stopIsPressed()) {
                b_state = BUTTONCLOSED ;
            }
            if (bounceCounter == 0) {
                b_state = BUTTONOPEN ;
            }
            break ;
    }
}

bool motorRunning = false ;
volatile int movePosition = 0;
volatile uint32_t totalSteps = 0;
volatile uint32_t loadValue = 0;

void controlMotor(void) {
    int steps;
		int pos;
    switch (sys_state) {
        case STATESTART:
            if(runPressed) {
                sys_state = MOVE;
                PTB->PSOR |= MASK(RED_LED_POS) ;
            } //if stop button pressed, do nothing
            break;
        case MOVE:
            movePosition++; //Respond to button press, new move
            runPressed = false; //acknowledge
            switch (movePosition) {
                case 1:
                    loadValue = ceil((0.3125*12000000)-1); //round up
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 64, true);
                    sys_state = MOVECOMPLETED;
                    break;
                case 2:
                    loadValue = ceil((0.07352941*12000000)-1);
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 272, true);
                    sys_state = MOVECOMPLETED;
                    break;
                case 3:
                    loadValue = ceil((0.04166667*12000000)-1);
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 480, false);
                    sys_state = MOVECOMPLETED;
                    break;
                case 4:
                    loadValue = ceil((0.0204918*12000000)-1);
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 976, false);
                    sys_state = MOVECOMPLETED;
                    break;
                case 5:
                    loadValue = ceil((0.01953125*12000000)-1);
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 512, false);
                    sys_state = MOVECOMPLETED;
                    break;
                case 6:
                    loadValue = ceil((0.01041667*12000000)-1);
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 960, false);
                    sys_state = MOVECOMPLETED;
                    break;
                case 7:
                    loadValue = ceil((0.00686813*12000000)-1);
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 1456, true);
                    sys_state = MOVECOMPLETED;
                    break;
                case 8:
                    loadValue = ceil((0.00512295*12000000)-1);
                    setTimer(0, loadValue);
                    startTimer(0);
                    moveSteps(m1, 1952, true);
                    sys_state = MOVECOMPLETED;
                    movePosition = 0; //restart move position
                    break;
            }
            
            
        case MOVECOMPLETED:
            if (runPressed) {
		PTB->PCOR |= MASK(GREEN_LED_POS) ;
                if(!isMoving(m1)) {
                    sys_state = MOVE;
                } else { //the motor is still running
                    stopMotor(m1);
                    totalSteps += abs(getSteps(m1));
			stopTimer(0);
                    sys_state = MOVE;
                }
                break;
            } else if (stopPressed) { //the motor is still running, case for stop
                stopPressed = false ; // acknowledge
                if (!isMoving(m1)) {
                    steps = getSteps(m1) ;
                    sys_state = RESET;
                } else {
                    stopMotor(m1) ;
                    steps = getSteps(m1);
                    totalSteps += abs(steps);
                    sys_state = MOVECOMPLETED;
                }
            }
            break;
        
        case RESET:
            //use steps to find way back to start
            stopTimer(0);
            
            if (steps < 0) { //anticlockwise
                totalSteps += abs(steps);
                pos = totalSteps % 48;
                if(pos > 24) { //half, motor in bottom position
                    moveSteps(m1, pos, false);
                } else {
                    moveSteps(m1, pos, true);
                }
            } else { //clockwise
                totalSteps += steps;
                pos = totalSteps % 48;
                if (pos > 24) {
                    moveSteps(m1, pos, true);
                } else {
                    moveSteps(m1, pos, false);
                }
            }
            //how fast?? last used? or a fast one?
            //using move no 6 value
            loadValue = ceil((0.01041667*12000000)-1);
            setTimer(0, loadValue);
            startTimer(0);
            sys_state = STATESTART;
            break;
        
        
        default:
            break;
    }
}



/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/

int main (void) {
	configureGPIOoutput() ;
	configureGPIOinput() ;
	configureMotor() ;
	configurePIT(0) ;

	Init_SysTick(1000) ; // SysTick every ms
  
    waitSysTickCounter(10) ; // initialise counter
	PTB->PCOR |= MASK(RED_LED_POS) ;
	while (1) {		

        buttonTask();
        controlMotor();
        waitSysTickCounter(10) ; // cycle every 10ms
	}
}

