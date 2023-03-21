from gpiozero import AngularServo
from time import sleep

from gpiozero.pins.pigpio import PiGPIOFactory

factory=PiGPIOFactory()
# this is created to control the ESC through PWM by treating it as a servp
ESC = AngularServo(13, min_angle=0, max_angle=100, min_pulse_width=1/1000, max_pulse_width=2/1000, pin_factory=factory)

# initialize the ESC by running the calibration routine:
# start at full throttle, hold for 2 sec
ESC.angle(100)
# TODO turn on the ESC here using the power button
# put code here
# hold full throttle for two seconds
sleep(2)
# throttle down. should now be calibrated	
ESC.angle(0)
# wait an extra moment before trying to do anything else

while (True):
    ESC.angle=0
    sleep(2)
    ESC.angle=90
    sleep(2)
    ESC.angle=180
    sleep(2)
