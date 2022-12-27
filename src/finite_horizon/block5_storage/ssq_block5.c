/* ------------------------------------------------------------------------- 
 * Name            : ssq_block5.c  (Single Server Queue)
 * Author          : Enrico D'Alessandro & Alessandro De Angelis
 * Language        : ANSI C
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include "../finite_helper.h"

#define M5 60


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

static long processedJobs = 0;           /* used to count departed jobs         */
static long number = 0;                  /* number in the node                  */

static double lastArrival = 0.0;

static int nextEvent;        /* Next event type */
static double depTime = 0;
/************************************************************************************/

double get_service_block_five(void) {
    SelectStream(5);
    return (Exponential(M5));
}

static void init_block() {
    t.current = START;              /* set the clock                         */
    t.arrival = INFINITY;           /* schedule the first arrival            */
    t.completion = INFINITY;        /* the first event can't be a completion */

    area.node = 0.0;
    area.queue = 0.0;
    area.service = 0.0;

    processedJobs = 0;                  /* used to count departed jobs         */
    number = 0;                  /* number in the node                  */

    lastArrival = 0.0;

    depTime = 0;

    init = 0;
}

static void process_arrival() {
    number++;

    t.arrival = t.current;
    lastArrival = t.arrival;

    if (number == 1)
        t.completion = t.current + get_service_block_five();
}

static void process_departure() {
    depTime = t.current;

    processedJobs++;
    number--;
    if (number > 0) {
        t.completion = t.current + get_service_block_five();
    }
    else {
        t.completion = INFINITY;
    }

    /* Return departure to the orchestrator */
    departureInfo.blockNum = 5;
    departureInfo.time = depTime;
}

static void print_statistics() {

    FILE *fp;

    printf("\nBLOCK 5 STATISTICS");
    
    printf("\nfor %ld jobs\n", processedJobs);
    printf("   average interarrival time = %6.2f\n", lastArrival / processedJobs);
    printf("   average wait ............ = %6.2f\n", area.node / processedJobs);
    printf("   average delay ........... = %6.2f\n", area.queue / processedJobs);
    printf("   average service time .... = %6.2f\n", area.service / processedJobs);
    printf("   average # in the node ... = %6.2f\n", area.node / t.current);
    printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
    printf("   utilization ............. = %6.2f\n", area.service / t.current);
    
    /* Write statistics on files */
    fp = fopen(FILENAME_WAIT_BLOCK5, "a");
    fprintf(fp,"%6.6f\n", area.node / processedJobs);
    fclose(fp);

    fp = fopen(FILENAME_DELAY_BLOCK5, "a");
    fprintf(fp,"%6.6f\n", area.queue / processedJobs);
    fclose(fp);
}

void block5() {

    /* Check if initialization of structures is needed */
    if (init == 1) {
        init_block();
    }
    
    /* Check for the end of the simulation */
    if (endSimulation == 1) 
    {
        update_next_event(5, INFINITY, -1);
        print_statistics();
        init = 1;

        return;
    }

    /* Check for server configuration changes */
    if (changeConfig == 1) {
        return;
    }

    //printf("\n-------- BLOCK 5 ACTIVATED --------\n");

    nextEvent = get_next_event_type(5);
    t.next = get_next_event_time(5);  /* next event time */

    if (number > 0 && t.next != INFINITY) {  /* update integrals  */
        area.node += (t.next - t.current) * number;
        area.queue += (t.next - t.current) * (number - 1);
        area.service += (t.next - t.current);
    }
    t.current = t.next; /* advance the clock */

    /* For global wait statistics */
    if (processedJobs == 0) /* Statistics not yet ready */
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

    update_next_event(5, t.completion, 1);

}