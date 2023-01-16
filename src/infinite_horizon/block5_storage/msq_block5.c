/* ------------------------------------------------------------------------- 
 * Name              : msq.c (Multi-Server Queue)
 * Author            : Enrico D'Alessandro & Alessandro De Angelis 
 * Language          : ANSI C 
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <math.h>

#include "block5_helper.h"
#include "../infinite_helper.h"

#ifndef F
#define SERVERS_FIVE 5  /* number of servers 1 */
#else
#define SERVERS_FIVE 4  /* number of servers time slot 2 */
#endif

#define M5 60

/***************************** GLOBAL VARIABLES *************************************/

static int init = 1;

/* the next-event structure */
typedef struct {             
    double t; /*   next event time      */
    int x;    /*   event status, 0 or 1 */
} event_list_five[SERVERS_FIVE];

struct
{
    double current; /* current time                       */
    double next;    /* next (most imminent) event time    */
} t;

struct
{                   /* accumulated sums of  */
    double service; /*   service times      */
    long served;    /*   number served      */
} sum[SERVERS_FIVE];

static event_list_five event; /* The next-event list */

static long number = 0;   /* number in the node                 */
static int e;             /* next event index                   */
static int s;             /* server index                       */
static long processedJobs = 0;    /* used to count processed jobs       */
static double area = 0.0; /* time integrated number in the node */

//double tmpArea = 0.0;

static double depTime = 0.0; /* departure time */

static double service;
static double lastArrival = 0.0;
static double totalService = 0.0;
static double avgService = 0.0;
static double totalUtilization = 0.0;
/************************************************************************************/

double get_service_block_five(void) {
    SelectStream(5);
    return (Exponential(M5));
}

int next_event_block_five(event_list_five event)
/* ---------------------------------------
 * return the index of the next event type
 * ---------------------------------------
 */
{
    int e;
    int i = 0;

    while (event[i].x == 0) { /* find the index of the first 'active' */
        i++;                /* element in the event list            */
        if (i == SERVERS_FIVE)
            return -1;  /* If all servers are idle return -1 */
    }
    e = i;

    while (i < (SERVERS_FIVE-1))
    {        /* now, check the others to find which  */
        i++; /* event type is most imminent          */
        if ((event[i].x == 1) && (event[i].t < event[e].t))
            e = i;
    }

    return (e);
}

int find_one_block_five(event_list_five event)
/* -----------------------------------------------------
 * return the index of the available server idle longest
 * -----------------------------------------------------
 */
{
    int s;
    int i = 0;

    while (event[i].x == 1) /* find the index of the first available */
        i++;                /* (idle) server                         */
    s = i;

    while (i < (SERVERS_FIVE-1))
    {        /* now, check the others to find which   */
        i++; /* has been idle longest                 */
        if ((event[i].x == 0) && (event[i].t < event[s].t))
            s = i;
    }

    return (s);
}

static void process_arrival() {
    //number++;
    if (number <= SERVERS_FIVE) {
        /* se nel sistema ci sono al più tanti job quanti i server allora calcola un tempo di servizio */
        lastArrival = t.current;
        service = get_service_block_five();
        s = find_one_block_five(event); 
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].x = 1;
    }
    lastArrival = t.current;
}

static void process_departure() {
    processedJobs++;
    //number--; /* il job è stato completato */
    s = e;

    //printf("\tDeparture: %6.2f\n", event[s].t);
    depTime = event[s].t;
    if (number >= SERVERS_FIVE)
    { /* se ci sono job in coda allora assegniamo un nuovo job
        con un nuovo tempo di servizio al
        server appena liberato */
        service = get_service_block_five();
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
    }
    else
    { /* altrimenti quel server resta idle */
        event[s].x = 0; 
    }

    /* Prepare le info di ritorno per l'orchestrator */
    departureInfo.blockNum = 5;
    departureInfo.time = depTime;
}

static void print_statistics() {
    //FILE *fp;

    printf("\nBLOCK 5 STATISTICS:");
    printf("\nlast arrival: %6.2f\n", lastArrival);
    printf("\n\nfor %ld jobs\n", processedJobs);
    printf("  avg interarrivals .. = %6.2f\n", lastArrival / processedJobs);
    printf("  avg wait ........... = %6.2f\n", area / processedJobs);
    printf("  avg # in node ...... = %6.2f\n", area / t.current);

    /* Write statistics on file */
    // fp = fopen(FILENAME_WAIT_BLOCK5, "a");
    // fprintf(fp,"%6.6f\n", area / processedJobs);
    // fclose(fp);

    for (s = 0; s < SERVERS_FIVE; s++)     /* adjust area to calculate */
        area -= sum[s].service;            /* averages for the queue */   

    printf("  avg delay .......... = %6.2f\n", area / processedJobs);
    printf("  avg # in queue ..... = %6.2f\n", area / t.current);
    printf("\nthe server statistics are:\n\n");
    printf("    server     utilization     avg service        share\n");
    
    for (s = 0; s < SERVERS_FIVE; s++) {
        printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / t.current,
               sum[s].service / sum[s].served,
               (double) sum[s].served / processedJobs);
        totalService += sum[s].service / sum[s].served;
        totalUtilization += sum[s].service / t.current;
    }

    avgService = totalService / SERVERS_FIVE;

    printf("\n   avg service ........ = %6.6f\n", avgService / SERVERS_FIVE);
    printf("   avg utilization .... = %6.6f\n", totalUtilization / SERVERS_FIVE);

    /* Write statistics on file */
    // fp = fopen(FILENAME_DELAY_BLOCK5, "a");
    // fprintf(fp,"%6.6f\n", area / processedJobs);
    // fclose(fp);

    printf("\n");
}

void block5() 
{
    /* Check if initialization of structures is needed */
    if (init == 1) {
        /* Initialize arrival event */
        t.current = START;

        /* Initialize server status */
        for (s = 0; s < SERVERS_FIVE; s++)
        {
            event[s].t = START; /* this value is arbitrary because */
            event[s].x = 0;     /* all servers are initially idle  */
            sum[s].service = 0.0;
            sum[s].served = 0;
        }
        init = 0;
    }

    /* Check for the end of the simulation */
    if (endSimulation == 1) {
        update_next_event(5, INFINITY, -1);
        print_statistics();
        init = 1;

        number = 0;   /* number in the node                 */
        processedJobs = 0;    /* used to count processed jobs       */
        area = 0.0; /* time integrated number in the node */

        //double tmpArea = 0.0;

        depTime = 0.0; /* departure time */

        lastArrival = 0.0;
        totalService = 0.0;
        avgService = 0.0;
        totalUtilization = 0.0;

        return;
    }

    //printf("\n-------- BLOCK 5 ACTIVATED --------\n");

    /* Find next event index */
    if (get_next_event_type(5) == 0) { /* Next event is an arrival */
        t.next = get_next_event_time(5);
        number++;
    }
    else {  /* Next event is a completition, find the server that has finished */
        e = next_event_block_five(event);
        t.next = event[e].t;                   /* next event time  */
        number--;
    }

    area += (t.next - t.current) * number; /* update integral  */
    t.current = t.next;                    /* advance the clock*/

    /* For global wait stats */
    if (processedJobs == 0) /* stats not yet ready */
        glblWaitBlockFive = 0.0;
    else
        glblWaitBlockFive = area / processedJobs;

    if (get_next_event_type(5) == 0) { /* Process an arrival */
        //printf("\nBLOCK5: Processing an arrival...\n");
        process_arrival();
    }
    else { /* Process a departure from server s */
        //printf("\nBLOCK5: Processing a departure...\n");
        process_departure();
    }

    /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
    e = next_event_block_five(event); /* next event index */
    if (e != -1) {
        update_next_event(5, event[e].t, 1); /* There is a next event for this block, update the global_info */
    }
    else {
        update_next_event(5, INFINITY, -1);
    }

}
