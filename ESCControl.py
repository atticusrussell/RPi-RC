from gpiozero import AngularServo
from time import sleep

from gpiozero.pins.pigpio import PiGPIOFactory

factory=PiGPIOFactory()
# this is created to control the ESC through PWM by treating it as a servp
ESC = AngularServo(13, min_angle=0, max_angle=100, min_pulse_width=1/1000, max_pulse_width=2/1000, pin_factory=factory)

# initialize the ESC by running the calibration routine:
ESC.angle=100 # start at full throttle
# TODO turn on the ESC here using the power button spliced into GPIO
sleep(2.2) # hold full throttle for two seconds (slight extra time for button)
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
