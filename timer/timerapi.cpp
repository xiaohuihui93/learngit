// Header Files
#include "TypeDefines.h"
#include "TimerMgrHeader.h"
#include "TimerAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

/*****************************************************
 * Global Variables
 *****************************************************
 */
// Timer Pool Global Variables
INT8U FreeTmrCount = 0;
RTOS_TMR *FreeTmrListPtr = NULL;

// Tick Counter
INT32U RTOSTmrTickCtr = 0;

// Hash Table
HASH_OBJ hash_table[HASH_TABLE_SIZE];

// Thread variable for Timer Task
pthread_t thread;

// Semaphore for Signaling the Timer Task
sem_t timer_task_sem;

// Mutex for Protecting Hash Table
pthread_mutex_t hash_table_mutex;

// Mutex for Protecting Timer Pool
pthread_mutex_t timer_pool_mutex;

/*****************************************************
 * Timer API Functions
 *****************************************************
 */

// Function to create a Timer
RTOS_TMR* RTOSTmrCreate(INT32U delay, INT32U period, INT8U option, RTOS_TMR_CALLBACK callback, void *callback_arg, INT8	*name, INT8U *err)
{
    RTOS_TMR *timer_obj = NULL;

    // Check the input Arguments for ERROR

    // Allocate a New Timer Obj
    timer_obj = alloc_timer_obj();

    if(timer_obj == NULL) {
        // Timers are not available
        *err = RTOS_ERR_TMR_NON_AVAIL;
        return NULL;
    }

    // Fill up the Timer Object
    timer_obj->RTOSTmrType          = RTOS_TMR_TYPE;
    timer_obj->RTOSTmrCallback      = callback;
    timer_obj->RTOSTmrCallbackArg   = callback_arg;
    timer_obj->RTOSTmrName          = name;
    timer_obj->RTOSTmrNext          = NULL;
    timer_obj->RTOSTmrPrev          = NULL;
    timer_obj->RTOSTmrState         = RTOS_TMR_STATE_STOPPED;
    timer_obj->RTOSTmrDelay         = delay;
    timer_obj->RTOSTmrPeriod        = period;
    timer_obj->RTOSTmrOpt           = option;

//    insert_hash_entry(timer_obj);


    *err = RTOS_SUCCESS;

    return timer_obj;
}

// Function to Delete a Timer
INT8U RTOSTmrDel(RTOS_TMR *ptmr, INT8U *perr)
{
    // ERROR Checking

    if (ptmr == NULL)
    {
        *perr=RTOS_ERR_TMR_INVALID;
        return RTOS_FALSE;
    }
    else if(ptmr->RTOSTmrType==NULL) {
        *perr = RTOS_ERR_TMR_INVALID_TYPE;

    }
    else if(ptmr->RTOSTmrName==NULL) {
        *perr = RTOS_ERR_TMR_INACTIVE;

    }

    // Free Timer Object according to its State

    switch (ptmr->RTOSTmrState) {
        case RTOS_TMR_STATE_COMPLETED:
        case RTOS_TMR_STATE_STOPPED:
        case RTOS_TMR_STATE_UNUSED:
            ptmr->RTOSTmrState = RTOS_TMR_STATE_UNUSED;
            free_timer_obj(ptmr);
            break;
        case RTOS_TMR_STATE_RUNNING:
            RTOSTmrStop(ptmr, RTOS_TMR_OPT_NONE, NULL, perr);
            free_timer_obj(ptmr);
            break;

        default:
            *perr = RTOS_ERR_TMR_INVALID_STATE;
            return RTOS_FALSE;

    }
    remove_hash_entry(ptmr);
    *perr = RTOS_SUCCESS;
    return RTOS_TRUE;
}

// Function to get the Name of a Timer
INT8* RTOSTmrNameGet(RTOS_TMR *ptmr, INT8U *perr)
{
    // ERROR Checking

    if (ptmr == NULL)
    {
        *perr=RTOS_ERR_TMR_INVALID;
        return 0;
    }
    else if(ptmr->RTOSTmrType==NULL) {
        *perr = RTOS_ERR_TMR_INVALID_TYPE;
    }
    else if(ptmr->RTOSTmrName==NULL) {
        *perr = RTOS_ERR_TMR_INACTIVE;

    }

    // Return the Pointer to the String
    return ptmr->RTOSTmrName;

}

// To Get the Number of ticks remaining in time out
INT32U RTOSTmrRemainGet(RTOS_TMR *ptmr, INT8U *perr)
{
    // ERROR Checking

    // Return the remaining ticks
    return ptmr->RTOSTmrMatch-RTOSTmrTickCtr;
}

// To Get the state of the Timer
INT8U RTOSTmrStateGet(RTOS_TMR *ptmr, INT8U *perr)
{
    // ERROR Checking
    if (ptmr == NULL)
    {
        *perr=RTOS_ERR_TMR_INVALID;

        return RTOS_FALSE;
    }
    else if(ptmr->RTOSTmrType==NULL) {
        *perr = RTOS_ERR_TMR_INVALID_TYPE;

    }
    else if(ptmr->RTOSTmrName==NULL) {
        *perr = RTOS_ERR_TMR_INACTIVE;

    }

    // Return State
    return ptmr->RTOSTmrState;
}

// Function to start a Timer
INT8U RTOSTmrStart(RTOS_TMR *ptmr, INT8U *perr)
{
    // ERROR Checking
    if (ptmr == NULL)
    {
        *perr=RTOS_ERR_TMR_INVALID;
        return RTOS_FALSE;
    }
    else if(ptmr->RTOSTmrType==NULL) {
        *perr = RTOS_ERR_TMR_INVALID_TYPE;

    }
    else if(ptmr->RTOSTmrName==NULL) {
        *perr = RTOS_ERR_TMR_INACTIVE;

    }


    // Based on the Timer State, update the RTOSTmrMatch using RTOSTmrTickCtr, RTOSTmrDelay and RTOSTmrPeriod
    if (ptmr->RTOSTmrState == RTOS_TMR_STATE_STOPPED||
        ptmr->RTOSTmrState == RTOS_TMR_STATE_COMPLETED) {
        if (ptmr->RTOSTmrOpt==RTOS_TMR_ONE_SHOT) {
            ptmr->RTOSTmrMatch = ptmr->RTOSTmrDelay + RTOSTmrTickCtr;
        }else{
            ptmr->RTOSTmrMatch = ptmr->RTOSTmrPeriod + RTOSTmrTickCtr;
        }
    }
    // You may use the Hash Table to Insert the Running Timer Obj
    insert_hash_entry(ptmr);
    ptmr->RTOSTmrState = RTOS_TMR_STATE_RUNNING;
    return RTOS_TRUE;
}

// Function to Stop the Timer
INT8U RTOSTmrStop(RTOS_TMR *ptmr, INT8U opt, void *callback_arg, INT8U *perr)
{
    // ERROR Checking
    if (ptmr == NULL)
    {
        *perr=RTOS_ERR_TMR_INVALID;
        return RTOS_FALSE;
    }
    else if(ptmr->RTOSTmrType==NULL) {
        *perr = RTOS_ERR_TMR_INVALID_TYPE;

    }
    else if(ptmr->RTOSTmrName==NULL) {
        *perr = RTOS_ERR_TMR_INACTIVE;

    }



    // Remove the Timer from the Hash Table List
    remove_hash_entry(ptmr);

    // Change the State to Stopped
    ptmr->RTOSTmrState = RTOS_TMR_STATE_STOPPED;
    // Call the Callback function if required
    if (ptmr->RTOSTmrCallback) {
        if(opt == RTOS_TMR_OPT_CALLBACK_ARG){
            ptmr->RTOSTmrCallback(callback_arg);
        }
        else if(opt == RTOS_TMR_OPT_CALLBACK){
            ptmr->RTOSTmrCallback(ptmr->RTOSTmrCallbackArg);

        }
    }


    return RTOS_TRUE;

}

// Function called when OS Tick Interrupt Occurs which will signal the RTOSTmrTask() to update the Timers
void RTOSTmrSignal(int signum)
{
    // Received the OS Tick
    // Send the Signal to Timer Task using the Semaphore
    sem_post(&timer_task_sem);
}

/*****************************************************
 * Internal Functions
 *****************************************************
 */

// Create Pool of Timers
INT8U Create_Timer_Pool(INT32U timer_count)
{
    // Create the Timer pool using Dynamic Memory Allocation
    // You can imagine of LinkedList Creation for Timer Obj

//    RTOS_TMR *
    if (FreeTmrListPtr==NULL) {
        FreeTmrListPtr = (RTOS_TMR*)malloc(sizeof(RTOS_TMR));
    }
    RTOS_TMR* current = FreeTmrListPtr;
    for (int i=1; i<timer_count; ++i) {
        current->RTOSTmrNext = (RTOS_TMR*)malloc(sizeof(RTOS_TMR));
        current->RTOSTmrNext->RTOSTmrPrev = current;
        current = current->RTOSTmrNext;
    }
    FreeTmrCount =timer_count;
    return RTOS_SUCCESS;
}

// Initialize the Hash Table
void init_hash_table(void)
{
    for (int i=0; i<HASH_TABLE_SIZE; ++i) {
        hash_table[i].timer_count   = 0;
        hash_table[i].list_ptr      = NULL;
    }
}

// Insert a Timer Object in the Hash Table
void insert_hash_entry(RTOS_TMR *timer_obj)
{
    // Calculate the index using Hash Function
    INT8U index = (timer_obj->RTOSTmrDelay+RTOSTmrTickCtr)%HASH_TABLE_SIZE;
    // Lock the Resources
    pthread_mutex_lock(&hash_table_mutex);
    // Add the Entry
    if (hash_table[index].list_ptr==NULL) {
        hash_table[index].list_ptr = timer_obj;
    }else{
        timer_obj->RTOSTmrNext = hash_table[index].list_ptr;
        hash_table[index].list_ptr->RTOSTmrPrev = timer_obj;
        hash_table[index].list_ptr = timer_obj;
    }
    ++hash_table[index].timer_count;
    // Unlock the Resources
    pthread_mutex_unlock(&hash_table_mutex);
}

// Remove the Timer Object entry from the Hash Table
void remove_hash_entry(RTOS_TMR *timer_obj)
{
    // Calculate the index using Hash Function
    INT8U index = (timer_obj->RTOSTmrDelay+RTOSTmrTickCtr)%HASH_TABLE_SIZE;
    // Lock the Resources
    pthread_mutex_lock(&hash_table_mutex);
    // Remove the Timer Obj
    if (hash_table[index].list_ptr==NULL) {
        return;
    }
    //timer_obj is the head of the list
    if (hash_table[index].list_ptr==timer_obj) {
        hash_table[index].list_ptr = timer_obj->RTOSTmrNext;
        if (hash_table[index].list_ptr!=NULL) {
            hash_table[index].list_ptr->RTOSTmrPrev = NULL;
        }
    }else{
        timer_obj->RTOSTmrPrev->RTOSTmrNext = timer_obj->RTOSTmrNext;
        if (timer_obj->RTOSTmrNext!=NULL) {
            timer_obj->RTOSTmrNext->RTOSTmrPrev = timer_obj->RTOSTmrPrev;
        }
    }
    --hash_table[index].timer_count;
    // Unlock the Resources
    pthread_mutex_unlock(&hash_table_mutex);
}

// Timer Task to Manage the Running Timers
void *RTOSTmrTask(void *temp)
{
    INT8U index = 0;
    while(1) {
        // Wait for the signal from RTOSTmrSignal()
        sem_wait(&timer_task_sem);
        // Once got the signal, Increment the Timer Tick Counter
        RTOSTmrTickCtr++;
        // Check the whole List associated with the index of the Hash Table
        for (INT8U i = 0; i<HASH_TABLE_SIZE; ++i) {
            RTOS_TMR *current = hash_table[i].list_ptr;
            while (current!=NULL) {
                if (current->RTOSTmrMatch==RTOSTmrTickCtr) {
                    current->RTOSTmrCallback(current->RTOSTmrCallbackArg);
                    if (current->RTOSTmrOpt==RTOS_TMR_PERIODIC) {
                        current->RTOSTmrMatch = current->RTOSTmrPeriod + RTOSTmrTickCtr;
                    }else{
                        remove_hash_entry(current);
                        current->RTOSTmrState = RTOS_TMR_STATE_COMPLETED;
                    }
                }
                current=current->RTOSTmrNext;

            }
        }
        // Compare each obj of linked list for Timer Completion
        // If the Timer is completed, Call its Callback Function, Remove the entry from Hash table
        // If the Timer is Periodic then again insert it in the hash table
        // Change the State
    }
    return temp;
}

// Timer Initialization Function
void RTOSTmrInit(void)
{
    INT32U timer_count = 0;
    INT8U	retVal;
    pthread_attr_t attr;

    fprintf(stdout,"Please Enter the number of Timers required in the Pool for the OS ");

    scanf("%d", &timer_count);

    // Create Timer Pool
    retVal = Create_Timer_Pool(timer_count);

    // Check the return Value
    if (retVal==RTOS_SUCCESS) {}
    // Init Hash Table
    init_hash_table();

    fprintf(stdout, "\n\nHash Table Initialized Successfully\n");

    // Initialize Semaphore for Timer Task
    sem_init(&timer_task_sem, 0, 0);
    // Initialize Mutex if any
    pthread_mutex_init(&hash_table_mutex, NULL);
    pthread_mutex_init(&timer_pool_mutex, NULL);

    // Create any Thread if required for Timer Task
    pthread_attr_init(&attr);
    pthread_create(&thread, &attr, &RTOSTmrTask, NULL);

    fprintf(stdout,"\nRTOS Initialization Done...\n");
}

// Allocate a timer object from free timer pool
RTOS_TMR* alloc_timer_obj(void)
{

    // Lock the Resources
    pthread_mutex_lock(&timer_pool_mutex);
    // Check for Availability of Timers
    RTOS_TMR *temp = NULL;
    if (FreeTmrCount>0 && FreeTmrListPtr!=NULL) {
        // Assign the Timer Object
        temp = FreeTmrListPtr;
        FreeTmrListPtr = FreeTmrListPtr->RTOSTmrNext;

        if (FreeTmrListPtr!=NULL) {
            FreeTmrListPtr->RTOSTmrPrev = NULL;
        }
        temp->RTOSTmrNext = NULL;
        temp->RTOSTmrPrev = NULL;
        --FreeTmrCount;
    }

    // Unlock the Resources
    pthread_mutex_unlock(&timer_pool_mutex);
    return temp;
}

// Free the allocated timer object and put it back into free pool
void free_timer_obj(RTOS_TMR *ptmr)
{
    // Lock the Resources
    pthread_mutex_lock(&timer_pool_mutex);
    // Clear the Timer Fields
    ptmr->RTOSTmrName = NULL;
    ptmr->RTOSTmrCallback=NULL;
    ptmr->RTOSTmrOpt = 0;

    // Change the State
    ptmr->RTOSTmrState = RTOS_TMR_STATE_UNUSED;
    // Return the Timer to Free Timer Pool
    ptmr->RTOSTmrNext = FreeTmrListPtr;
    ptmr->RTOSTmrPrev = NULL;
    if (FreeTmrListPtr!=NULL) {
        FreeTmrListPtr->RTOSTmrPrev = ptmr;
    }
    FreeTmrListPtr = ptmr;
    ++FreeTmrCount;
    // Unlock the Resources
    pthread_mutex_unlock(&timer_pool_mutex);
}

// Function to Setup the Timer of Linux which will provide the Clock Tick Interrupt to the Timer Manager Module
void OSTickInitialize(void) {
    time_t timer_id;
    struct timespec time_value;

    // Setup the time of the OS Tick as 100 ms after 3 sec of Initial Delay
    time_value.tv_sec = 0;
    time_value.tv_nsec = RTOS_CFG_TMR_TASK_RATE;

    time_value.tv_sec = 0;
    time_value.tv_nsec = RTOS_CFG_TMR_TASK_RATE;

    // Change the Action of SIGALRM to call a function RTOSTmrSignal()
    signal(SIGALRM, &RTOSTmrSignal);

    // Create the Timer Object
   // timer_create(CLOCK_REALTIME, NULL, &timer_id);
    clock_getres(CLOCK_REALTIME, &time_value);

    // Start the Timer
  //  timer_settime(timer_id, 0, &time_value, NULL);
    clock_settime(CLOCK_REALTIME,&time_value);
}
