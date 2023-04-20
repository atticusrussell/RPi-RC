#include <pigpio.h>
#include <iostream>
// #include <cstdlib>
// #include <functional>
#include <unistd.h>
#include "esc.hpp"
// #include "angular_servo.hpp"

using namespace std;


ESC::ESC(int pwmPin, int fullRevThrottle, int fullFwdThrottle, int minPulseWidthUs, int maxPulseWidthUs, int powerPin,  int neutralThrottle, float minFwdThrottle, float minRevThrottle) : AngularServo(pwmPin, fullRevThrottle, fullFwdThrottle, minPulseWidthUs, maxPulseWidthUs) {
    __powerPin = powerPin;
    __neutralThrottle = neutralThrottle;
    __minFwdThrottle = minFwdThrottle;
    __minRevThrottle = minRevThrottle;
    int rc = gpioSetMode(__powerPin, PI_OUTPUT);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_MODE) {
        throw "Inalid GPIO mode Error!";
    }
    // turn off ESC to start
    rc = gpioWrite(__powerPin, 0);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_LEVEL) {
        throw "Inalid GPIO level Error!";
    }
}

void ESC::turnOn() {
    cout << "Powering on ESC" << endl;
    int rc = gpioWrite(__powerPin, 1);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_LEVEL) {
        throw "Inalid GPIO level Error!";
    }
    cout << "ESC on" << endl;
}

void ESC::turnOff() {
    cout << "Powering off ESC" << endl;
    int rc = gpioWrite(__powerPin, 0);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_LEVEL) {
        throw "Inalid GPIO level Error!";
    }
    cout << "ESC off" << endl;
}

void ESC::setThrottleRaw(double throttle) {
    setAngle(throttle);
    cout << "Throttle: " << throttle << " / ±" << __maxAngle << endl;
}

double ESC::fixThrottle(double throttle) {
    if (throttle > __neutralThrottle) {
        throttle += __minFwdThrottle - 1;
    } else if (throttle < __neutralThrottle) {
        throttle += __minRevThrottle + 1;
    }

    if (throttle > __maxAngle) {
        throttle = __maxAngle;
    } else if (throttle < __minAngle) {
        throttle = __minAngle;
    }

    return throttle;
}

void ESC::setThrottle(double throttle) {
    throttle = fixThrottle(throttle);
    setAngle(throttle);
    cout << "Throttle: " << throttle << " / ±" << __maxAngle << endl;
}


void ESC::calibrate() {
    cout << "Calibrating:" << endl;
    turnOff(); // make sure ESC is off just in case
    cout << "ESC should start off" << endl;
    cout << "Setting max throttle" << endl;
    setThrottle(__maxAngle);
    turnOn();
    sleep(2); // hold full throttle for two seconds
    cout << "Should hear two beeps" << endl;
    sleep(1); // wait a second
    cout << "Setting neutral throttle" << endl;
    setThrottle(__neutralThrottle);
    cout << "Should hear long beep" << endl;
    sleep(2); // hold neutral throttle for two seconds
    cout << "ESC should be calibrated" << endl;
    cout << "Normal startup noises:" << endl;
    cout << "First beeps: 3 for 3 cell battery, 4 for 4 cell" << endl;
    sleep(1);
    cout << "Second beeps: 1 for brake on, 2 for brake off" << endl;
    sleep(1);
    cout << "ESC startup done" << endl;
}

void ESC::start() {
    cout << "ESC starting up" << endl;
    setThrottle(__neutralThrottle);
    turnOn();
    cout << "Listen to the ESC beeps now" << endl;
    sleep(2);
    cout << "First beeps: 3 for 3 cell battery, 4 for 4 cell" << endl;
    sleep(2);
    cout << "Second beeps: 1 for brake on, 2 for brake off" << endl;
    sleep(2);
    cout << "ESC startup done" << endl;
}
