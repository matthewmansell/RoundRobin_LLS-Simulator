//
//  main.c
//  OS_As2_LLS-Simulation_v3.0
//
//  Created by Matthew Mansell on 10/05/2017.
//  Copyright © 2017 Matthew Mansell. All rights reserved.
//
//  ################################################################################
//  MAIN FILE
//  Contains the main run time of the simulator. Largely relating to the user
//  interface and general operation. Includes
//  - Display functions
//  - Simulation controller
//  - Various demos
//  - Main (method w\) menu/command input loop
//  ################################################################################

#include <stdio.h>
#include <string.h>
#include "header.h"

//Simulator Lifetime
int lifetime=0;

//  ################################################################################
//                                  DISPLAY FUNCTIONS
//  ################################################################################
//  DISPLAY HELP
void displayHelp(void) {
    printf("\nHELP - All Commands\n");
    printf("(\"/\"choice     \"{}\"=optional    \"[]\"=input)\n");
    printf("demo 1/2/3/4/5                              - Runs the input demo/test simulation\n");
    printf("display processes/queues                    - Displays the processes or queues\n");
    printf("exit                                        - Quits the application\n");
    printf("lifetime {reset}                            - Displays the lifetime, with optional reset\n");
    printf("process create [Proc_ID] io/cpu [Exec_Time] - Creates a process with the input values\n");
    printf("process remove [Proc_ID]/all                - Removes the input process, or all processes\n");
    printf("burst {[Bursts]}                            - Runs a single CPU burst, or for value input\n");
    printf("simspeed {[Speed]}                          - Displays the speed multiplier, or sets it\n");
}

//  DISPLAY PROCESSES
void displayProcesses(void) {
    printf("\nProcesses:\n");
    int number=0;
    //Loop processes
    int i;
    for(i=0;i<=MAX_PROCESSES-1;i++) {
        if(procExists(&process[i])) {
            number++;
            char status[MAX_INPUT],type[MAX_INPUT];
            //Convert state
            if(process[i].status==STAT_READY) {
                strcpy(status,"READY");
            } else if(process[i].status==STAT_BLOCKED) {
                strcpy(status,"BLOCKED");
            } else if(process[i].status==STAT_COMPLETE) {
                strcpy(status,"COMPLETE");
            } else {
                strcpy(status,"EMPTY");
            }
            //Convert types
            if(process[i].type==TYP_IO) {
                strcpy(type,"IO");
            } else {
                strcpy(type,"CPU");
            }
            printf("Process |%s| - Status|%s| Type|%s| EstRunTime |%d| CPUTime|%d| TurnaroundTime|%d|\n",process[i].id,status,type,process[i].estRunTime,process[i].cpuTime,process[i].turnaroundTime);
        }
    }
    if(number==0) {
        printf("No Processes To Display\n");
    }
    printf("\n");
}

//  DISPLAY QUEUES
void displayQueues(void) {
    printf("\n Ready Queue:\n");
    //Loop ready queue backwards
    int i;
    for(i=MAX_PROCESSES-1;i>=0;i--) {
        //Make sure que item contains data
        if(runQueue[i].procLoc!=0) {
            //Get proc location
            PCB *proc=runQueue[i].procLoc;
            printf("|%s: %d/%d|  ",(*proc).id,(*proc).cpuTime,(*proc).estRunTime);
        } else {
            printf("|NULL|  ");
        }
    }
    printf("\n Blocked Processes:\n");
    //Loop blocked queue backwards
    for(i=MAX_PROCESSES-1;i>=0;i--) {
        //Make sure que item contains data
        if(blockedQueue[i].procLoc!=0) {
            //Get proc location
            PCB *proc=blockedQueue[i].procLoc;
            printf("|%s: %d/%d|  ",(*proc).id,(*proc).cpuTime,(*proc).estRunTime);
        } else {
            printf("|NULL|  ");
        }
    }
    printf("\n\n");
}

//  ################################################################################
//                                  SIMULATOR CONTROLLER
//  ################################################################################
void runSimulator(int bursts) {
    int continueSim=1;
    
    //Run simulator for set bursts
    int i;
    for(i=1;i<=bursts&&continueSim==1;i++) {
        //Run number of bursts
        continueSim=runBurst();
        displayQueues();
    }
}


//  ################################################################################
//                                       DEMO
//  ################################################################################
void runDemo(int demo) {
    switch(demo) {
        case 1:
            procRemoveAll();
            lifetime=0;
            procCreate("a",TYP_CPU,1000);
            procCreate("b",TYP_CPU,1000);
            procCreate("c",TYP_CPU,1000);
            procCreate("d",TYP_CPU,1000);
            procCreate("e",TYP_CPU,1000);
            displayProcesses();
            runSimulator(100);
            displayProcesses();
            break;
        case 2:
            procRemoveAll();
            lifetime=0;
            procCreate("a",TYP_CPU,1000);
            procCreate("b",TYP_CPU,1000);
            procCreate("c",TYP_CPU,200);
            procCreate("d",TYP_CPU,200);
            displayProcesses();
            runSimulator(100);
            displayProcesses();
            break;
        case 3:
            procRemoveAll();
            lifetime=0;
            procCreate("a",TYP_CPU,400);
            procCreate("b",TYP_CPU,400);
            displayProcesses();
            runSimulator(5);
            procCreate("c",TYP_CPU,400);
            procCreate("d",TYP_CPU,400);
            runSimulator(100);
            displayProcesses();
            break;
        case 4:
            procRemoveAll();
            lifetime=0;
            procCreate("a",TYP_IO,200);
            procCreate("b",TYP_CPU,600);
            displayProcesses();
            runSimulator(100);
            displayProcesses();
            break;
        case 5:
            procRemoveAll();
            lifetime=0;
            procCreate("a",TYP_CPU,1000);
            procCreate("b",TYP_IO,200);
            procCreate("c",TYP_CPU,1500);
            procCreate("d",TYP_CPU,800);
            procCreate("e",TYP_IO,100);
            procCreate("f",TYP_IO,250);
            procCreate("g",TYP_CPU,1250);
            procCreate("h",TYP_CPU,500);
            procCreate("i",TYP_IO,225);
            procCreate("j",TYP_CPU,1400);
            displayProcesses();
            runSimulator(200);
            displayProcesses();
            break;
        default:
            printf("ERROR: No Such Demo -> Try 1,2,3,4 or 5\n");
    }
    
}


//  ################################################################################
//                                      MAIN & MENU
//  ################################################################################
int main(void) {
    //Opening Message
    printf("Round Robin - Low Level Scheduler Simulation v3.0\nby Matthew Mansell\n");
    //Main runtime
    int exit=0;
    do {
        //Entire input
        char input[MAX_INPUT];
        //6 total possible command inputs
        char cmd1[20]="";
        char cmd2[20]="";
        char cmd3[20]="";
        char cmd4[20]="";
        char cmd5[20]="";
        //Variable to store command handled?
        int cmdHandled=0;
        //Scan for input
        fgets(input,MAX_INPUT,stdin);
        //Split input into commands
        sscanf(input,"%s%s%s%s%s",cmd1,cmd2,cmd3,cmd4,cmd5);
        
        //  CHECK COMMANDS AGAINST FIRST OPTIONS
        //HELP
        if(strcmp(cmd1,"help")==0) {
            //  Command was help
            cmdHandled=1;
            //Display help
            displayHelp();
        }
        //DEMO
        if(strcmp(cmd1,"demo")==0) {
            //  Command was demo
            cmdHandled=1;
            if(strcmp(cmd2,"")!=0) {
                //  Second command entered
                //Get int from it
                int x;
                sscanf(cmd2,"%d",&x);
                runDemo(x);
            } else {
                printf("ERROR: 'demo' requires additional command/s -> [1,2,3,4,5]\n");
            }
        }
        //EXIT
        if(strcmp(cmd1,"exit")==0) {
            //  Command was exit
            cmdHandled=1;
            //Set to exit
            exit=1;
        }
        //DISPLAY
        if(strcmp(cmd1,"display")==0) {
            //  Command was display
            cmdHandled=1;
            //Find selection
            if(strcmp(cmd2,"processes")==0) {
                //  Command was display processes
                displayProcesses();
            } else if(strcmp(cmd2,"queues")==0) {
                //  Command was display queues
                displayQueues();
            } else {
                //  Second command not recognised
                printf("ERROR: 'display' requires additional command/s -> processes/queues\n");
            }
            
        }
        //BURST
        if(strcmp(cmd1,"burst")==0) {
            //  Command was burst
            cmdHandled=1;
            //See if burst value set
            if(strcmp(cmd2,"")==0) {
                //No 2nd command
                runSimulator(1);
            } else {
                //Get the number from command 2
                int x;
                sscanf(cmd2,"%d",&x);
                runSimulator(x);
            }
        }
        //LIFETIME
        if(strcmp(cmd1,"lifetime")==0) {
            //  Command was burst
            cmdHandled=1;
            //See if burst value set
            if(strcmp(cmd2,"")==0) {
                //Display lifetime
                printf("Lifetime: %d\n",lifetime);
            } else if(strcmp(cmd2,"reset")==0) {
                //Reset lifetime
                lifetime=0;
                printf("Lifetime Reset\n");
            } else {
                //Unknown
                printf("ERROR: Unknown command for 'lifetime' -> lifetime {reset}\n");
            }
        }
        //SIMSPEED
        if(strcmp(cmd1,"simspeed")==0) {
            //  Command was simspeed
            cmdHandled=1;
            //See if burst value set
            if(strcmp(cmd2,"")==0) {
                //Display lifetime
                printf("Simspeed: %f\n",SIM_SPEED);
            } else {
                //Extract new value
                float x;
                sscanf(cmd2,"%f",&x);
                //Cant be 0
                if(x!=0) {
                    SIM_SPEED=x;
                    printf("Simspeed set: %f\n",SIM_SPEED);
                } else {
                    printf("ERROR: Simspeed not changed -> Cannot be 0 or char\n");
                }
            }
        }
        //PROCESS
        if(strcmp(cmd1,"process")==0) {
            //  Command was process
            cmdHandled=1;
            //Need additional commands
            if(strcmp(cmd2,"create")==0) {
                //  Get additional commamds
                if(strcmp(cmd3,"")!=0&&strcmp(cmd4,"")!=0&&strcmp(cmd5,"")!=0) {
                    //  Attempt to get values from inputs
                    char id[ID_LENGTH];
                    int type=0,execTime;
                    //Copy id
                    strcpy(id,cmd3);
                    //Extract execTime as int
                    sscanf(cmd5,"%d",&execTime);
                    //Get type from string
                    if(strcmp(cmd4,"io")==0) {
                        type=TYP_IO;
                        //Attempt to create process
                        procCreate(id, type, execTime);
                    } else if(strcmp(cmd4,"cpu")==0) {
                        type=TYP_CPU;
                        //Attempt to create process
                        procCreate(id, type, execTime);
                    } else {
                        printf("ERROR: Non valid type -> io/cpu\n");
                    }
                } else {
                    printf("ERROR: 'process create' requires additional command/s -> [Proc_ID] io/cpu [Exec_Time]\n");
                }
            } else if(strcmp(cmd2,"remove")==0) {
                //  Either need cmd3 to be all or a value
                if(strcmp(cmd3,"all")==0) {
                    //Key word all
                    procRemoveAll();
                } else if(strcmp(cmd3,"")!=0) {
                    //Remove process with input that id
                    if(procGetAddr(cmd3)!=NULL) {
                        procRemove(procGetAddr(cmd3));
                    } else {
                        printf("ERROR: Cant Remove Process -> Process does not exist\n");
                    }
                } else {
                    printf("ERROR: 'process remove' requires additional command/s -> [Proc_ID]/all\n");
                }
            } else {
                //  2nd command not understood
                printf("ERROR: 'process' requires additional command/s -> create/remove\n");
            }
        }
        //CHECK COMMAND WAS HANDLED
        if(cmdHandled!=1) {
            printf("ERROR: Unrecognised Command -> 'help' for list of valid commands\n");
        }

        
    } while(exit!=1);
    //Program exit
    printf("EXITING...\n");
    
    return 0;
}
