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

ServoMotor::ServoMotor(int pin) {
    ServoMotor::_pin = pin;
    ServoMotor::_angle = 0;
    gpioSetMode(_pin, PI_OUTPUT);
}

ServoMotor::~ServoMotor() {
}

void ServoMotor::write(int angle) {
    ServoMotor::_angle = angle;
    if (ServoMotor::_angle < 0) ServoMotor::_angle = 0;
    if (ServoMotor::_angle > 180) ServoMotor::_angle = 180;

    int pulseWidthMin = 650; 
    int pulseWidthMax = 2500; 

    int pulseWidth = pulseWidthMin + (ServoMotor::_angle * (pulseWidthMax - pulseWidthMin) / 180);
    std::cout << "Pulse width: " << pulseWidth << std::endl;
    gpioServo(ServoMotor::_pin, pulseWidth);
}

int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "Error initializing pigpio" << std::endl;
        return 1;
    }

    ServoMotor servo(27); 
    int angle;

    while (true) {
        std::cout << "Enter angle (0-180, -1 to exit): ";
        std::cin >> angle;

        if (angle == -1) {
            break;
        }

        servo.write(angle);
        sleep(1);
    }

    gpioTerminate();
    return 0;
}
