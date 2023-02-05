#ifndef PROGETTOPMCSN_FINITE_HELPER_H
#define PROGETTOPMCSN_FINITE_HELPER_H

#include <stdio.h>
#include "rngs.h"

#define REP 256
#define SEED 123456789
#define START 0.0       /* initial (open the door)                                          */
#define CHANGE 36000.0
#define SAMPLING 100.0
#define STOP 57600.0    /* terminal (close the door) time  (Fascia1: 36000, Fascia2: 21600) */
#define INT1 2.4        /* interarrivals (1/lambda1)                                          */
#define INT2 4.32       /* interarrivals (1/lambda2)                                          */

#define MAX_SERVERS 100

#define ONLINE_PROBABILITY 0.6
#define DISABLED_PROBABILITY 0.25
#define STORAGE_PROBABILITY 0.3
#define LOST_PROBABILITY 0.9

#define FILENAME_WAIT_GLOBAL "stats/wait_global.dat"
#define FILENAME_WAIT_BLOCK1 "stats/wait_block1.dat"
#define FILENAME_DELAY_BLOCK1 "stats/delay_block1.dat"
#define FILENAME_WAIT_BLOCK2 "stats/wait_block2.dat"
#define FILENAME_DELAY_BLOCK2 "stats/delay_block2.dat"
#define FILENAME_WAIT_BLOCK3 "stats/wait_block3.dat"
#define FILENAME_DELAY_BLOCK3 "stats/delay_block3.dat"
#define FILENAME_WAIT_BLOCK4 "stats/wait_block4.dat"
#define FILENAME_DELAY_BLOCK4 "stats/delay_block4.dat"
#define FILENAME_WAIT_BLOCK5 "stats/wait_block5.dat"
#define FILENAME_DELAY_BLOCK5 "stats/delay_block5.dat"
#define FILENAME_AVGNODE_BLOCK1 "stats/avgnode_block1.dat"
#define FILENAME_AVGNODE_BLOCK2 "stats/avgnode_block2.dat"
#define FILENAME_AVGNODE_BLOCK3 "stats/avgnode_block3.dat"
#define FILENAME_AVGNODE_BLOCK4 "stats/avgnode_block4.dat"
#define FILENAME_AVGNODE_BLOCK5 "stats/avgnode_block5.dat"

#define FILENAME_WAIT_BLOCK4_CLASS1 "stats/wait_block4_class1.dat"
#define FILENAME_DELAY_BLOCK4_CLASS1 "stats/delay_block4_class1.dat"
#define FILENAME_WAIT_BLOCK4_CLASS2 "stats/wait_block4_class2.dat"
#define FILENAME_DELAY_BLOCK4_CLASS2 "stats/delay_block4_class2.dat"
#define FILENAME_AVGNODE_BLOCK4_CLASS1 "stats/avgnode_block4_class1.dat"
#define FILENAME_AVGNODE_BLOCK4_CLASS2 "stats/avgnode_block4_class2.dat"

/* Structure used to handle global event times */
typedef struct global_info {
    double time;    /* Event time */
    int eventType;  /* Event type: 0 arrival in the block, 1 departure from the block */
    int disabled;
} global_info;

/* Structure used to return the departure info to the orchestrator */
typedef struct departure_info {
    int blockNum;   /* Block from which there was a departure */
    double time;    /* Departure time */
    int disabled;
} departure_info;

/* -------------- GLOBAL VARIABLES -------------- */
extern global_info globalInfo[8];
extern departure_info departureInfo;

extern int block4Lost;
extern int block4ToExit;

extern int endSimulation;
extern int changeConfig;

extern double glblWaitBlockOne;
extern double glblWaitBlockTwo;
extern double glblWaitBlockThree;
extern double glblWaitBlockFour;
extern double glblWaitBlockFive;

extern int sampling;
/* ---------------------------------------------- */

double Exponential(double);
double Uniform(double, double);

void create_statistics_files();
void init_global_info_structure();
double get_external_arrival(double currentTime, double interarrival);
int get_next_event();
int get_next_event_type(int blockNum);
double get_next_event_time(int blockNum);
int get_next_event_disabled(int blockNum);
void update_next_event(int blockNum, double time, int eventType, int disabled);
double get_probability();

#endif 