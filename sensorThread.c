/*****************************************
* Class:: CSC-615-01 Spring 2024
* Name:: Tyler Hsieh
* Student ID:: 920216320
* Github-Name:: Tyler9648
* Project:: Assignment 4 - Follow in line but stop
*
* File:: sensorThread.c 
*
* Description:: creates a thread that reads the
* value of a Raspberry PI sensor. sensor functions
* directly reads values from sensor output
******************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <pigpio.h>
#include "sensorThread.h"

extern volatile sig_atomic_t exitThread;
extern pthread_mutex_t exitLock;

void *sensor_thread(void* arg){         //thread function that constantly reads a 
    tArg* sensorArgs = (tArg*) arg;     //sensor's value

    /* 
    * as of right now all sensor switch cases are the exact same, but having cases
    * makes it easier to debug or make changes to a specific sensor. also makes it
    * very easy to add more sensors 
    * aim of having switches and seperate functions is to make it easier to implement
    * in upcoming car project 
    */

    switch(sensorArgs->senType){
        case LINE:          //line sensor
            gpioSetMode(sensorArgs->pin, PI_INPUT);
            while(exitThread == 0){
                sensorArgs->value = lineSensor(sensorArgs->pin);
            }
            break;
        case AVOID:         //avoidance sensor
            gpioSetMode(sensorArgs->pin, PI_INPUT);
            while(exitThread == 0){
                sensorArgs->value = avoidSensor(sensorArgs->pin);
            }
            break;
        case TEST:          //for sensor free thread testing and debugging
            while(exitThread == 0){
                sensorArgs->value = 2;
            }
            break;
        case SONAR:         //sonar sensor
            gpioSetMode(sensorArgs->trigger, PI_OUTPUT);    //initialize trigger
            gpioWrite(sensorArgs->trigger, PI_LOW);
            gpioSetMode(sensorArgs->pin, PI_INPUT);         //initialize echo pin
            while(exitThread == 0){
                sensorArgs->value = sonarSensor(sensorArgs->pin, sensorArgs->trigger);
            }
        default:            //invalid sensor type
            printf("invalid sensor type in sensorIR_thread: arg -> sensorArgs.senType\n");
            break;
    }
    
    //exit thread now
    pthread_mutex_lock(&exitLock);      //mutex lock to protect argument deallocation
    printf("\nfreeing allocated thread args in thread: %d\n", pthread_self());
    free(arg);                          //free arg that was malloced from main
    arg = NULL;
    pthread_mutex_unlock(&exitLock);
}

int lineSensor(int pin){    //reads 0 (LOW) when line is white, reads 1 (HIGH) when line is black
    int val = gpioRead(pin);
    return val;
}

int avoidSensor(int pin){   //reads 0 (LOW) when obstacle detected, reads 1 (HIGH) when no obstacle
    int val = gpioRead(pin);
    return val;
}

//sonar sensor accurately reads between 2 cm and 200 cm so keep that in mind
int sonarSensor(int pin, int trigger){  //returns distance between sonar and closest object in cm
    timedGPIOHigh(trigger, 15);         //start the sonar trigger
    clock_t echoUp, echoDown;
    float distance;
    while(gpioRead(pin) == 0){       //wait until we get echo input
    }
    if(gpioRead(pin) != 0){          //once we get echo input,
        echoUp = clock();           //keep track of how when we first receive echo input
        while(gpioRead(pin) != 0){   //wait until echoing stops
        }
        if(gpioRead(pin) == 0){      //keep track of when echo input stops
            echoDown = clock();
        }
    }
    float timeEchoedSecs;

        //calculate total uptime the echo input was up for
    timeEchoedSecs = (float)(echoDown - echoUp) / CLOCKS_PER_SEC;   

        //calculate distance between sonar and object in cm
    distance = timeEchoedSecs * (float)SOUND_DIST_MULT * 100;

        //printf("measured distance of %f centimeters\n", distance);
    return distance;

}

int timedGPIOHigh(int trigger, int duration){     // takes gpio trigger pin, and duration in microseconds to seend HIGH (1) signal for 
    gpioWrite(trigger, PI_HIGH);
    usleep(duration);
    gpioWrite(trigger, PI_LOW);
}