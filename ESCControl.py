# interface DYNM3876 ESC with Raspberry Pi
# credit to https://elinux.org/RPi_GPIO_Interface_Circuits for help
#
# ESC power button was removed and is now controlled by GPIO 23:
# the ESC power button is has 4 wires in this order:
# A red - carries 12v when the ESC has power
# B red - if the ESC has power, no voltage until button pressed
# C white - unimportant rn
# D black  - unimportant atm
# Connecting wires A and B turns on the ESC
#
# because the wires run at 12v, a relay is used to control them with the Pi:
# at present, the relay is a JS1E-6V, (janky current setup with parts I have)
# wire A is connected to pin 3 of the relay, which is Normally Open
# wire B is connected to pin 1 of the relay, aka COM
# pin 5 of the relay, one side of the coil, is connected to 5v supply
# the other side of the coil is pin 2, and is connected to the collector of a transistor

# pins 2 and 5 are connected with a diode to theoretically isolate the pi from 
# the inductive load of the relay the cathode of the diode is connected to 
# the positive supply rail
# this diode is currently Zener diode N5817 but I would use 1N4001 if i had it
#
# the transistor is an NPN 2N3904
# its base is connected to GPIO 23 of the pi through a 1k resistor
# its collector is connectd to pin 2 of the relay
# its emitter is connected to the ground of the 5v supply

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
    sleep(2) # hold full throttle for two seconds 
    # NOTE not sure if should go all the way down or to 50 - if we should have reverse throttle?
    # should we say goes from -100 to +100? idek
    ESC.angle=50 # throttle down. should now be calibrated
    sleep(0.5) # wait a moment before anything else

    # loop infinitley ramping up throttle
    while (True):
        ESC.angle=0 # throttle off
        for i in range (15):
            ESC.angle=i
            print(i)
            sleep(0.5) # wait a half second
        sleep(1) # wait a full second before repeating the sequence

except KeyboardInterrupt: # If CTRL+C is pressed, exit cleanly:
   print("Keyboard interrupt")

except:
   print("some error") 

finally:
   print("clean up") 
   GPIO.cleanup() # cleanup all GPIO 
