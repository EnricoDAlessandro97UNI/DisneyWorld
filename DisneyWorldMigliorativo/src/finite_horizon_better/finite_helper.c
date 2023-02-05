/**
 * Authors: Alessandro De Angelis & Enrico D'Alessandro
 * 
 */
#include <math.h>
#include <stdlib.h>
#include "rngs.h"
#include "finite_helper.h"


double Exponential(double m) {
    return (-m * log(1.0 - Random()));
}

double Uniform(double a, double b) {
    return (a + (b - a) * Random());
}

double get_external_arrival(double arrivalBlockOne, double interarrival) {   
    SelectStream(0);
    arrivalBlockOne += Exponential(interarrival);
    return arrivalBlockOne;
}

/* Create statistics files */
void create_statistics_files() {
    FILE *fp;
    fp = fopen(FILENAME_WAIT_GLOBAL, "w");
    fclose(fp);
    fp = fopen(FILENAME_DELAY_BLOCK1, "w");
    fclose(fp);
    fp = fopen(FILENAME_WAIT_BLOCK1, "w");
    fclose(fp);
    fp = fopen(FILENAME_DELAY_BLOCK2, "w");
    fclose(fp);
    fp = fopen(FILENAME_WAIT_BLOCK2, "w");
    fclose(fp);
    fp = fopen(FILENAME_DELAY_BLOCK3, "w");
    fclose(fp);
    fp = fopen(FILENAME_WAIT_BLOCK3, "w");
    fclose(fp);
    fp = fopen(FILENAME_DELAY_BLOCK4, "w");
    fclose(fp);
    fp = fopen(FILENAME_WAIT_BLOCK4, "w");
    fclose(fp);
    fp = fopen(FILENAME_DELAY_BLOCK5, "w");
    fclose(fp);
    fp = fopen(FILENAME_WAIT_BLOCK5, "w");
    fclose(fp);
    fp = fopen(FILENAME_AVGNODE_BLOCK1, "w");
    fclose(fp);
    fp = fopen(FILENAME_AVGNODE_BLOCK2, "w");
    fclose(fp);
    fp = fopen(FILENAME_AVGNODE_BLOCK3, "w");
    fclose(fp);
    fp = fopen(FILENAME_AVGNODE_BLOCK4, "w");
    fclose(fp);
    fp = fopen(FILENAME_AVGNODE_BLOCK5, "w");
    fclose(fp);

    fp = fopen(FILENAME_DELAY_BLOCK4_CLASS1, "w");
    fclose(fp);
    fp = fopen(FILENAME_WAIT_BLOCK4_CLASS1, "w");
    fclose(fp);
    fp = fopen(FILENAME_DELAY_BLOCK4_CLASS2, "w");
    fclose(fp);
    fp = fopen(FILENAME_WAIT_BLOCK4_CLASS2, "w");
    fclose(fp);
    fp = fopen(FILENAME_AVGNODE_BLOCK4_CLASS1, "w");
    fclose(fp);
    fp = fopen(FILENAME_AVGNODE_BLOCK4_CLASS2, "w");
    fclose(fp);
}

/* Initializes the initial state of the global_info structure  */
void init_global_info_structure() {
    /* Initializes first block */
    globalInfo[0].time = 0;         /* The first block must have the most immininent event */
    globalInfo[0].eventType = 0;    /* At the beginning all the blocks await an arrival */
    globalInfo[0].disabled = -1;

    globalInfo[6].time = SAMPLING;  /* Event update stats */
    globalInfo[6].eventType = -1;   
    globalInfo[6].disabled = -1;

    globalInfo[7].time = CHANGE;    /* Event change time slot */
    globalInfo[7].eventType = -1;
    globalInfo[7].disabled = -1;

    /* Initializes other blocks */
    for (int i=1; i<=5; i++) {
        globalInfo[i].time = INFINITY;  /* The other blocks have an infinite time  */
        globalInfo[i].eventType = 0;    /* At the beginning all the blocks await an arrival */
        globalInfo[i].disabled = -1;
    }
}

/* Return the index of the block with the most imminent event */
int get_next_event() {
    int blockNumber = 0;
    double min = globalInfo[0].time;
    for (int i=0; i<=7; i++) {
        if (globalInfo[i].time < min) {
            blockNumber = i;
            min = globalInfo[i].time;
        }
    }

    if (min == INFINITY)
        return -1;
    else if (globalInfo[6].time <= min) /* Sampling */
        return 6;
    else if (globalInfo[7].time <= min) /* Change slot times */
        return 7;
    else
        return blockNumber;
}

int get_next_event_type(int blockNum) {
    return globalInfo[blockNum].eventType;
}

double get_next_event_time(int blockNum) {
    return globalInfo[blockNum].time;
}

int get_next_event_disabled(int blockNum) {
    return globalInfo[blockNum].disabled;
}

void update_next_event(int blockNum, double time, int eventType, int disabled) {
    globalInfo[blockNum].time = time;             /* Instant of the next event */
    globalInfo[blockNum].eventType = eventType;   /* Next event type: 0 arrival, 1 departure */
    globalInfo[blockNum].disabled = disabled;
}

double get_probability() {
    SelectStream(6);
    return Uniform(0.0, 1.0);
}