#pragma once

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
