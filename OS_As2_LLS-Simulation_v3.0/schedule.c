//
//  scheduler.c
//  OS_As2_LLS-Simulation_v3.0
//
//  Created by Matthew Mansell on 11/05/2017.
//  Copyright © 2017 Matthew Mansell. All rights reserved.
//
//  ################################################################################
//  ROUND ROBIN SCHEDULER
//  Contains the main RR LLS Scheduler function.
//  ################################################################################

#include <stdio.h>
#include "header.h"

//  SIMULATE QUEUES
ProcQueue runQueue[MAX_PROCESSES];
int runItems=0;

//  ################################################################################
//                                  UTILITY FUNCTIONS
//  ################################################################################
// ADD PROCESS TO RUN QUEUE
int queAddProc(PCB *addr) {
    //Check process doesnt already exist
    int i;
    for(i=0;i<=runItems-1;i++) {
        if(runQueue[i].procLoc==addr) {
            //  Found the process
            //No point in continuing
            return 0;
        }
    }    //  Process not found
    //Add process to next queue slot
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        if(runQueue[i].procLoc==0) {
            //  Found empty location
            //Add process
            runQueue[i].procLoc=addr;
            printf("%d: Process |%s| Queued\n",lifetime,(*addr).id);
            //Increment queue number
            runItems++;
            //Stop method
            return 1;
        }
    }
    //If we get here, the process wasnt added
    return 0;
}

//  SHIFT QUEUE FORWARD FROM A POINT
int queShift(int point) {
    //Shifts queue forward from point specified
    //Cant shift forward from 0th element or point larger than queue
    if(point==0||point>MAX_PROCESSES-1) {
        //Force method quit;
        return 0;
    }
    int i;
    for(i=point;i<=MAX_PROCESSES-1;i++) {
        //Move variables into the element before
        runQueue[i-1].procLoc=runQueue[i].procLoc;
        runQueue[i-1].waitingTime=runQueue[i].waitingTime;
        //Clean current element variables
        runQueue[i].procLoc=0;
        runQueue[i].waitingTime=0;
    }
    return 1;
}

//  REMOVE PROCESS FROM RUN QUEUE
int queRemoveProc(PCB *addr) {
    //Loop queue until we find process
    int i;
    for(i=0;i<=runItems-1;i++) {
        if(runQueue[i].procLoc==addr) {
            //  Found process
            if(i==MAX_PROCESSES-1) {
                //  Its the last element
                //Just blank its data
                runQueue[i].procLoc=0;
                runQueue[i].waitingTime=0;
            } else {
                //  Its not the last element
                //Shift queue
                queShift(i+1);
            }
            //Lower process items
            runItems--;
            //Quit
            return 1;
        }
    }
    //Cant have found process
    return 0;
}

//  AUTO REMOVES NULL PROCESSES FROM QUEUE
//  A check in case processes have been forcebly removed before completion
void queueAutoRemove() {
    //Loop through all processes in queue checking state is ready
    //Have to step down due to process removal queue shifting
    int i;
    for(i=runItems-1;i>=runItems;i--) {
        if((*runQueue[i].procLoc).status==STAT_EMPTY) {
            printf("%d: |%s| Removed From Queue",lifetime,(*runQueue[i].procLoc).id);
            queRemoveProc(runQueue[i].procLoc);
        }
    }
}

//  AUTO POPULATE RUN QUEUE
//  Adds READY processes to queue if they are not already
void queAutoAdd(void) {
    //Loop through processes and check they are in queue
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        //Only checking process if its in READY state
        if(process[i].status==STAT_READY) {
            //queAddProc already checks for pre-existance so attempt to add it
            if(queAddProc(&process[i])==1) {
                //Process added
            } else {
                //Process wasnt added
            }
        }
    }
}

//  ################################################################################
//                                  MAIN SCHEDULER
//  ################################################################################
void schedule() {
    //Auto remove EMPTY processes, blocked processes removed later
    queueAutoRemove();
    if(runQueue[0].procLoc==0) {
        //There is no process to run, try adding any new processes and switch to it
        //The processor will call this repeatedly is context remains NULL
        queAutoAdd();
        switch_to(runQueue[0].procLoc);
    } else {
        //  We have a process in the queue
        //Auto add any now processes
        queAutoAdd();
        //Check the state of the process in 0
        //We assume that if the process is still ready, it did not complete and needs
        //to be re-scheduled. If the process is now blocked or completed we need to
        //remove it from the queue.
        
        PCB *proc=runQueue[0].procLoc;
        if((*proc).status==STAT_READY) {
            //  Process is still ready
            //Move it to back of queue by removing and re-adding
            queRemoveProc(proc);
            queAddProc(proc);
        } else {
            //  Process is no longer ready
            //Remove it from queue
            queRemoveProc(proc);
        }
        //Increment all wait times
        //Return process to run
        switch_to(runQueue[0].procLoc);
    }
}
