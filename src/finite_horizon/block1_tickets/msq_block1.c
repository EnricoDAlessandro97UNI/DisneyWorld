/* -------------------------------------------------------------------------
 * Name              : msq_block1.c (Multi-Server Queue)
 * Author            : Enrico D'Alessandro & Alessandro De Angelis
 * Language          : ANSI C
 * -------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../finite_helper.h"
#include "block1_helper.h"

#define SERVERS_ONE_F1 20 /* number of servers time slot 1 */
#define SERVERS_ONE_F2 10 /* number of servers time slot 2 */
#define M1 120


/***************************** GLOBAL VARIABLES *************************************/

static int init = 1;

/* the next-event structure */
typedef struct
{
    double t;   /* next event time      */
    int x;      /* event status, 0 or 1 */
    int status; /* 0 = server down, 1 = server up */
} event_list_one[MAX_SERVERS];

static struct
{
    double current; /* current time                       */
    double next;    /* next (most imminent) event time    */
} t;

static struct
{                   /* accumulated sums of  */
    double service; /*   service times      */
    long served;    /*   number served      */
} sum[MAX_SERVERS];

static event_list_one event; /* The next-event list */

static int numberOfServers = SERVERS_ONE_F1; /* current number of servers */
static int newAvailableServers = 0;
static long totalArr = 0;

static long number = 0;   /* number in the node                 */
static long queue = 0;    /* number of jobs in the queue        */
static int e;             /* next event index                   */
static int s;             /* server index                       */
static long processedJobs = 0;    /* used to count processed jobs       */
static double area = 0.0; /* time integrated number in the node */

static double depTime = 0.0; /* departure time */
static int forwarded = 0;

static double service;
static double lastArrival = 0.0;
static double totalService = 0.0;
static double avgService = 0.0;
static double totalUtilization = 0.0;
/************************************************************************************/

double get_service_block_one(void)
{
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

    while (event[i].x == 0)
    {        /* find the index of the first 'active' */
        i++; /* element in the event list            */
        if (i == MAX_SERVERS)
            return -1; /* If all servers are idle return -1 */
    }
    e = i;

    while (i < (MAX_SERVERS - 1))
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

    /* Find the index of the first idle and active server */
    while ((event[i].x == 1) && (event[i].status == 1))
        i++;
    s = i;

    /* Now, check the others to find which has been idle and active longest */
    while (i < (MAX_SERVERS - 1))
    {
        i++;
        if (((event[i].x == 0) && (event[i].status == 1)) && (event[i].t < event[s].t))
            s = i;
    }

    return (s);
}

void change_servers_status_one(event_list_one event, int servers)
{
    for (int s = 0; s < MAX_SERVERS; s++)
    {
        if (s < servers)
            event[s].status = 1;
        else
            event[s].status = 0;
    }
}

static void init_block() {
    /* Initialize arrival event */
    t.current = START;

    /* Initialize server status */
    for (s = 0; s < MAX_SERVERS; s++)
    {
        event[s].t = START; /* this value is arbitrary because */
        event[s].x = 0;     /* all servers are initially idle  */
        if (s < numberOfServers)
            event[s].status = 1; /* server up */
        else
            event[s].status = 0; /* server down */
        sum[s].service = 0.0;
        sum[s].served = 0;
    }

    numberOfServers = SERVERS_ONE_F1; /* current number of servers */
    newAvailableServers = 0;
    totalArr = 0;

    number = 0;   /* number in the node                 */
    queue = 0;    /* number of jobs in the queue        */
    processedJobs = 0;    /* used to count processed jobs       */
    area = 0.0; /* time integrated number in the node */

    depTime = 0.0; /* departure time */
    forwarded = 0;

    lastArrival = 0.0;
    totalService = 0.0;
    avgService = 0.0;
    totalUtilization = 0.0;

    init = 0; /* initialization disabled */
}

static void process_arrival() {
    number++;
    totalArr++;

    if (number <= numberOfServers)
    {
        /* se nel sistema ci sono al più tanti job quanti i server allora calcola un tempo di servizio */
        lastArrival = t.current;
        service = get_service_block_one();
        s = find_one_block_one(event);
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].x = 1;
    }
    else
    {
        queue++;
    }

    lastArrival = t.current;
}

static void process_departure() {
    processedJobs++;
    number--; /* il job è stato completato */
    s = e;

    // printf("\tDeparture: %6.2f\n", event[s].t);
    depTime = event[s].t;
    if ((number >= numberOfServers) && (event[s].status == 1))
    { /* se ci sono job in coda allora assegniamo un nuovo job
        con un nuovo tempo di servizio al
        server appena liberato */
        queue--;
        service = get_service_block_one();
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
    }
    else
    { /* altrimenti quel server resta idle */
        event[s].x = 0;
    }

    /* Prepare le info di ritorno per l'orchestrator */
    departureInfo.blockNum = 1;
    departureInfo.time = depTime;
}

static void print_statistics() {

    //FILE *fp;

    printf("\nBLOCK 1 STATISTICS");
    printf("\n\nfor %ld jobs, forwarded %d\n", processedJobs, forwarded);
    printf("  avg interarrivals .. = %6.6f\n", lastArrival / processedJobs);
    printf("  avg wait ........... = %6.6f\n", area / processedJobs);
    printf("  avg # in node ...... = %6.6f\n", area / t.current);
    
    printf("\n\n[BLOCCO1]: Processed jobs %ld, arrivals %ld\n", processedJobs, totalArr);

    /* Write statistics on file */
    /*
    fp = fopen(FILENAME_WAIT_BLOCK1, "a");
    fprintf(fp, "%6.6f\n", area / processedJobs);
    fclose(fp);
    */

    for (s = 0; s < MAX_SERVERS; s++) /* adjust area to calculate */
        area -= sum[s].service;       /* averages for the queue   */

    printf("  avg delay .......... = %6.6f\n", area / processedJobs);
    printf("  avg # in queue ..... = %6.6f\n", area / t.current);
    printf("\nthe server statistics are:\n\n");
    printf("    server     utilization     avg service        share\n");
    
    for (s = 0; s < MAX_SERVERS; s++)
    {
        /*
        printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / t.current,
               sum[s].service / sum[s].served,
               (double)sum[s].served / processedJobs);
        */
        totalService += sum[s].service / sum[s].served;
        totalUtilization += sum[s].service / t.current;
    }

    // avgService = totalService / SERVERS_ONE;

    /*
    printf("\n   avg service ........ = %6.6f\n", avgService / SERVERS_ONE);
    printf("   avg utilization .... = %6.6f\n", totalUtilization / SERVERS_ONE);
    */

    /* Write statistics on file */
    /*
    fp = fopen(FILENAME_DELAY_BLOCK1, "a");
    fprintf(fp, "%6.6f\n", area / processedJobs);
    fclose(fp);
    */

    // printf("\n");
}

void block1()
{
    /* Check if initialization of structures is needed */
    if (init == 1) {
        init_block();
    }
    
    /* Check for the end of the simulation */
    if (endSimulation == 1)
    {
        update_next_event(1, INFINITY, -1);
        print_statistics();
        init = 1;

        return;
    }

    /* Check for server configuration change */
    if (changeConfig == 1)
    {
        numberOfServers = SERVERS_ONE_F2;
        change_servers_status_one(event, numberOfServers);

        /* Assegna i job in coda immediatamente ai nuovi server (se più dei precedenti) */
        if (SERVERS_ONE_F2 > SERVERS_ONE_F1)
        {
            newAvailableServers = SERVERS_ONE_F2 - SERVERS_ONE_F1;
            while (queue > 0)
            {
                if (newAvailableServers > 0)
                {
                    /* se nel sistema ci sono al più tanti job quanti i server allora calcola un tempo di servizio */
                    service = get_service_block_one();
                    s = find_one_block_one(event);
                    sum[s].service += service;
                    sum[s].served++;
                    event[s].t = lastArrival + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
                    event[s].x = 1;
                    queue--;
                    newAvailableServers--;
                }
                else
                    break;
            }

            /* Update the most imminent event of this block */
            /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
            e = next_event_block_one(event); /* next event index */
            if (e != -1)
            {
                update_next_event(1, event[e].t, 1); /* There is a next event for this block, update the global_info */
            }
            else
            {
                update_next_event(1, INFINITY, 0);
            }
        }

        return;
    }

    //printf("\n-------- BLOCK 1 ACTIVATED --------\n");

    /* Find next event index */
    if (get_next_event_type(1) == 0) { /* Next event is an arrival */
        t.next = get_next_event_time(1);
    }
    else { /* Next event is a completition, find the server that has finished */
        e = next_event_block_one(event);
        t.next = event[e].t; /* next event time  */
    }

    area += (t.next - t.current) * number; /* update integral  */
    t.current = t.next;                    /* advance the clock*/

    /* For global wait statistics */
    if (processedJobs == 0) /* Statistics not yet ready */
        glblWaitBlockOne = 0.0;
    else
        glblWaitBlockOne = area / processedJobs;

    if (get_next_event_type(1) == 0) { /* Process an arrival */
        //printf("\nBLOCK1: Processing an arrival...\n");
        process_arrival();
    }
    else { /* Process a departure from server s */
        //printf("\nBLOCK1: Processing a departure...\n");
        process_departure();
    }

    /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
    e = next_event_block_one(event); /* next event index */
    if (e != -1) {
        update_next_event(1, event[e].t, 1); /* There is a next event for this block, update the global_info */
    }
    else {
        update_next_event(1, INFINITY, 0);
    }

}