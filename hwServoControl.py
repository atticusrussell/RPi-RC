from gpiozero import AngularServo
from time import sleep

from gpiozero.pins.pigpio import PiGPIOFactory

factory=PiGPIOFactory()
# these servo params are tuned for the Spektrum s605 horizon hobby servo
servo = AngularServo(18, min_angle=0, max_angle=180, min_pulse_width=0.65/1000, max_pulse_width=2.5/1000, pin_factory=factory)

while (True):
    servo.angle=0
    sleep(2)
    servo.angle=90
    sleep(2)
    servo.angle=180
    sleep(2)
