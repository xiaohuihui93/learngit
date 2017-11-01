//
// Created by 陈家辉 on 2017/10/29.
//

// Application file to Demonstrate the Usage of Timer Manager
// Include Header Files
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "TypeDefines.h"
#include "TimerMgrHeader.h"
#include "TimerAPI.h"
#include <pthread.h>


// Function to Print the Time
void print_time(void)
{
    time_t temp;
    struct tm *t;
    time(&temp);
    t=localtime(&temp);
    printf("%d %d %d %d:%d:%d %d",t->tm_wday,t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,t->tm_year+1900);

}

// Function to Print the Time and Msg in Callback Function
void print_time_msg(int num)
{

    // Function to print the message for each Timer like
    struct tm *Time;
    time_t t;
    t = time(NULL);
    Time = localtime(&t);
    fprintf(stdout, "This is Function %d and UTC time and date: %s\n", num,asctime(Time));
    // "This is Function 2 and UTC time and date: Thu Oct 27 20:53:27 2016
}

void print_program_info(void)
{
    fprintf(stdout, "\n\n\n\nTimer Manager Project");
    fprintf(stdout, "\n=====================");

    fprintf(stdout, "\n\nCreated by: Amit Bhorania");

    fprintf(stdout, "\n\n-> This Program will initialize the timers and creates Timer Task as Thread");
    fprintf(stdout, "\n-> It creates 3 Timers");
    fprintf(stdout, "\n\tTimer1 - Periodic 5 second");
    fprintf(stdout, "\n\tTimer2 - Periodic 3 second");
    fprintf(stdout, "\n\tTimer3 - One Shot 10 second");
    fprintf(stdout, "\n\nPress Enter to start the Program...\n");

    getchar();
}

void function1(void *arg)
{
    print_time_msg(1);
}

void function2(void *arg)
{
    print_time_msg(2);
}

void function3(void *arg)
{
    print_time_msg(3);
}

int main(void)
{
    INT8U err_val;

    RTOS_TMR *timer_obj1 = NULL;
    RTOS_TMR *timer_obj2 = NULL;
    RTOS_TMR *timer_obj3 = NULL;

    INT8 *timer_name[3] = {"Timer1", "Timer2", "Timer3"};

    // Display the Program Info
    print_program_info();

    // Initialize the OS Tick
    OSTickInitialize();

    fprintf(stdout, "OS Tick Initialization completed successfully\n");

    // Initialize the RTOS Timer
    RTOSTmrInit();

    fprintf(stdout, "\nApplication Started....... :-)\n");

    // ================================================================
    // Timer Creation
    // ================================================================

    // Create Timer1
    // Provide the required arguments in the function call
    timer_obj1 = RTOSTmrCreate(0, 50, RTOS_TMR_PERIODIC, &function1, NULL, timer_name[0], &err_val);
    // Check the return value and determine if it created successfully or not
    if(err_val == RTOS_SUCCESS) {
        fprintf(stdout, "\ntimer_obj1 created successfully");
    }
    else {
        fprintf(stderr, "\ntimer_obj1 failed error = %d", err_val);
    }
    // Create Timer2
    // Provide the required arguments in the function call
    timer_obj2 = RTOSTmrCreate(0, 30, RTOS_TMR_PERIODIC, &function2, NULL, timer_name[1], &err_val);
    // Check the return value and determine if it created successfully or not
    if(err_val == RTOS_SUCCESS) {
        fprintf(stdout, "\ntimer_obj2 created successfully");
    }
    else {
        fprintf(stderr, "\ntimer_obj2 failed error = %d", err_val);
    }
    // Create Timer3
    // Provide the required arguments in the function call
    timer_obj3 = RTOSTmrCreate(100, 0, RTOS_TMR_ONE_SHOT, &function3, NULL, timer_name[2], &err_val);
    // Check the return value and determine if it created successfully or not
    if(err_val == RTOS_SUCCESS) {
        fprintf(stdout, "\ntimer_obj3 created successfully");
    }
    else {
        fprintf(stderr, "\ntimer_obj3 failed error = %d", err_val);
    }
    // ================================================================
    // Starting Timer
    // ================================================================
    // Start Timer1
    RTOSTmrStart(timer_obj1, &err_val);
    // Check the return value and determine if it started successfully or not
    // Start Timer2
    RTOSTmrStart(timer_obj2, &err_val);
    // Check the return value and determine if it started successfully or not
    // Start Timer3
    RTOSTmrStart(timer_obj3, &err_val);
    // Check the return value and determine if it started successfully or not


    // Other Code if needed

    return 0;
}

