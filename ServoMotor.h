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
        ServoMotor(int pin, int pulseWidthMin, int pulseWidthMax);
        ~ServoMotor();
        void write(int angle);

    private:
        int _pin;
        int _angle;
        int _pulseWidthMin;
        int _pulseWidthMax;
};

class AngularServo {
    public:
        AngularServo(int pin, int minAngle, int maxAngle, int minPulseWidthMs, int maxPulseWidthMs);
        ~AngularServo();
        void setAngle(int angle);
        int getAngle() const;

    private:
        int _pin;
        int _angle;
        int _minAngle;
        int _maxAngle;
        int _minPulseWidthMs;
        int _maxPulseWidthMs;
};

#endif
