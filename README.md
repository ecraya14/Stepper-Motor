# Stepper-Motor
Implementation of a stepper motor control, using the Programmable Interrupt Timer (PIT) to control the stepping speed precisely.

****************************************** Description of system ***************************************

Two buttons control the stepper motor.
The first button, the ‘run’ button, causes the motor to move, provided that the
motor is in the start position. 
The system has a number of moves.
On each press of the button, the motor advances to the next move, before returning to move 1.
A second button, the ‘stop button’, behaves as follows:
If the motor is moving, pressing the stop button causes it to stop.
If the motor is not moving, for example because the move has been
completed, then pressing the button causes the motor to return to its start
position, using the minimum number of steps.
The stepper speed is controlled by the PIT.
