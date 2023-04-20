#include <pigpio.h>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <functional>
#include "angular_servo.hpp"

using namespace std;

Servo::Servo(int pin) {
    __pin=pin;
    int rc;
    rc = gpioSetMode(__pin, PI_OUTPUT);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_MODE) {
        throw "Inalid GPIO mode Error!";
    }
    
    // initializing to zero with pigpio is no input
    rc = gpioServo(__pin, 0);
    if (rc == PI_BAD_USER_GPIO) {
        throw "Invalid user GPIO pin Error!";
    } else if (rc == PI_BAD_PULSEWIDTH) {
        throw "Invalid pulsewidth Error!";
    }
}

int Servo::getPulseWidth() {
    int rc = gpioGetServoPulsewidth(__pin);
    if (rc == PI_BAD_USER_GPIO) {
        throw "Invalid user GPIO pin Error!";
    } else if (rc == PI_NOT_SERVO_GPIO) {
        throw "Invalid servo GPIO Error!";
    }
    return rc;
}

void Servo::setPulseWidth(int pulseWidth){
    int rc = gpioServo(__pin, pulseWidth);
    if (rc == PI_BAD_USER_GPIO) {
        throw "Invalid user GPIO pin Error!";
    } else if (rc == PI_BAD_PULSEWIDTH) {
        throw "Invalid pulsewidth Error!";
    }
}

AngularServo::AngularServo(int pin, float minAngle, float maxAngle, int minPulseWidthUs, int maxPulseWidthUs) : Servo(pin) {
            __minAngle = minAngle;
            __maxAngle = maxAngle;
            __minPulseWidthUs = minPulseWidthUs;
            __maxPulseWidthUs = maxPulseWidthUs;
}

void AngularServo::setAngle(float angle){
    __angle = angle;
    // make sure angle is within bounds
    if (__angle < __minAngle){
        __angle = __minAngle;
    }
    if (__angle > __maxAngle){
        __angle = __maxAngle;
    }
    int pulseWidth = __minPulseWidthUs + (__angle - __minAngle) * (__maxPulseWidthUs - __minPulseWidthUs) / (__maxAngle - __minAngle);

    setPulseWidth(pulseWidth);
}

int AngularServo::getAngle(){
    int pulseWidth = getPulseWidth();
    float angle = __minAngle + (pulseWidth - __minPulseWidthUs) * (__maxAngle - __minAngle) / (__maxPulseWidthUs - __minPulseWidthUs);
    return angle;
}

bool isPigpiodRunning() {
    int result = system("pgrep pigpiod");

    if (result == 0) {
        // pigpiod daemon is running
        return true;
    } else {
        // pigpiod daemon is not running
        return false;
    }
}

void killPigpiod() {
    if (isPigpiodRunning()) {
        int result = system("sudo killall pigpiod -q");

        if (result == 0) {
            // Successfully killed the pigpiod daemon
            std::cout << "pigpiod daemon killed successfully" << std::endl;
        } else {
            // An error occurred while trying to kill the pigpiod daemon
            std::cerr << "Error killing pigpiod daemon. Return code: " << result << std::endl;
        }
    } else {
        std::cout << "pigpiod daemon is not running" << std::endl;
    }
}
