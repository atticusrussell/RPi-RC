/**
 * @file ServoMotor.cpp
 * @author Savan Agrawal
 * @version 0.1
 * 
 * Servo Moto cpp file to take pin and angle of motor
 */
#include "ServoMotor.h"

#include <iostream>
#include <unistd.h>
#include <pigpio.h>

AngularServo::AngularServo(int pin, int minAngle, int maxAngle, int minPulseWidthMs, int maxPulseWidthMs) {
	AngularServo::_pin = pin;
	AngularServo::_minAngle = minAngle;
	AngularServo::_maxAngle = maxAngle;
	AngularServo::_minPulseWidthUs = minPulseWidthMs;
	AngularServo::_maxPulseWidthUs = maxPulseWidthMs;
	AngularServo::_angle = 0;
	gpioSetMode(_pin, PI_OUTPUT);
}


AngularServo::~AngularServo() {
}


void AngularServo::setAngle(int angle) {
	AngularServo::_angle = angle;
	if (AngularServo::_angle < AngularServo::_minAngle) AngularServo::_angle = AngularServo::_minAngle;
	if (AngularServo::_angle > AngularServo::_maxAngle) AngularServo::_angle = AngularServo::_maxAngle;

	int pulseWidth = AngularServo::_minPulseWidthUs + (AngularServo::_angle * (AngularServo::_maxPulseWidthUs - AngularServo::_minPulseWidthUs) / (AngularServo::_maxAngle - AngularServo::_minAngle));
	std::cout << "Pulse width: " << pulseWidth << std::endl;
	gpioServo(AngularServo::_pin, pulseWidth);
}


int AngularServo::getAngle() const {
	return AngularServo::_angle;
}


int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "Error initializing pigpio" << std::endl;
        return 1;
    }

    AngularServo servo(18, 0, 180, 650, 2500); 
    int angle;

    while (true) {
        std::cout << "Enter angle (0-180, -1 to exit): ";
        std::cin >> angle;

        if (angle == -1) {
            break;
        }

        servo.setAngle(angle);
        sleep(1);
    }

    gpioTerminate();
    return 0;
}
