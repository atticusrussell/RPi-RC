/**
 * @file ServoMotor.h
 * @author Savan Agrawal
 * @version 0.1
 * 
 * Servo Moto Header file to take pin and angle of motor
 */
#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

class ServoMotor {
    public:
        ServoMotor(int pin);
        ~ServoMotor();
        void write(int angle);

    private:
        int _pin;
        int _angle;
};

#endif
