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

//   build with: g++ -o foo_cpp  foo.cpp -lpigpio -lrt -lpthread
//  * run with : sudo ./foo_cpp

// #include "ServoMotor.h"

#include <iostream>
#include <pigpio.h>
#include <unistd.h>
#include <cmath>
using namespace std;

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

// void pwrOn();
// void pwrOff();
// void calibrate();
// void start();
// double fixThrottle(double throttle);
// void setThrottle(double throttle);
// void setThrottleRaw(double throttle);


class Servo{
    public:
        // constructor
        Servo(int pin){
            __pin=pin;
            int rc;
            rc = gpioSetMode(__pin, PI_OUTPUT);
            if ( rc == PI_BAD_GPIO){
                throw "Invalid GPIO pin Error!";
            } else if ( rc == PI_BAD_MODE) {
                throw "Inalid GPIO mode Error!";
            }

            rc = gpioServo(__pin, 0);
            if (rc == PI_BAD_USER_GPIO) {
                throw "Invalid user GPIO pin Error!";
            } else if (rc == PI_BAD_PULSEWIDTH) {
                throw "Invalid pulsewidth Error!";
            }
        }

        int getPulseWidth(){
            int rc = gpioGetServoPulsewidth(__pin);
            if (rc == PI_BAD_USER_GPIO) {
                throw "Invalid user GPIO pin Error!";
            } else if (rc == PI_NOT_SERVO_GPIO) {
                throw "Invalid servo GPIO Error!";
            }
            return rc;
        }

        void setPulseWidth(int pulseWidth){
            int rc = gpioServo(__pin, pulseWidth);
            if (rc == PI_BAD_USER_GPIO) {
                throw "Invalid user GPIO pin Error!";
            } else if (rc == PI_BAD_PULSEWIDTH) {
                throw "Invalid pulsewidth Error!";
            }
        }

    protected:
        int __pin;
};


class AngularServo : public Servo{
    public:
        // constructor
        AngularServo(int pin, int minAngle, int maxAngle, int minPulseWidthUs, int maxPulseWidthUs) : Servo(pin){
            __minAngle = minAngle;
            __maxAngle = maxAngle;
            __minPulseWidthUs = minPulseWidthUs;
            __maxPulseWidthUs = maxPulseWidthUs;
        }

        void setAngle(int angle){
            __angle = angle;
            if (__angle < __minAngle){
                __angle = __minAngle;
            }
            if (__angle > __maxAngle){
                __angle = __maxAngle;
            }

            int pulseWidth = __minPulseWidthUs + (__angle * (__maxPulseWidthUs - __minPulseWidthUs) / (__maxAngle - __minAngle));
            setPulseWidth(pulseWidth);
        }

        int getAngle(){
            int pulseWidth = getPulseWidth();
            int angle = __minAngle + (pulseWidth - __minPulseWidthUs) * (__maxAngle - __minAngle) / (__maxPulseWidthUs - __minPulseWidthUs);
            return angle;
        }

    protected:
        int __angle;
        int __minAngle;
        int __maxAngle;
        int __minPulseWidthUs;
        int __maxPulseWidthUs;
};


class ESC : public AngularServo{
    public:
        // constructor
        ESC(int pwmPin, int fullRevThrottle, int fullFwdThrottle, int minPulseWidthUs, int maxPulseWidthUs, int powerPin,  int neutralThrottle, float minFwdThrottle, float minRevThrottle) : AngularServo(pwmPin, fullRevThrottle, fullFwdThrottle, minPulseWidthUs, maxPulseWidthUs){
            __powerPin = powerPin;
            __neutralThrottle = neutralThrottle;
            __minFwdThrottle = minFwdThrottle;
            __minRevThrottle = minRevThrottle;
        };

        // TODO implement the rest of the methods for ESC

    protected:
        int __powerPin;
        int __neutralThrottle;
        float __minFwdThrottle;
        float __minRevThrottle;
};


int main() {
    if (gpioInitialise() < 0) {
        std::cerr << "Error initializing pigpio" << std::endl;
        return 1;
    }

    // test the actual servo

    AngularServo rudderServo(18, 0, 180, 650, 2500); 
    int angle;

    while (true) {
        cout << "Enter angle (0-180, -1 to exit): ";
        cin >> angle;

        if (angle == -1) {
            break;
        }

        rudderServo.setAngle(angle);
        sleep(1);
    }

    gpioTerminate();
    return 0;
}



// void pwrOn() {
//     cout << "Powering on ESC" << endl;
//     gpioWrite(ESC_POWER_PIN, 1);
//     cout << "ESC on" << endl;
// }

// void pwrOff() {
//     cout << "Powering off ESC" << endl;
//     gpioWrite(ESC_POWER_PIN, 0);
//     cout << "ESC off" << endl;
// }

// void calibrate() {
//     cout << "Calibrating:" << endl;
//     cout << "Setting max throttle" << endl;
//     setThrottle(FULL_FWD_THROTTLE);
//     pwrOn();
//     sleep(3);
//     cout << "Should hear two beeps" << endl;
//     sleep(1);
//     cout << "Setting neutral throttle" << endl;
//     setThrottle(NEUTRAL_THROTTLE);
//     cout << "Should hear long beep" << endl;
//     sleep(2);
//     cout << "ESC should be calibrated" << endl;
//     cout << "Normal startup noises:" << endl;
//     cout << "First beeps: 3 for 3 cell battery, 4 for 4 cell" << endl;
//     sleep(1);
//     cout << "Second beeps: 1 for brake on, 2 for brake off" << endl;
//     sleep(1);
//     cout << "ESC startup done" << endl;
// }

// void start() {
//     cout << "ESC starting up" << endl;
//     setThrottle(NEUTRAL_THROTTLE);
//     pwrOn();
//     cout << "Listen to the ESC beeps now" << endl;
//     sleep(2);
//     cout << "First beeps: 3 for 3 cell battery, 4 for 4 cell" << endl;
//     sleep(2);
//     cout << "Second beeps: 1 for brake on, 2 for brake off" << endl;
//     sleep(2);
//     cout << "ESC startup done" << endl;
// }

// double fixThrottle(double throttle) {
//     if (throttle > NEUTRAL_THROTTLE) {
//         throttle += MIN_FWD_THROTTLE - 1;
//     } else if (throttle < NEUTRAL_THROTTLE) {
//         throttle += MIN_REV_THROTTLE + 1;
//     }

//     if (throttle > FULL_FWD_THROTTLE) {
//         throttle = FULL_FWD_THROTTLE;
//     } else if (throttle < FULL_REV_THROTTLE) {
//         throttle = FULL_REV_THROTTLE;
//     }

//     return throttle;
// }

// void setThrottle(double throttle) {
//     double adjustThrottle = fixThrottle(throttle);
//     double pulseWidth = (adjustThrottle - FULL_REV_THROTTLE) * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / (FULL_FWD_THROTTLE - FULL_REV_THROTTLE) + MIN_PULSE_WIDTH;
//     int dutyCycle = round(pulseWidth * PWM_RANGE);
//     gpioPWM(ESC_PWM_PIN, dutyCycle);
//     cout << "Throttle: " << adjustThrottle << " / ±" << FULL_FWD_THROTTLE << endl;
// }

// void setThrottleRaw(double throttle) {
//     double pulseWidth = (throttle - FULL_REV_THROTTLE) * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / (FULL_FWD_THROTTLE - FULL_REV_THROTTLE) + MIN_PULSE_WIDTH;
//     int dutyCycle = round(pulseWidth * PWM_RANGE);
//     gpioPWM(ESC_PWM_PIN, dutyCycle);
//     cout << "Throttle: " << throttle << " / ±" << FULL_FWD_THROTTLE << endl;
// }


// int main() {
//     if (gpioInitialise() < 0) {
//         cerr << "Failed to initialize GPIO" << endl;
//         return 1;
//     }

//     AngularServo esc(ESC_PWM_PIN, FULL_REV_THROTTLE, FULL_FWD_THROTTLE, 1000, 2000);



//     gpioSetMode(ESC_POWER_PIN, PI_OUTPUT);
//     gpioWrite(ESC_POWER_PIN, 0);
// 	sleep(1); // wait for the relay to turn off - cpp is too fast

//     try {
//         calibrate();

//         double testvector[] = {1.0, 2.0, 3.0, 0.0, -1.0, -2.0, -3.0, 0.0};
//         for (double i : testvector) {
//             setThrottle(i);
//             sleep(2);
//         }

//         pwrOff();
//     } catch (...) {
//         cerr << "Error occurred" << endl;
//     }

//     setThrottle(0);
//     pwrOff();
//     gpioTerminate();

//     return 0;
// }
