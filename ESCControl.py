# this code is to initialize and calibrate an ESC interfacing with the Pi

# ESC power button was removed and is now controlled by GPIO 23 
# the two red wires from the power button run at 12v and are connected by a 
# relay, which is driven from GPIO 23 of the pi using an NPN transistor, whose
# gate is connected to through a 1k resistor
 
from gpiozero import AngularServo
from time import sleep

import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)

from gpiozero.pins.pigpio import PiGPIOFactory
factory=PiGPIOFactory()

try:
    # control the ESC through PWM by treating it as a servp
    ESC = AngularServo(13, min_angle=0, max_angle=100, min_pulse_width=1/1000, max_pulse_width=2/1000, pin_factory=factory)
    # initialize the GPIO pin to switch on/off the ESC
    GPIO.setup(23, GPIO.OUT)
    GPIO.output(23, 0) # start with the pin off

    # initialize the ESC by running the calibration routine:
    ESC.angle=100 # start at full throttle
    GPIO.output(23, 1) # turn on the ESC
    sleep(2) # hold full throttle for two seconds (slight extra time for button)
    # NOTE not sure if should go all the way down or to 50 - if we should have reverse throttle?
    # should we say goes from -100 to +100? idek
    ESC.angle=0 # throttle down. should now be calibrated
    sleep(0.5) # wait a moment before anything else

    # loop infinitley ramping up throttle
    while (True):
        ESC.angle=0 # throttle off
        for i in range (15):
            ESC.angle=i
            print(i)
            sleep(0.5) # wait a half second
        sleep(1) # wait a full second 

except KeyboardInterrupt: # If CTRL+C is pressed, exit cleanly:
   print("Keyboard interrupt")

except:
   print("some error") 

finally:
   print("clean up") 
   GPIO.cleanup() # cleanup all GPIO 
