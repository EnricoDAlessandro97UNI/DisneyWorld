#ifndef PROGETTOPMCSN_SIMULATOR_HELPER_H
#define PROGETTOPMCSN_SIMULATOR_HELPER_H

#include <stdio.h>
#include "rngs.h"

#define SEED 123456789
#define START 0.0       /* initial (open the door)                                          */

#ifndef F
#define STOP 36000.0    /* terminal (close the door) time  (Fascia1: 36000) */
#define INT 2.4     /* interarrivals (1/lambda1) */
#else
#define STOP 21600.0    /* terminal (close the door) time  (Fascia2: 21600) */
#define INT 4.32    /* interarrivals (1/lambda2) */
#endif 

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
#define FILENAME_AVGQUEUE_BLOCK1 "stats/avgqueue_block1.dat"
#define FILENAME_AVGNODE_BLOCK2 "stats/avgnode_block2.dat"
#define FILENAME_AVGQUEUE_BLOCK2 "stats/avgqueue_block2.dat"
#define FILENAME_AVGNODE_BLOCK3 "stats/avgnode_block3.dat"
#define FILENAME_AVGQUEUE_BLOCK3 "stats/avgqueue_block3.dat"
#define FILENAME_AVGNODE_BLOCK4 "stats/avgnode_block4.dat"
#define FILENAME_AVGQUEUE_BLOCK4 "stats/avgqueue_block4.dat"
#define FILENAME_AVGNODE_BLOCK5 "stats/avgnode_block5.dat"
#define FILENAME_AVGQUEUE_BLOCK5 "stats/avgqueue_block5.dat"


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