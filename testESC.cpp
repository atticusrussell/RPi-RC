// build:
// g++ -o testESC testESC.cpp esc.cpp angular_servo.cpp -lpigpiod_if2 -lrt -lpthread
// run:
// sudo ./testESC

#include <csignal>
#include <iostream>
#include <pigpiod_if2.h>
#include <unistd.h>
#include "esc.hpp"
#include "angular_servo.hpp"

using namespace std;

// global variable to store a reference to the local ESC instance:
ESC *escPtr = nullptr;

void handleSignal(int signal) {
    if (signal == SIGINT && escPtr != nullptr) {
        std::cout << "Ctrl+C pressed, setting throttle to zero and turning off the ESC" << std::endl;
        escPtr->setThrottle(0);
        escPtr->turnOff();
        pigpio_stop(0); // NOTE should be actual pi number
        exit(0);
    }
}

int main() {
    if (!isPigpiodRunning()) {
        startPigpiod();
    }


    int pi = pigpio_start(nullptr, nullptr);
    if (pi < 0) {
        cerr << "Error initializing pigpio" << endl;
        return 1;
    }

    // test the actual servo
    AngularServo rudderServo(pi, 18, 0, 180, 650, 2500);
    rudderServo.setAngle(90);
    sleep(1);
    rudderServo.setAngle(0);
    sleep(1);
    rudderServo.setAngle(180);
    sleep(1);
    // stop sending any real signal
    rudderServo.setPulseWidth(0);


    // test the ESC
    ESC esc(pi, 13, -90, 90, 1000, 2000, 23, 0, 10.1, -8.1);
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

    } catch (const exception& e) {
        cerr <<  "Error initializing MyObject: " << e.what() << endl;
    }

    esc.setThrottle(0);
    esc.turnOff();
    cout << "Terminating gpio" << endl;
    pigpio_stop(pi);
    return 0;
}
