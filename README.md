# robot

## Programs
The following programs are, or will be written. They will eventually be blended 
into one or two single programs ran concurrently on two threads.

### command_receiver.ino
This program continuously receives velocity commands from the central computer.

### command_to_motor.ino
This program feeds the velocity commands received to each motor driver. It also
runs a PID loop to ensure that the correct angular velocities are being outputted.
