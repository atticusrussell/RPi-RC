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
#include <chrono>
#include <thread>
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

// void turnOn();
// void turnOff();
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
            
            // initializing to zero with pigpio is no input
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
            int rc = gpioSetMode(__powerPin, PI_OUTPUT);
            if ( rc == PI_BAD_GPIO){
                throw "Invalid GPIO pin Error!";
            } else if ( rc == PI_BAD_MODE) {
                throw "Inalid GPIO mode Error!";
            }
            // turn off ESC to start
            rc = gpioWrite(ESC_POWER_PIN, 0);
            if ( rc == PI_BAD_GPIO){
                throw "Invalid GPIO pin Error!";
            } else if ( rc == PI_BAD_LEVEL) {
                throw "Inalid GPIO level Error!";
            }
        };

        // // destructor
        // ~ESC(){
        //     cout << "ESC destructor called" << endl;
        //     setThrottle(__neutralThrottle);
        //     turnOff();
        //     cout << "ESC destructor finished" << endl;
        // }

        // TODO implement the rest of the methods for ESC
        void turnOn() {
            cout << "Powering on ESC" << endl;
            int rc = gpioWrite(__powerPin, 1);
            if ( rc == PI_BAD_GPIO){
                throw "Invalid GPIO pin Error!";
            } else if ( rc == PI_BAD_LEVEL) {
                throw "Inalid GPIO level Error!";
            }
            cout << "ESC on" << endl;
        }

        void turnOff() {
            cout << "Powering off ESC" << endl;
            int rc = gpioWrite(ESC_POWER_PIN, 0);
            if ( rc == PI_BAD_GPIO){
                throw "Invalid GPIO pin Error!";
            } else if ( rc == PI_BAD_LEVEL) {
                throw "Inalid GPIO level Error!";
            }
            cout << "ESC off" << endl;
        }

        void setThrottleRaw(double throttle) {
            setAngle(throttle);
            cout << "Throttle: " << throttle << " / ±" << __maxAngle << endl;
        }

        double fixThrottle(double throttle) {
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

        void setThrottle(double throttle) {
            throttle = fixThrottle(throttle);
            setAngle(throttle);
            cout << "Throttle: " << throttle << " / ±" << __maxAngle << endl;

        }


        void calibrate() {
            cout << "Calibrating:" << endl;
            turnOff();
            sleep(1); // time for relay to switch off
            cout << "ESC should start off" << endl;
            cout << "Setting max throttle" << endl;
            setThrottle(__maxAngle);
            // sleep(1); // arbitrary time for PWM stuff?
            sleep(1); //time for relay to switch on
            turnOn();
            // sleep(4); // hold full throttle for two seconds
            std::this_thread::sleep_for(std::chrono::seconds(2));
            cout << "Should hear two beeps" << endl;
            // sleep(1); // wait a second
            std::this_thread::sleep_for(std::chrono::seconds(1));
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

        void start() {
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

    // test the ESC relay
    ESC esc(13, -90, 90, 1000, 2000, 23, 0, 10.1, -8.1);
    esc.calibrate();

    sleep(10);


    // // test the actual servo
    // AngularServo rudderServo(18, 0, 180, 650, 2500); 
    // int angle;

    // while (true) {
    //     cout << "Enter angle (0-180, -1 to exit): ";
    //     cin >> angle;

    //     if (angle == -1) {
    //         break;
    //     }

    //     rudderServo.setAngle(angle);
    //     sleep(1);
    // }

    esc.setThrottle(0);
    esc.turnOff();

    cout << "terminating gpio" << endl;
    gpioTerminate();
    return 0;
}





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

//         turnOff();
//     } catch (...) {
//         cerr << "Error occurred" << endl;
//     }

//     setThrottle(0);
//     turnOff();
//     gpioTerminate();

//     return 0;
// }
