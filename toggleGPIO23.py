# code to test the ability of the pi gpio to toggle the relay

from time import sleep

import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

# initialize the GPIO pin to switch on/off the ESC
GPIO.setup(23, GPIO.OUT)
GPIO.output(23, 0) # start with the pin off

try:
    while (True):
        GPIO.output(23, 1)
        sleep(2) 
        GPIO.output(23, 0)
        sleep(2)
except KeyboardInterrupt: # If CTRL+C is pressed, exit cleanly:
   print("Keyboard interrupt")

except:
   print("some error") 

finally:
   print("clean up") 
   GPIO.cleanup() # cleanup all GPIO 
