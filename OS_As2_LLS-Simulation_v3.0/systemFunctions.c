//
//  systemFunctions.c
//  OS_As2_LLS-Simulation_v3.0
//
//  Created by Matthew Mansell on 11/05/2017.
//  Copyright © 2017 Matthew Mansell. All rights reserved.
//
//  ################################################################################
//  PROCESS FUNCTIONS
//  Contains funtions for simulating other system functions from the perspective
//  of the scheduler. These include:
//  - Process creation and submission to system by HLS
//  - IO handling
//  - The processor and interupt handler
//  ################################################################################

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "header.h"

//  DEFINITIONS
PCB process[MAX_PROCESSES];
ProcQueue blockedQueue[MAX_PROCESSES];
PCB *context;
float SIM_SPEED=1;


//  ################################################################################
//                                  UTILITY FUNCTIONS
//  ################################################################################
//  GET PROCESS ADDRESS
PCB * procGetAddr(char id[ID_LENGTH]) {
    //Loop until process found
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        if(strcmp(process[i].id,id)==0) {
            //  Found process
            //Return process address
            return &process[i];
        }
    }
    //  Loop exited without return
    //Process ID does not exist -> return NULL
    return NULL;
}

//  CHECK PROCESS EXISTENCE
int procExists(PCB *addr) {
    if((*addr).status!=STAT_EMPTY) {
        //  Process ID not empty
        //Return true
        return 1;
    } else {
        //  Process ID empty
        //Return false
        return 0;
    }
}

//  CREATE PROCESS
int procCreate(char id[ID_LENGTH],int type,int estRunTime) {
    //Check ID is unique
    if(procGetAddr(id)!=NULL) {
        //  Process with that ID already existed
        //Inform user
        printf("ERROR: Process Not Created -> A process already exists with that ID\n");
        return 0;
    } else {
        //  Process ID not in use
        ////Find empty memory process list slot
        int i;
        for(i=0;i<=MAX_PROCESSES-1;i++) {
            //Test empty
            if(procExists(&process[i])==0) {
                //  Found empty space
                //Load values here
                strcpy(process[i].id,id);
                process[i].type=type;
                process[i].estRunTime=estRunTime;
                //Switch the status so its included in queue
                process[i].status=STAT_READY;
                //Return complete
                //Increment process number
                printf("%d: Process |%s| Created\n",lifetime,id);
                return 1;
            }
        }
        printf("ERROR: Process Not Created -> Maximum processes reached\n");
        return 0;
    }
}

//  REMOVE SINGLE PROCESS
int procRemove(PCB *addr) {
    //Only prints removed if its a process
    if(procExists(addr)) {
        printf("%d: Process |%s| Removed\n",lifetime,(*addr).id);
    }
    //Yet clear the address anyway
    strcpy((*addr).id,"");
    (*addr).status=STAT_EMPTY;
    (*addr).type=0;
    (*addr).estRunTime=0;
    (*addr).cpuTime=0;
    (*addr).turnaroundTime=0;
    return 1;
}

//  REMOVE ALL PROCESSES
int procRemoveAll(void) {
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        procRemove(&process[i]);
    }
    return 1;
}

//  UTILITY TO INCREMENT ALL READY OR BLOCKED PROCESS LIFETIMES
void procIncLifetime(int time) {
    //Iterate all prococesses
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        //If we want to increment
        if(process[i].status==STAT_READY||process[i].status==STAT_BLOCKED) {
            process[i].turnaroundTime+=time;
        }
    }
}

//  CONTEXT SWITCH SYSTEM CALL
int switch_to(PCB *addr) {
    context=addr;
    return 1;
}

//  ################################################################################
//                                  IO INTERUPT MANAGEMENT
//  ################################################################################
//  AUTO POPULATE BLOCKED LIST
//  Automatically adds new blocked processes to blockedQueue
void ioAutoAdd() {
    //Loop through processes and check if they are in the queue
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        //Only check BLOCKED processes
        if(process[i].status==STAT_BLOCKED) {
            //Check if in blocked queue aleady
            int i2,exists=0;
            for(i2=0;i2<=MAX_PROCESSES-1;i2++) {
                if(blockedQueue[i2].procLoc==&process[i]) {
                    //  Process already in queue
                    exists=1;
                    //Ensure loop quits
                    i2=MAX_PROCESSES;
                }
            }
            //If process didnt exist add it

            if(exists!=1) {
                //Find available location
                for(i2=0;i2<=MAX_PROCESSES-1;i2++) {
                    if(blockedQueue[i2].procLoc==0) {
                        //  Found empty location
                        //Add process
                        blockedQueue[i2].procLoc=&process[i];
                        //Ensure loop quits
                        i2=MAX_PROCESSES;
                    }
                }
            }
        }
    }
}

//  RESOLVE INTERUPT BLOCK/WAIT
//  Causes a process to be re-entered into the queue
int ioResolve(PCB *addr) {
    (*addr).status=STAT_READY;
    printf("%d: Process |%s| IO Block Resolved\n",lifetime,(*addr).id);
    //Remove process from list
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        if(blockedQueue[i].procLoc==addr) {
            //Clear values
            blockedQueue[i].procLoc=NULL;
            blockedQueue[i].waitingTime=0;
        }
    }
    return 0;
}

//  AUTO INTERUPT MANAGER
//  Automatically decides the state of a process based on its type
//  Can interuprt processor
void ioManage(int time) {
    //Increment wait times, checking if any have been resolved in the mean time
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        //Make sure process exists in this location
        if(blockedQueue[i].procLoc!=NULL) {
            //Add processor time to blocked wait times
            blockedQueue[i].waitingTime+=time;
            //Check if IO complete
            if(blockedQueue[i].waitingTime>=IO_COMPLETION) {
                //io is complete for this process
                ioResolve(blockedQueue[i].procLoc);
            }
        }
    }
    //Attempt to add any new BLOCKED processes
    ioAutoAdd();
}

//  ################################################################################
//                                  PROCESSOR SIMULATION
//  ################################################################################
//  RUNS INPUT NO. CPU BURSTS
int runBurst() {
    //Get process to run
    //Check system has context, and context reference is good
    if(context!=NULL&&procExists(context)!=0) {
        //  There is a process to run
        //Processor stops at quntum, process completion or io interupt
        int ms,io=0;
        printf("%d: Running process |%s|...\n",lifetime,(*context).id);
        for(ms=1;ms<=QUANTUM&&(*context).estRunTime!=(*context).cpuTime&&io==0&&io==0;ms++) {
            //Simulate processing time of 1 millisecond
            usleep(1000/SIM_SPEED);
            //Increase run time of process
            (*context).cpuTime++;
            //Increase process turnaround
            //Simulate io request for io type
            if((*context).type==TYP_IO&&ms==IO_BURST) {
                //Have reached max IO burst
                io=1;
            }
            lifetime++;
            ioManage(1);
        }
        procIncLifetime(ms-1);
        //Check reason for quit
        if ((*context).estRunTime==(*context).cpuTime) {
            //  Quit becuase process completed
            printf("%d: Process |%s |Complete\n",lifetime,(*context).id);
            //Change process state
            (*context).status=STAT_COMPLETE;
        } else if(ms-1==QUANTUM) {
            //  Quit because quantum value reached
        } else if (io!=0) {
            //  Quit because of io interupt
            printf("%d: Process |%s| Blocked On IO\n",lifetime,(*context).id);
            //Change process state
            (*context).status=STAT_BLOCKED;
        }
        //Run io manager once more
        ioManage(0);
        //Run scheduler
        schedule();
        //Continue
        return 1;
    } else {
        //  No Runnable Process
        //It could be the case that there are processes that need to be run, yet
        //are in the BLOCKED state.
        //Check for processes
        int i;
        for(i=0;i<=MAX_PROCESSES-1;i++) {
            if(process[i].status==STAT_BLOCKED) {
                //  Found a process that still needs running
                //Loop timer to run ioManager and scheduler until the scheduler
                //adds a process to context
                printf("%d: Waiting for process...\n",lifetime);
                while(context==NULL) {
                    //Simulate time
                    usleep(1000/SIM_SPEED);
                    lifetime++;
                    procIncLifetime(1);
                    ioManage(1);
                    schedule();
                }
                //Ensure loop exits
                i=MAX_PROCESSES;
                //Continue
                return 1;
            } else if(process[i].status==STAT_READY) {
                //Schedule the process imediately if its ready
                //queAutoAdd();
                schedule();
                return 1;
            }
        }
        //There are no processes
        printf("%d: No more processes\n",lifetime);
        //Discontinue
        return 0;
    }
}
