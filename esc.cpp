/* interface DYNM3876 ESC with Raspberry Pi
 credit to https://elinux.org/RPi_GPIO_Interface_Circuits for help

 ESC power button was removed and is now controlled by GPIO 23:
 the ESC power button is has 4 wires in this order:
 A red - carries 12v when the ESC has power
 B red - if the ESC has power, no voltage until button pressed
 C white - unimportant rn
 D black  - unimportant atm
 Connecting wires A and B turns on the ESC

 because the wires run at 12v, a relay is used to control them with the Pi:
 at present, the relay is a JS1E-6V, (janky current setup with parts I have)
 wire A is connected to pin 3 of the relay, which is Normally Open
 wire B is connected to pin 1 of the relay, aka COM
 pin 5 of the relay, one side of the coil, is connected to 5v supply
 the other side of the coil is pin 2, and is connected to the collector of a transistor

 pins 2 and 5 are connected with a diode to theoretically isolate the pi from 
 the inductive load of the relay the cathode of the diode is connected to 
 the positive supply rail
 this diode is currently Zener diode N5817 but I would use 1N4001 if i had it

 the transistor is an NPN 2N3904
 its base is connected to GPIO 23 of the pi through a 1k resistor
 its collector is connectd to pin 2 of the relay
 its emitter is connected to the ground of the 5v supply */

#include <iostream>
#include <pigpio.h>
#include <unistd.h>
#include <cmath>

// RPi 4b pins to be plugged in
constexpr int ESC_POWER_PIN = 23;
constexpr int ESC_PWM_PIN = 13;

// Throttle parameters
constexpr double FULL_REV_THROTTLE = -90.0;
constexpr double NEUTRAL_THROTTLE = 0.0;
constexpr double FULL_FWD_THROTTLE = 90.0;

// Minimum throttles that will move the motor when 90 max -90 min
constexpr double MIN_FWD_THROTTLE = 10.1;
constexpr double MIN_REV_THROTTLE = -8.1;

// PWM parameters
constexpr int PWM_FREQUENCY = 50;
constexpr double MIN_PULSE_WIDTH = 1.0 / 1000.0;
constexpr double MAX_PULSE_WIDTH = 2.0 / 1000.0;
constexpr int PWM_RANGE = 10000;

void pwrOn();
void pwrOff();
void calibrate();
void start();
double fixThrottle(double throttle);
void setThrottle(double throttle);
void setThrottleRaw(double throttle);

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "Failed to initialize GPIO" << std::endl;
        return 1;
    }

    gpioSetMode(ESC_POWER_PIN, PI_OUTPUT);
    gpioWrite(ESC_POWER_PIN, 0);

    gpioSetMode(ESC_PWM_PIN, PI_OUTPUT);
    gpioSetPWMfrequency(ESC_PWM_PIN, PWM_FREQUENCY);
    gpioSetPWMrange(ESC_PWM_PIN, PWM_RANGE);

    try {
        calibrate();

        double testvector[] = {1.0, 2.0, 3.0, 0.0, -1.0, -2.0, -3.0, 0.0};
        for (double i : testvector) {
            setThrottle(i);
            sleep(2);
        }

        pwrOff();
    } catch (...) {
        std::cerr << "Error occurred" << std::endl;
    }

    setThrottle(0);
    pwrOff();
    gpioTerminate();

    return 0;
}

void pwrOn() {
    std::cout << "Powering on ESC" << std::endl;
    gpioWrite(ESC_POWER_PIN, 1);
    std::cout << "ESC on" << std::endl;
}

void pwrOff() {
    std::cout << "Powering off ESC" << std::endl;
    gpioWrite(ESC_POWER_PIN, 0);
    std::cout << "ESC off" << std::endl;
}

void calibrate() {
    std::cout << "Calibrating:" << std::endl;
    std::cout << "Setting max throttle" << std::endl;
    setThrottle(FULL_FWD_THROTTLE);
    pwrOn();
    sleep(2);
    std::cout << "Should hear two beeps" << std::endl;
    sleep(1);
    std::cout << "Setting neutral throttle" << std::endl;
    setThrottle(NEUTRAL_THROTTLE);
    std::cout << "Should hear long beep" << std::endl;
    sleep(2);
    std::cout << "ESC should be calibrated" << std::endl;
    std::cout << "Normal startup noises:" << std::endl;
    std::cout << "First beeps: 3 for 3 cell battery, 4 for 4 cell" << std::endl;
    sleep(1);
    std::cout << "Second beeps: 1 for brake on, 2 for brake off" << std::endl;
    sleep(1);
    std::cout << "ESC startup done" << std::endl;
}

void start() {
    std::cout << "ESC starting up" << std::endl;
    setThrottle(NEUTRAL_THROTTLE);
    pwrOn();
    std::cout << "Listen to the ESC beeps now" << std::endl;
    sleep(2);
    std::cout << "First beeps: 3 for 3 cell battery, 4 for 4 cell" << std::endl;
    sleep(2);
    std::cout << "Second beeps: 1 for brake on, 2 for brake off" << std::endl;
    sleep(2);
    std::cout << "ESC startup done" << std::endl;
}

double fixThrottle(double throttle) {
    if (throttle > NEUTRAL_THROTTLE) {
        throttle += MIN_FWD_THROTTLE - 1;
    } else if (throttle < NEUTRAL_THROTTLE) {
        throttle += MIN_REV_THROTTLE + 1;
    }

    if (throttle > FULL_FWD_THROTTLE) {
        throttle = FULL_FWD_THROTTLE;
    } else if (throttle < FULL_REV_THROTTLE) {
        throttle = FULL_REV_THROTTLE;
    }

    return throttle;
}

void setThrottle(double throttle) {
    double adjustThrottle = fixThrottle(throttle);
    double pulseWidth = (adjustThrottle - FULL_REV_THROTTLE) * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / (FULL_FWD_THROTTLE - FULL_REV_THROTTLE) + MIN_PULSE_WIDTH;
    int dutyCycle = std::round(pulseWidth * PWM_RANGE);
    gpioPWM(ESC_PWM_PIN, dutyCycle);
    std::cout << "Throttle: " << adjustThrottle << " / ±" << FULL_FWD_THROTTLE << std::endl;
}

void setThrottleRaw(double throttle) {
    double pulseWidth = (throttle - FULL_REV_THROTTLE) * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / (FULL_FWD_THROTTLE - FULL_REV_THROTTLE) + MIN_PULSE_WIDTH;
    int dutyCycle = std::round(pulseWidth * PWM_RANGE);
    gpioPWM(ESC_PWM_PIN, dutyCycle);
    std::cout << "Throttle: " << throttle << " / ±" << FULL_FWD_THROTTLE << std::endl;
}

