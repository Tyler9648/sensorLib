/*****************************************
* Class:: CSC-615-01 Spring 2024
* Name:: Tyler Hsieh
* Student ID:: 920216320
* Github-Name:: Tyler9648
* Project:: sensor library
*
* File:: sensorLib.c 
*
* Description:: sensor library, edit this file to
* add more sensors
*
* - to access them, use nameSensorArgs->value 
*
* - wrap main body of car code with exitLock
******************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pigpio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "sensorThread.h"

#define LINESENSOR_GPIO 19              // sensor gpio pins 
#define AVOIDSENSOR_GPIO 26             // add EVERY GPIO pin used by sensors here
#define SONARSENSOR_GPIO 23
#define TRIGGER_GPIO 24
                                        // add ENABLE_SENSOR for each sensor or sensor cluster
#define ENABLE_LINESENSOR 1             // set to 1 to enable, 0 to disable
#define ENABLE_AVOIDSENSOR 1
#define ENABLE_SONARSENSOR 1
#define ENABLE_TEST 0

#define NUMTHREADS (ENABLE_LINESENSOR + ENABLE_AVOIDSENSOR + ENABLE_TEST + ENABLE_SONARSENSOR)  // only need 2, one for each sensor
                                        // can be changed if more sensors/components are added

pthread_mutex_t exitLock;
volatile sig_atomic_t exitThread = 0;   // 0 by default
                                        // 1 to exit all threads and main 

//int activeThreads;                    // uncomment if implemented with car

pthread_t threadID[NUMTHREADS];         // sensor threads 

void progExit(int sig){                 // catch for ctr+c to exit so we can properly
    exitThread = 1;                                                                
}

void sensorLibInit(void){               // handles creation of all sensor threads
                                         // NOT used when testing main in sensorLib, only for car's main
                                         // completely replaces main in sensorLib in car implementation
    if(pthread_mutex_init(&exitLock, NULL) != 0){       //initialize mutex lock       
        printf("\n mutex init failed, now exiting\n");
    }
    printf("# of sensors: %d\n", NUMTHREADS);

    //thread args - add another for each thread
    tArg* lineSensorArgs;
    tArg* avoidSensorArgs;
    tArg* testArgs;
    tArg* sonarSensorArgs;

    activeThreads = 0;

    //initialize and launch line sensor thread
    if(ENABLE_LINESENSOR == 1){
        lineSensorArgs = (tArg*)malloc(sizeof(tArg));
        lineSensorArgs->senType = LINE;            
        lineSensorArgs->pin = LINESENSOR_GPIO;
        lineSensorArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)lineSensorArgs);
        activeThreads++;
    }
    //prepare and launch avoidance sensor thread 
    if(ENABLE_AVOIDSENSOR == 1){
        avoidSensorArgs = (tArg*)malloc(sizeof(tArg));
        avoidSensorArgs->senType = AVOID;
        avoidSensorArgs->pin = AVOIDSENSOR_GPIO;
        avoidSensorArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)avoidSensorArgs);    
        activeThreads++;
    }
    //prepare and launch avoidance sensor thread 
    if(ENABLE_SONARSENSOR == 1){
        sonarSensorArgs = (tArg*)malloc(sizeof(tArg));
        sonarSensorArgs->senType = SONAR;
        sonarSensorArgs->pin = SONARSENSOR_GPIO;
        sonarSensorArgs->trigger = TRIGGER_GPIO;
        sonarSensorArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)sonarSensorArgs);    
        activeThreads++;
    }
    //prepare and launch test sensor thread w/o pins 
    if(ENABLE_TEST == 1){
        testArgs = (tArg*)malloc(sizeof(tArg));
        testArgs->senType = TEST;     
        testArgs->pin = -1;           //no pin being used
        testArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)testArgs);
        activeThreads++;
    }
    //feel free to add more, if more sensors are added
    //must CREATE SEPERATE THREAD FOR EVERY SENSOR (unless theyre clustered)
    //sensor clusters must be added to sensorThread as if they're a completely new sensor
    //IMPORTANT -- make sure main loop in car's main that reads sensor vals is protected w exitLock
}

void sensorLibTerminate(void){          // call after motors are terminated
                                        // ONLY used for car implementation
    //close threads to prepare for clean up
    printf("\nClosing threads, %d threads currently running\n", activeThreads);
    int i;
    for(i = 0; i < NUMTHREADS; i++){
        if(threadID[i]){
            pthread_join(threadID[i], NULL);
            activeThreads--;
            printf("\nThread closed, %d still running\n", activeThreads);
        }
    }
    printf("\nAll threads closed, now cleaning up and exiting\n");

    pthread_mutex_destroy(&exitLock);   // clean up
}

int main(int argc, char *argv[])        //main driver code, replace with functions to moduralize later, main should become sensorLibInit
{  
    if (gpioInitialise() < 0){                          //initialize GPIO pins
        printf("\nGPIO initialization failed, now exiting\n\n");
        exit(0);
    } else {
        printf("\nGPIO initialization success\n");
    }
    if(pthread_mutex_init(&exitLock, NULL) != 0){       //initialize mutex lock       
        printf("\n mutex init failed, now exiting\n");
    }

    signal(SIGINT, progExit);                           //intialize signal interrupt handler so
                                                        //we can stop with ctrl+c

    printf("# of sensors: %d\n", NUMTHREADS);

    //thread args
    tArg* lineSensorArgs;
    tArg* avoidSensorArgs;
    tArg* testArgs;
    tArg* sonarSensorArgs;

    int activeThreads = 0;

    //initialize and launch line sensor thread
    if(ENABLE_LINESENSOR == 1){
        lineSensorArgs = (tArg*)malloc(sizeof(tArg));
        lineSensorArgs->senType = LINE;            
        lineSensorArgs->pin = LINESENSOR_GPIO;
        lineSensorArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)lineSensorArgs);
        activeThreads++;
    }
    //prepare and launch avoidance sensor thread 
    if(ENABLE_AVOIDSENSOR == 1){
        avoidSensorArgs = (tArg*)malloc(sizeof(tArg));
        avoidSensorArgs->senType = AVOID;
        avoidSensorArgs->pin = AVOIDSENSOR_GPIO;
        avoidSensorArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)avoidSensorArgs);    
        activeThreads++;
    }
    //prepare and launch sonar sensor thread 
    if(ENABLE_SONARSENSOR == 1){
        sonarSensorArgs = (tArg*)malloc(sizeof(tArg));
        sonarSensorArgs->senType = SONAR;
        sonarSensorArgs->pin = SONARSENSOR_GPIO;
        sonarSensorArgs->trigger = TRIGGER_GPIO;
        sonarSensorArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)sonarSensorArgs);    
        activeThreads++;
    }
    //prepare and launch test sensor thread w/o pins 
    if(ENABLE_TEST == 1){
        testArgs = (tArg*)malloc(sizeof(tArg));
        testArgs->senType = TEST;     
        testArgs->pin = -1;           //no pin being used
        testArgs->value = -1;
        pthread_create(&threadID[activeThreads], NULL, &sensor_thread, (void*)testArgs);
        activeThreads++;
    }
    while(exitThread == 0){
        usleep(1000000);                        //1 second delay so we dont spam terminal too much            

        pthread_mutex_lock(&exitLock);          //protect malloc'd thread args so we can finish 
                                                //reading values before thread deallocates them
                                                //during exit
        printf("\n");
                                               
        if(ENABLE_LINESENSOR == 1 && lineSensorArgs){           //read the line sensor to see if we're
                                                                //on black or white 
            if(lineSensorArgs->value == 0){
                printf("Line sensor: on white\n");
            } else if(lineSensorArgs->value == 1){
                printf("Line sensor: on black\n");   
            }
        }
            
        if(ENABLE_AVOIDSENSOR == 1 && avoidSensorArgs){         //read avoid sensor to see if there 
                                                                //are any obstacles
            if(avoidSensorArgs->value == 0){
                printf("Avoid sensor: obstacle detected\n");
            } else if(avoidSensorArgs->value == 1){
                printf("Avoid sensor: no obstacles detected\n");
            }
        }

        if(ENABLE_SONARSENSOR == 1 && sonarSensorArgs){         //read sonar sensor to get distance                                                     
            if(sonarSensorArgs->value >= 0){
                printf("Sonar sensor: distance is %d\n", sonarSensorArgs->value);
            }
        }
                                                                //read test sensor to debug/test threads 
        if(ENABLE_TEST == 1 && testArgs){

            if(testArgs->value >= 0) {
                printf("test value displaying: %d\n", testArgs->value);
            }
        }
        printf("\n");

        pthread_mutex_unlock(&exitLock);
    }
    
    //close threads to prepare for clean up
    printf("\nClosing threads, %d threads currently running\n", activeThreads);
    int i;
    for(i = 0; i < NUMTHREADS; i++){
        if(threadID[i]){
            pthread_join(threadID[i], NULL);
            activeThreads--;
            printf("\nThread closed, %d still running\n", activeThreads);
        }
    }
    printf("\nAll threads closed, now cleaning up and exiting\n");

    pthread_mutex_destroy(&exitLock);   // clean up
    gpioTerminate();                    // unexport gpio pins
    exit(0);
}