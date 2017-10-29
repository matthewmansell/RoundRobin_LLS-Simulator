//
//  header.h
//  OS_As2_LLS-Simulation_v3.0
//
//  Created by Matthew Mansell on 11/05/2017.
//  Copyright © 2017 Matthew Mansell. All rights reserved.
//
//  ################################################################################
//  HEADER FILE
//  Contains all global variables and function definitions to be used by the
//  simulator.
//  ################################################################################

//  Prevent multiple header inclusion error
#ifndef header_h
#define header_h

//  FIXED SYSTEM SETTINGS
#define MAX_PROCESSES 10
#define QUANTUM 100
#define IO_BURST 50
#define IO_COMPLETION 200
#define ID_LENGTH 20
#define MAX_INPUT 256

//  VARIABLE SIMULATOR SETTINGS
extern float SIM_SPEED;

//  STATUS DEFINITIONS
#define STAT_EMPTY 0
#define STAT_READY 1
#define STAT_BLOCKED 2
#define STAT_COMPLETE 3

//  TYPE DEFINITIONS
#define TYP_IO 1
#define TYP_CPU 2

//  STRUCT DEFINITIONS
typedef struct pcb {
    char id[ID_LENGTH];
    int status;
    int type;
    int estRunTime;
    int cpuTime;
    int turnaroundTime;
} PCB;
typedef struct procQueue {
    PCB *procLoc;
    int waitingTime;
} ProcQueue;

//  VARIABLES THAT REQUIRE GLOBAL ACCESS
extern int lifetime;
extern PCB process[MAX_PROCESSES];
extern PCB *context;
extern ProcQueue runQueue[MAX_PROCESSES];
extern int runItems;
extern ProcQueue blockedQueue[MAX_PROCESSES];

//  SYSTEM FUNCTIONS REQUIRING ACCESS
PCB * procGetAddr(char id[ID_LENGTH]);
extern int procExists(PCB *addr);
extern int procCreate(char id[ID_LENGTH],int type,int totalTime);
extern int procRemove(PCB *addr);
extern int procRemoveAll(void);
extern int switch_to(PCB *proc);
extern int runBurst(void);

//  SCHEDULER FUNCTION
extern void schedule(void);

#endif /* header_h */
