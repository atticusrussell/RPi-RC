from gpiozero import Servo
from time import sleep

from gpiozero.pins.pigpio import PiGPIOFactory

factory=PiGPIOFactory()
# these servo params are tuned for the Spektrum s605 horizon hobby servo
servo = Servo(18, min_pulse_width=0.65/1000, max_pulse_width=2.5/1000, pin_factory=factory)

while (True):
    servo.min()
    sleep(2)
    servo.mid()
    sleep(2)
    servo.max()
    sleep(2)
    servo.value = None
