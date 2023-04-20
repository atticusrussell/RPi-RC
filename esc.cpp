//   build with: g++ -o foo_cpp  foo.cpp -lpigpio -lrt -lpthread
//  * run with : sudo ./foo_cpp


#include "esc.hpp"

using namespace std;


Servo::Servo(int pin) {
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

int Servo::getPulseWidth() {
    int rc = gpioGetServoPulsewidth(__pin);
    if (rc == PI_BAD_USER_GPIO) {
        throw "Invalid user GPIO pin Error!";
    } else if (rc == PI_NOT_SERVO_GPIO) {
        throw "Invalid servo GPIO Error!";
    }
    return rc;
}

void Servo::setPulseWidth(int pulseWidth){
    int rc = gpioServo(__pin, pulseWidth);
    if (rc == PI_BAD_USER_GPIO) {
        throw "Invalid user GPIO pin Error!";
    } else if (rc == PI_BAD_PULSEWIDTH) {
        throw "Invalid pulsewidth Error!";
    }
}

AngularServo::AngularServo(int pin, float minAngle, float maxAngle, int minPulseWidthUs, int maxPulseWidthUs) : Servo(pin) {
            __minAngle = minAngle;
            __maxAngle = maxAngle;
            __minPulseWidthUs = minPulseWidthUs;
            __maxPulseWidthUs = maxPulseWidthUs;
}

void AngularServo::setAngle(float angle){
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

int AngularServo::getAngle(){
    int pulseWidth = getPulseWidth();
    float angle = __minAngle + (pulseWidth - __minPulseWidthUs) * (__maxAngle - __minAngle) / (__maxPulseWidthUs - __minPulseWidthUs);
    return angle;
}



ESC::ESC(int pwmPin, int fullRevThrottle, int fullFwdThrottle, int minPulseWidthUs, int maxPulseWidthUs, int powerPin,  int neutralThrottle, float minFwdThrottle, float minRevThrottle) : AngularServo(pwmPin, fullRevThrottle, fullFwdThrottle, minPulseWidthUs, maxPulseWidthUs) {
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
    rc = gpioWrite(__powerPin, 0);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_LEVEL) {
        throw "Inalid GPIO level Error!";
    }
}

// // destructor
// ~ESC(){
//     cout << "ESC destructor called" << endl;
//     setThrottle(__neutralThrottle);
//     turnOff();
//     cout << "ESC destructor finished" << endl;
// }

void ESC::turnOn() {
    cout << "Powering on ESC" << endl;
    int rc = gpioWrite(__powerPin, 1);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_LEVEL) {
        throw "Inalid GPIO level Error!";
    }
    cout << "ESC on" << endl;
}

void ESC::turnOff() {
    cout << "Powering off ESC" << endl;
    int rc = gpioWrite(__powerPin, 0);
    if ( rc == PI_BAD_GPIO){
        throw "Invalid GPIO pin Error!";
    } else if ( rc == PI_BAD_LEVEL) {
        throw "Inalid GPIO level Error!";
    }
    cout << "ESC off" << endl;
}

void ESC::setThrottleRaw(double throttle) {
    setAngle(throttle);
    cout << "Throttle: " << throttle << " / ±" << __maxAngle << endl;
}

double ESC::fixThrottle(double throttle) {
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

void ESC::setThrottle(double throttle) {
    throttle = fixThrottle(throttle);
    setAngle(throttle);
    cout << "Throttle: " << throttle << " / ±" << __maxAngle << endl;
}


void ESC::calibrate() {
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

void ESC::start() {
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





