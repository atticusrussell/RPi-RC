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

# set rpi 4b pins to be plugged in
ESC_POWER_PIN = 23
ESC_PWM_PIN = 13

# define params that will be needed
NO_THROTTLE = 0
FULL_THROTTLE = 100
MIN_MOV_THROTTLE = 7


# control the ESC through PWM by treating it as a servo
# currently doesn't work until set to angle 7/100
ESC = AngularServo(ESC_PWM_PIN, min_angle= NO_THROTTLE, max_angle=FULL_THROTTLE, min_pulse_width=1/1000, max_pulse_width=2/1000, pin_factory=factory)

# initialize the GPIO pin to switch on/off the ESC
GPIO.setup(ESC_POWER_PIN, GPIO.OUT)
GPIO.output(ESC_POWER_PIN, 0) # start with the pin off


# initialize the ESC by running the calibration routine:
# NOTE not sure if should go all the way down or to 50 - if we should have reverse throttle?
# should we say goes from -100 to +100? idek
def calibrateESC():
	print("calibrating:")
	ESC.angle=FULL_THROTTLE # start at full throttle
	print("powering on ESC")
	GPIO.output(ESC_POWER_PIN, 1) # turn on the ESC
	print("setting max throttle")
	sleep(2) # hold full throttle for two seconds 
	ESC.angle=NO_THROTTLE # throttle down. 
	print("setting neutral throttle ")
	sleep(2) # wait a moment before anything else
	print("ESC should be calibrated")

def normalESCStartup():
	print("ESC starting up")
	setThrottle = NO_THROTTLE
	print("Throttle:", setThrottle, "/", FULL_THROTTLE)
	ESC.angle=setThrottle
	print("powering on ESC")
	GPIO.output(ESC_POWER_PIN, 1) # turn on the ESC
	print("listen to the ESC beeps now")
	sleep(2)
	print("first beeps: 3 for 3 cell battery, 4 for 4 cell")
	sleep(2)
	print("second beeps: 1 for brake on, 2 for brake off")
	sleep(2)
	print("ESC startup done")

def cycleThrottle():
	print("no throttle")
	ESC.angle=0 # throttle off
	sleep(2)
	print("min throttle that moves")
	ESC.angle=MIN_MOV_THROTTLE
	sleep(2)
	print("cycling through some throttle values")
	for i in range (3):
		setESC = i + MIN_MOV_THROTTLE
		print("Throttle:", setESC, "/", FULL_THROTTLE)
		ESC.angle= setESC
		sleep(1) # wait a second
	setESC=0
	print("Throttle:", setESC,"/", FULL_THROTTLE)
	ESC.angle = setESC
	

def setThrottle(throttle):
	ESC.angle = throttle
	print("Throttle:", throttle, "/", FULL_THROTTLE)



# if this isn't being called from another program
if __name__ == '__main__':
	try:
		normalESCStartup()
		cycleThrottle()
		setThrottle(5)
		sleep(5)


	except KeyboardInterrupt: # If CTRL+C is pressed, exit cleanly:
		print("Keyboard interrupt")

# except:
#    print("some error")  # commented out so that I find runtime python errors

	finally:
		print("clean up") 
		GPIO.cleanup() # cleanup all GPIO 
