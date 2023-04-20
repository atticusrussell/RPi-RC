#pragma once

#include <pigpio.h>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <functional>

class Servo {
	public:
		Servo(int pin);
		int getPulseWidth();
		void setPulseWidth(int pulseWidth);

	protected:
		int __pin;
};

class AngularServo : public Servo {
	public:
		AngularServo(int pin, float minAngle, float maxAngle, int minPulseWidthUs, int maxPulseWidthUs);
		void setAngle(float angle);
		int getAngle();

	protected:
		float __angle;
		float __minAngle;
		float __maxAngle;
		int __minPulseWidthUs;
		int __maxPulseWidthUs;
};

class ESC : public AngularServo {
	public:
		ESC(int pwmPin, int fullRevThrottle, int fullFwdThrottle, int minPulseWidthUs, int maxPulseWidthUs, int powerPin, int neutralThrottle, float minFwdThrottle, float minRevThrottle);
		void turnOn();
		void turnOff();
		void setThrottleRaw(double throttle);
		double fixThrottle(double throttle);
		void setThrottle(double throttle);
		void calibrate();
		void start();

	protected:
		int __powerPin;
		int __neutralThrottle;
		float __minFwdThrottle;
		float __minRevThrottle;
};

bool isPigpiodRunning();
void killPigpiod();
