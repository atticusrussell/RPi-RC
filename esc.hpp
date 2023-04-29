#pragma once

#include "angular_servo.hpp"

class ESC : public AngularServo {
	public:
		ESC(int pi, int pwmPin, int fullRevThrottle, int fullFwdThrottle, int minPulseWidthUs, int maxPulseWidthUs, int powerPin, int neutralThrottle, float minFwdThrottle, float minRevThrottle);
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
