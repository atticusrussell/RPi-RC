// build:
// g++ -o testServo testServo.cpp angular_servo.cpp -lpigpiod_if2 -lrt -lpthread
// run:
// sudo ./testESC

#include <csignal>
#include <iostream>
#include <pigpiod_if2.h>
#include <unistd.h>
#include "angular_servo.hpp"

using namespace std;


void handleSignal(int signal) {
    if (signal == SIGINT ) {
        cout << "Ctrl+C pressed, setting angle to zero and turning off the Servo" << endl;
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
    try {
        AngularServo rudderServo(pi, 18, 0, 180, 650, 2500);
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
    pigpio_stop(pi);
    return 0;
}
