// build:
// g++ -o testServo testServo.cpp angular_servo.cpp -lpigpio -lrt -lpthread
// run:
// sudo ./testESC

#include <csignal>
#include <iostream>
#include <pigpio.h>
#include <unistd.h>
#include "angular_servo.hpp"

using namespace std;


void handleSignal(int signal) {
    if (signal == SIGINT ) {
        std::cout << "Ctrl+C pressed, setting throttle to zero and turning off the Servo" << std::endl;
        gpioTerminate();
        exit(0);
    }
}

int main() {
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
    try {
        AngularServo rudderServo(18, 0, 180, 650, 2500);
    rudderServo.setAngle(90);
    sleep(1);
    rudderServo.setAngle(0);
    sleep(1);
    rudderServo.setAngle(180);
    sleep(1);
    // stop sending any real signal
    rudderServo.setPulseWidth(0);

    // Set the signal handler function for SIGINT (Ctrl+C)
    struct sigaction sa;
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    } catch (const exception& e) {
        cerr <<  "Error initializing MyObject: " << e.what() << endl;
    }
    
    cout << "Terminating gpio" << endl;
    gpioTerminate();
    return 0;
}
