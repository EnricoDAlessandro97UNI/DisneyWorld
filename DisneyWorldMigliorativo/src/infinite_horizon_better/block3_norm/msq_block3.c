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

#include "block3_helper.h"
#include "../infinite_helper.h"

#ifndef F
#define SERVERS_THREE 4     /* number of servers time slot 1 */
#else
#define SERVERS_THREE 3     /* number of servers time slot 2 */
#endif

#define M3 10

/***************************** GLOBAL VARIABLES *************************************/

static int init = 1;

/* the next-event structure */
typedef struct {             
    double t; /*   next event time      */
    int x;    /*   event status, 0 or 1 */
    int disabled;
} event_list_three[SERVERS_THREE];

static struct
{
    double current; /* current time                       */
    double next;    /* next (most imminent) event time    */
} t;

static struct
{                   /* accumulated sums of  */
    double service; /*   service times      */
    long served;    /*   number served      */
} sum[SERVERS_THREE];

typedef struct _person {
    int disabled;
    struct _person *next;
} person;

static person *head;
static person *tmp;

static event_list_three event; /* The next-event list */

static long number = 0;   /* number in the node                 */
static long numberDis = 0;
static long numberNoDis = 0;

static int e;             /* next event index                   */
static int s;             /* server index                       */

static long processedJobs = 0;    /* used to count processed jobs       */
static long processedJobsDis = 0;
static long processedJobsNoDis = 0;

static double area = 0.0; /* time integrated number in the node */

//double tmpArea = 0.0;

static double depTime = 0.0; /* departure time */

static double service;
static double lastArrival = 0.0;
static double totalService = 0.0;
static double avgService = 0.0;
static double totalUtilization = 0.0;

static double avgnode = 0.0;
static double avgqueue = 0.0;

static FILE *fp;

static int disabled;
/************************************************************************************/

double get_service_block_three(void) {
    SelectStream(3);
    return (Exponential(M3));
}

int next_event_block_three(event_list_three event)
/* ---------------------------------------
 * return the index of the next event type
 * ---------------------------------------
 */
{
    int e;
    int i = 0;

    while (event[i].x == 0) { /* find the index of the first 'active' */
        i++;                /* element in the event list            */
        if (i == SERVERS_THREE)
            return -1;  /* If all servers are idle return -1 */
    }
    e = i;

    while (i < (SERVERS_THREE-1))
    {        /* now, check the others to find which  */
        i++; /* event type is most imminent          */
        if ((event[i].x == 1) && (event[i].t < event[e].t))
            e = i;
    }

    return (e);
}

int find_one_block_three(event_list_three event)
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

    while (i < (SERVERS_THREE-1))
    {        /* now, check the others to find which   */
        i++; /* has been idle longest                 */
        if ((event[i].x == 0) && (event[i].t < event[s].t))
            s = i;
    }

    return (s);
}

static void insert_person() {
    person *node;
    node = (person*)malloc(sizeof(person));
    node -> disabled = disabled;
    node -> next = NULL;

    if (head == NULL)
        head = node;
    else {
        tmp = head;
        while (tmp -> next != NULL) {
            tmp = tmp -> next;
        }
        tmp -> next = node;
    }
}

static void delete_person() {
    person *tmp = head;
    head = head -> next;
    free(tmp);
}

static void process_arrival() {
    if (number <= SERVERS_THREE) {
        /* se nel sistema ci sono al più tanti job quanti i server allora calcola un tempo di servizio */
        service = get_service_block_three();
        s = find_one_block_three(event); 
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].x = 1;
        event[s].disabled = disabled;
    }
}

static void process_departure() {
    processedJobs++;
    s = e;

    //printf("\tDeparture: %6.2f\n", event[s].t);
    depTime = event[s].t;
    if (number >= SERVERS_THREE)
    { /* se ci sono job in coda allora assegniamo un nuovo job
        con un nuovo tempo di servizio al
        server appena liberato */
        service = get_service_block_three();
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].disabled = head -> disabled;
    }
    else
    { /* altrimenti quel server resta idle */
        event[s].x = 0; 
    }

    /* Prepare le info di ritorno per l'orchestrator */
    departureInfo.blockNum = 3;
    departureInfo.time = depTime;
    departureInfo.disabled = disabled;
}

static void print_statistics() {
    printf("\nBLOCK 3 STATISTICS:");

    printf("\n\nfor %ld jobs, disabled %ld, non disabled %ld\n", processedJobs, processedJobsDis, processedJobsNoDis);
    printf("  avg interarrivals .. = %6.2f\n", lastArrival / processedJobs);
    printf("  avg wait ........... = %6.2f\n", area / processedJobs);
    printf("  avg # in node ...... = %6.2f\n", area / t.current);

    /* Write statistics on file */
    fp = fopen(FILENAME_WAIT_BLOCK3, "a");
    fprintf(fp,"%6.6f\n", area / processedJobs);
    fclose(fp);

    for (s = 0; s < SERVERS_THREE; s++)     /* adjust area to calculate */
        area -= sum[s].service;              /* averages for the queue   */

    printf("  avg delay .......... = %6.2f\n", area / processedJobs);
    printf("  avg # in queue ..... = %6.2f\n", area / t.current);

    printf("\nthe server statistics are:\n\n");
    printf("    server     utilization     avg service        share\n");
    for (s = 0; s < SERVERS_THREE; s++) {
        printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / t.current,
               sum[s].service / sum[s].served,
               (double) sum[s].served / processedJobs);
        totalService += sum[s].service / sum[s].served;
        totalUtilization += sum[s].service / t.current;
    }

    avgService = totalService / SERVERS_THREE;

    printf("\n   avg service ........ = %6.6f\n", avgService / SERVERS_THREE);
    printf("   avg utilization .... = %6.6f\n", totalUtilization / SERVERS_THREE);

    /* Write statistics on file */
    fp = fopen(FILENAME_DELAY_BLOCK3, "a");
    fprintf(fp,"%6.6f\n", area / processedJobs);
    fclose(fp);

    printf("\n");
}

void block3() 
{
    /* Check if initialization of structures is needed */
    if (init == 1) {
        /* Initialize arrival event */
        t.current = START;

        head = NULL;

        avgnode = 0.0;
        avgqueue = 0.0;

        /* Initialize server status */
        for (s = 0; s < SERVERS_THREE; s++)
        {
            event[s].t = START; /* this value is arbitrary because */
            event[s].x = 0;     /* all servers are initially idle  */
            event[s].disabled = -1;
            sum[s].service = 0.0;
            sum[s].served = 0;
        }
        init = 0;
    }

    /* Sampling avgnode and avgqueue */
    if (sampling == 1) {

        /* For global wait stats */
        if (processedJobs == 0) /* stats not yet ready */ {
            fp = fopen(FILENAME_AVGNODE_BLOCK3, "a");
            fprintf(fp,"%6.6f\n", avgnode);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK3, "a");
            fprintf(fp,"%6.6f\n", avgqueue);
            fclose(fp);
        }
        else {
            fp = fopen(FILENAME_AVGNODE_BLOCK3, "a");
            fprintf(fp,"%6.6f\n", area / t.current);
            fclose(fp);
            int tmpArea = area;
            for (s = 0; s < SERVERS_THREE; s++) /* adjust area to calculate */
                tmpArea -= sum[s].service;        /* averages for the queue   */
            fp = fopen(FILENAME_AVGQUEUE_BLOCK3, "a");
            fprintf(fp,"%6.6f\n", tmpArea / t.current);
            fclose(fp);
        }
            
        return;
    }

    /* Check for the end of the simulation */
    if (endSimulation == 1) {
        update_next_event(3, INFINITY, -1, -1);
        print_statistics();
        init = 1; /* re-enable initialization */
        
        number = 0;   /* number in the node                 */
        numberDis = 0;
        numberNoDis = 0;

        processedJobs = 0;    /* used to count processed jobs       */
        processedJobsDis = 0;
        processedJobsNoDis = 0;

        area = 0.0; /* time integrated number in the node */

        //double tmpArea = 0.0;

        depTime = 0.0; /* departure time */

        lastArrival = 0.0;
        totalService = 0.0;
        avgService = 0.0;
        totalUtilization = 0.0;
        
        return;
    }

    //printf("\n-------- BLOCK 3 ACTIVATED --------\n");

    /* Find next event index */
    if (get_next_event_type(3) == 0) { /* Next event is an arrival */
        t.next = get_next_event_time(3);
        number++;
        lastArrival = t.next;
    }
    else {  /* Next event is a completition, find the server that has finished */
        e = next_event_block_three(event);
        t.next = event[e].t;                   /* next event time  */
        number--;
    }

    area += (t.next - t.current) * number; /* update integral  */
    t.current = t.next;                    /* advance the clock*/

    /* For global wait stats */
    if (processedJobs == 0) /* stats not yet ready */
        glblWaitBlockThree = 0.0;
    else
        glblWaitBlockThree = area / processedJobs;

    if (get_next_event_disabled(3))
        disabled = 1;
    else 
        disabled = 0;

    if (get_next_event_type(3) == 0) { /* Process an arrival */
        //printf("\nBLOCK3: Processing an arrival...\n");
        if (disabled)
            numberDis++;
        else 
            numberNoDis++;
        insert_person();
        process_arrival();
    }
    else { /* Process a departure from server s */
        //printf("\nBLOCK3: Processing a departure...\n");
        if (disabled) 
            processedJobsDis++;
        else   
            processedJobsNoDis++;
        delete_person();
        process_departure();
    }

    /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
    e = next_event_block_three(event); /* next event index */
    if (e != -1) {
        update_next_event(3, event[e].t, 1, event[e].disabled); /* There is a next event for this block, update the global_info */
    }
    else {
        update_next_event(3, INFINITY, -1, event[e].disabled);
    }
    
}