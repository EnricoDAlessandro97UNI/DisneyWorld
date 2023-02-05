/* ------------------------------------------------------------------------- 
 * This program is a next-event simulation of a single-server FIFO service
 * node using Exponentially distributed interarrival times and Erlang 
 * distributed service times (i.e., a M/E/1 queue).  The service node is 
 * assumed to be initially idle, no arrivals are permitted after the 
 * terminal time STOP, and the node is then purged by processing any 
 * remaining jobs in the service node.
 *
 * Name            : ssq4.c  (Single Server Queue, version 4)
 * Author          : Steve Park & Dave Geyer
 * Language        : ANSI C
 * Latest Revision : 11-09-98
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include <sys/sem.h>
#include "../infinite_helper.h"

#define M2 20


/***************************** GLOBAL VARIABLES *************************************/

static int init = 1;

static struct {
    double arrival;                 /* next arrival time                   */
    double completion;              /* next completion time                */
    double current;                 /* current time                        */
    double next;                    /* next (most imminent) event time     */
    double last;                    /* last arrival time                   */
} t;

static struct {
    double node;                    /* time integrated number in the node  */
    double queue;                   /* time integrated number in the queue */
    double service;                 /* time integrated number in service   */
} area = {0.0, 0.0, 0.0};

static long processedJobs = 0;                  /* used to count departed jobs         */
static long number = 0;                  /* number in the node                  */

static double lastArrival = 0.0;

static int nextEvent;        /* Next event type */
static double depTime = 0;

static double avgnode = 0.0;
static double avgqueue = 0.0;

static FILE *fp;
/************************************************************************************/


double get_service_block_two() {
    SelectStream(2);
    return (Exponential(M2));
}

static void process_arrival() {
    //number++;
    t.arrival = t.current;
    lastArrival = t.arrival;
    if (number == 1)
        t.completion = t.current + get_service_block_two();
}

static void process_departure() {
    depTime = t.current;
    processedJobs++;
    //number--;
    if (number > 0) {
        t.completion = t.current + get_service_block_two();
    }
    else {
        t.completion = INFINITY;
    }
    /* Return departure to the orchestrator */
    departureInfo.blockNum = 2;
    departureInfo.time = depTime;
}

static void print_statistics() {
    printf("\nBLOCK 2 STATISTICS:");

    printf("\nfor %ld jobs\n", processedJobs);
    printf("   average interarrival time = %6.2f\n", lastArrival / processedJobs);
    printf("   average wait ............ = %6.2f\n", area.node / processedJobs);
    printf("   average delay ........... = %6.2f\n", area.queue / processedJobs);
    printf("   average service time .... = %6.2f\n", area.service / processedJobs);
    printf("   average # in the node ... = %6.2f\n", area.node / t.current);
    printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
    printf("   utilization ............. = %6.2f\n", area.service / t.current);

    /* Write statistics on files */
    fp = fopen(FILENAME_WAIT_BLOCK2, "a");
    fprintf(fp,"%6.6f\n", area.node / processedJobs);
    fclose(fp);

    fp = fopen(FILENAME_DELAY_BLOCK2, "a");
    fprintf(fp,"%6.6f\n", area.queue / processedJobs);
    fclose(fp);
}

void block2() {

    /* Check if initialization of structures is needed */
    if (init == 1) {
        
        t.current = START;              /* set the clock                         */
        t.arrival = INFINITY;           /* schedule the first arrival            */
        t.completion = INFINITY;        /* the first event can't be a completion */

        avgnode = 0.0;
        avgqueue = 0.0;

        init = 0;
    }

    /* Sampling avgnode and avgqueue */
    if (sampling == 1) {

        /* For global wait stats */
        if (processedJobs == 0) /* stats not yet ready */ {
            fp = fopen(FILENAME_AVGNODE_BLOCK2, "a");
            fprintf(fp,"%6.6f\n", avgnode);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK2, "a");
            fprintf(fp,"%6.6f\n", avgqueue);
            fclose(fp);
        }
        else {
            fp = fopen(FILENAME_AVGNODE_BLOCK2, "a");
            fprintf(fp,"%6.6f\n", area.node / t.current);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK2, "a");
            fprintf(fp,"%6.6f\n", area.queue / t.current);
            fclose(fp);
        }
            
        return;
    }

    /* Check for the end of the simulation */
    if (endSimulation == 1) {
        update_next_event(2, INFINITY, -1);
        print_statistics();
        init = 1; /* re-enable initialization */
        
        area.node = 0.0;
        area.queue = 0.0;
        area.service = 0.0;

        processedJobs = 0;           /* used to count departed jobs         */
        number = 0;                  /* number in the node                  */

        lastArrival = 0.0;

        depTime = 0;

        return;
    }

    //printf("\n-------- BLOCK 2 ACTIVATED --------\n");

    nextEvent = get_next_event_type(2);
    t.next = get_next_event_time(2);  /* next event time */

    if (nextEvent == 0) {
        number++;
    }
    else {
        number--;
    }

    if (number > 0 && t.next != INFINITY) {  /* update integrals  */
        area.node += (t.next - t.current) * number;
        area.queue += (t.next - t.current) * (number - 1);
        area.service += (t.next - t.current);
    }
    t.current = t.next; /* advance the clock */

    /* For global wait stats */
    if (processedJobs == 0) /* stats not yet ready */
        glblWaitBlockTwo = 0.0;
    else 
        glblWaitBlockTwo = area.node / processedJobs;

    if (nextEvent == 0) { /* Process an arrival */
        //printf("\nBLOCK2: Processing an arrival...\n");
        process_arrival();
    } 
    else { /* Process a completion */
        //printf("\nBLOCK2: Processing a departure...\n");
        process_departure();
    }

    update_next_event(2, t.completion, 1);
}
