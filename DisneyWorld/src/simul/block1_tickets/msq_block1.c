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

#include "../simulator_helper.h"
#include "block1_helper.h"

#ifndef F
#define SERVERS_ONE 20  /* number of servers time slot 1 */
#else
#define SERVERS_ONE 10  /* number of servers time slot 2 */
#endif

#define M1 120


/***************************** GLOBAL VARIABLES *************************************/

static int init = 1;

/* the next-event structure */
typedef struct {  
    double t;   /* next event time      */
    int x;      /* event status, 0 or 1 */
} event_list_one[SERVERS_ONE];

static struct
{
    double current; /* current time                       */
    double next;    /* next (most imminent) event time    */
} t;

static struct
{                   /* accumulated sums of  */
    double service; /*   service times      */
    long served;    /*   number served      */
} sum[SERVERS_ONE];

static event_list_one event;       /* The next-event list */

static long number = 0;            /* number in the node                 */
static int e;                      /* next event index                   */
static int s;                      /* server index                       */
static long processedJobs = 0;     /* used to count processed jobs       */
static double area = 0.0;          /* time integrated number in the node */

//double tmpArea = 0.0;

static double depTime = 0.0;       /* departure time */
static int forwarded = 0;

static double service;
static double lastArrival = 0.0;
static double totalService = 0.0;
static double avgService = 0.0;
static double totalUtilization = 0.0;
/************************************************************************************/


double get_service_block_one(void) {
    SelectStream(1);
    return (Exponential(M1));
}

int next_event_block_one(event_list_one event)
/* ---------------------------------------
 * return the index of the next event type
 * ---------------------------------------
 */
{
    int e;
    int i = 0;

    while (event[i].x == 0) { /* find the index of the first 'active' */
        i++;                /* element in the event list            */
        if (i == SERVERS_ONE)
            return -1;  /* If all servers are idle return -1 */
    }
    e = i;

    while (i < (SERVERS_ONE-1))
    {        /* now, check the others to find which  */
        i++; /* event type is most imminent          */
        if ((event[i].x == 1) && (event[i].t < event[e].t))
            e = i;
    }

    return (e);
}

int find_one_block_one(event_list_one event)
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

    while (i < (SERVERS_ONE-1))
    {        /* now, check the others to find which   */
        i++; /* has been idle longest                 */
        if ((event[i].x == 0) && (event[i].t < event[s].t))
            s = i;
    }

    return (s);
}

static void process_arrival() {
    number++;
    if (number <= SERVERS_ONE) { /* se nel sistema ci sono al pi?? tanti job quanti i server allora calcola un tempo di servizio */
        lastArrival = t.current;
        service = get_service_block_one();
        s = find_one_block_one(event); 
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].x = 1;
    }
    lastArrival = t.current;
}

static void process_departure() {
    processedJobs++;
    number--; /* il job ?? stato completato */
    s = e;

    //printf("\tDeparture: %6.2f\n", event[s].t);
    depTime = event[s].t;
    if (number >= SERVERS_ONE) { /* se ci sono job in coda allora assegniamo un nuovo job con un nuovo tempo di servizio al server appena liberato */
        service = get_service_block_one();
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* aggiorna l'istante del prossimo evento su quel server (partenza) */
    }
    else { /* altrimenti quel server resta idle */
        event[s].x = 0; 
    }

    /* Preparazione delle info di ritorno per l'orchestrator */
    departureInfo.blockNum = 1;
    departureInfo.time = depTime;
}

static void print_statistics() {
    //FILE *fp;
    printf("\nBLOCK 1 STATISTICS:");
    printf("\n\nfor %ld jobs, forwarded %d\n", processedJobs, forwarded);
    printf("  avg interarrivals .. = %6.6f\n", lastArrival / processedJobs);
    printf("  avg wait ........... = %6.6f\n", area / processedJobs);
    printf("  avg # in node ...... = %6.6f\n", area / t.current);

    /* Write statistics on file */
    // fp = fopen(FILENAME_WAIT_BLOCK1, "a");
    // fprintf(fp,"%6.6f\n", area / index);
    // fclose(fp);

    for (s = 0; s < SERVERS_ONE; s++) /* adjust area to calculate */
        area -= sum[s].service;        /* averages for the queue   */

    printf("  avg delay .......... = %6.6f\n", area / processedJobs);
    printf("  avg # in queue ..... = %6.6f\n", area / t.current);
    printf("\nthe server statistics are:\n\n");
    printf("    server     utilization     avg service        share\n");
    for (s = 0; s < SERVERS_ONE; s++) {
        printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / t.current,
            sum[s].service / sum[s].served,
            (double)sum[s].served / processedJobs);
        totalService += sum[s].service / sum[s].served;
        totalUtilization += sum[s].service / t.current;
    }
    
    avgService = totalService / SERVERS_ONE;

    printf("\n   avg service ........ = %6.6f\n", avgService / SERVERS_ONE);
    printf("   avg utilization .... = %6.6f\n", totalUtilization / SERVERS_ONE);

    /* Write statistics on file */
    // fp = fopen(FILENAME_DELAY_BLOCK1, "a");
    // fprintf(fp,"%6.6f\n", area / index);
    // fclose(fp);

    printf("\n");
}

void block1()
{   
    /* Check if initialization of structures is needed */
    if (init == 1) {
        /* Initialize arrival event */
        t.current = START;

        /* Initialize server status */
        for (s = 0; s < SERVERS_ONE; s++)
        {
            event[s].t = START; /* this value is arbitrary because */
            event[s].x = 0;     /* all servers are initially idle  */
            sum[s].service = 0.0;
            sum[s].served = 0;
        }
        init = 0; /* initialization disabled */
    }

    /* Check for the end of the simulation */
    if (endSimulation == 1) {
        update_next_event(1, INFINITY, -1);
        print_statistics();
        init = 1; /* re-enable initialization */

        number = 0;   /* number in the node                 */
        processedJobs = 0;    /* used to count processed jobs       */
        area = 0.0; /* time integrated number in the node */

        //double tmpArea = 0.0;

        depTime = 0.0; /* departure time */
        forwarded = 0;

        lastArrival = 0.0;
        totalService = 0.0;
        avgService = 0.0;
        totalUtilization = 0.0;
        
        return;
    }

    //printf("\n-------- BLOCK 1 ACTIVATED --------\n");

    /* Find next event index */
    if (get_next_event_type(1) == 0) { /* Next event is an arrival */
        t.next = get_next_event_time(1);
    }
    else {  /* Next event is a completition, find the server that has finished */
        e = next_event_block_one(event);
        t.next = event[e].t;  /* next event time  */
    }
    
    area += (t.next - t.current) * number; /* update integral   */
    t.current = t.next;                    /* advance the clock */

    if (get_next_event_type(1) == 0) { /* Process an arrival */
        //printf("\nBLOCK1: Processing an arrival...\n");
        process_arrival();
    }
    else { /* Process a departure from server s */
        //printf("\nBLOCK1: Processing a departure...\n");
        process_departure();
    }

    /* L' orchestrator deve sapere quale sar?? il prossimo evento di questo blocco */
    e = next_event_block_one(event); /* next event index */
    if (e != -1) {
        update_next_event(1, event[e].t, 1); /* There is a next event for this block, update the global_info */
    }
    else {
        update_next_event(1, INFINITY, 0); /* There isn't a next event for this block, update the global_info */
    }

}