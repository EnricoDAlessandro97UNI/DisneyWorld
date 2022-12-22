#ifndef PROGETTOPMCSN_ORCHESTRATOR_HELPER_H
#define PROGETTOPMCSN_ORCHESTRATOR_HELPER_H

#include <stdio.h>
#include "rngs.h"

#define REP 3     /* number of repetitions for each interval */
#define K 128       /* number of batches */
#define B 1024      /* sampling interval */
#define N 131072    /* number of jobs to process (N=K*B) */

#define SEED 123456789
#define START 0.0       /* initial (open the door) */

#ifndef F
#define INT 2.4     /* interarrivals (1/lambda1) */
#else
#define INT 4.32    /* interarrivals (1/lambda2) */
#endif   

#define ONLINE_PROBABILITY 0.6
#define DISABLED_PROBABILITY 0.2
#define STORAGE_PROBABILITY 0.3
#define LOST_PROBABILITY 0.9

#define FILENAME_WAIT_GLOBAL "wait_global.dat"
#define FILENAME_WAIT_BLOCK1 "block1_tickets/wait_block1.dat"
#define FILENAME_DELAY_BLOCK1 "block1_tickets/delay_block1.dat"
#define FILENAME_WAIT_BLOCK2 "block2_dis/wait_block2.dat"
#define FILENAME_DELAY_BLOCK2 "block2_dis/delay_block2.dat"
#define FILENAME_WAIT_BLOCK3 "block3_norm/wait_block3.dat"
#define FILENAME_DELAY_BLOCK3 "block3_norm/delay_block3.dat"
#define FILENAME_WAIT_BLOCK4 "block4_contr/wait_block4.dat"
#define FILENAME_DELAY_BLOCK4 "block4_contr/delay_block4.dat"
#define FILENAME_WAIT_BLOCK5 "block5_storage/wait_block5.dat"
#define FILENAME_DELAY_BLOCK5 "block5_storage/delay_block5.dat"


/* Structure used to handle global event times */
typedef struct global_info {
    double time;    /* Event time */
    int eventType;  /* Event type: 0 arrival in the block, 1 departure from the block */
} global_info;

/* Structure used to return the departure info to the orchestrator */
typedef struct departure_info {
    int blockNum;   /* Block from which there was a departure */
    double time;    /* Departure time */
} departure_info;

/* -------------- GLOBAL VARIABLES -------------- */
extern global_info globalInfo[6];
extern departure_info departureInfo;

extern int block4Lost;
extern int block4ToExit;

extern int endSimulation;

extern double glblWaitBlockOne;
extern double glblWaitBlockTwo;
extern double glblWaitBlockThree;
extern double glblWaitBlockFour;
extern double glblWaitBlockFive;
/* ---------------------------------------------- */

double Exponential(double);
double Uniform(double, double);

void create_statistics_files();
void init_global_info_structure();
double get_external_arrival(double currentTime, double interarrival);
int get_next_event();
int get_next_event_type(int blockNum);
double get_next_event_time(int blockNum);
void update_next_event(int blockNum, double time, int eventType);
double get_probability();

#endif 