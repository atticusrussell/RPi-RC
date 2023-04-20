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


#include <iostream>
#include <pigpio.h>
#include <unistd.h>
#include <cmath>
#include <csignal>
#include <cstdlib> // Required for system()
#include <functional>

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
        AngularServo(int pin, float minAngle, float maxAngle, int minPulseWidthUs, int maxPulseWidthUs) : Servo(pin){
            __minAngle = minAngle;
            __maxAngle = maxAngle;
            __minPulseWidthUs = minPulseWidthUs;
            __maxPulseWidthUs = maxPulseWidthUs;
        }

        void setAngle(float angle){
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

        int getAngle(){
            int pulseWidth = getPulseWidth();
            float angle = __minAngle + (pulseWidth - __minPulseWidthUs) * (__maxAngle - __minAngle) / (__maxPulseWidthUs - __minPulseWidthUs);
            return angle;
        }

    protected:
        float __angle;
        float __minAngle;
        float __maxAngle;
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
            turnOff(); // make sure ESC is off just in case
            cout << "ESC should start off" << endl;
            cout << "Setting max throttle" << endl;
            setThrottle(__maxAngle);
            turnOn();
            sleep(2); // hold full throttle for two seconds
            cout << "Should hear two beeps" << endl;
            sleep(1); // wait a second
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

// global variable to store a reference to the local ESC instance:
ESC *escPtr = nullptr;

void handleSignal(int signal) {
    if (signal == SIGINT && escPtr != nullptr) {
        std::cout << "Ctrl+C pressed, setting throttle to zero and turning off the ESC" << std::endl;
        escPtr->setThrottle(0);
        escPtr->turnOff();
        gpioTerminate();
        exit(0);
    }
}


int main() {
    // // TODO kill pigpiod if it is running (after getting rest of this working)
    if (isPigpiodRunning()) {
        std::cout << "pigpiod daemon is running" << std::endl;
        killPigpiod();
        sleep(1); // wait a second for pigpiod to die
    }

    if (gpioInitialise() < 0) {
        std::cerr << "Error initializing pigpio" << std::endl;
        return 1;
    }

    // test the actual servo
    AngularServo rudderServo(18, 0, 180, 650, 2500);
    rudderServo.setAngle(90);
    sleep(1);
    rudderServo.setAngle(0);
    sleep(1);
    rudderServo.setAngle(180);
    sleep(1);
    // stop sending any real signal
    rudderServo.setPulseWidth(0);


    // test the ESC
    ESC esc(13, -90, 90, 1000, 2000, 23, 0, 10.1, -8.1);
    // Set the global pointer to the local instance
    escPtr = &esc;

    // Set the signal handler function for SIGINT (Ctrl+C)
    struct sigaction sa;
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    try {
        // test the ESC 
        esc.calibrate();

        // sleep(10);

        double testvector[] = {1.0, 2.0, 3.0, 0.0, -1.0, -2.0, -3.0, 0.0};
        for (double i : testvector) {
            esc.setThrottle(i);
            sleep(2);
        }

    } catch (const std::exception& e) {
        cerr <<  "Error initializing MyObject: " << e.what() << endl;
    }

    esc.setThrottle(0);
    esc.turnOff();
    cout << "Terminating gpio" << endl;
    gpioTerminate();
    return 0;
}

    // test the actual servo
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
