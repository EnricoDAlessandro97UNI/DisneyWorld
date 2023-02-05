/* ------------------------------------------------------------------------- 
 * Name              : msq_block2.c (Multi-Server Queue)
 * Author            : Enrico D'Alessandro & Alessandro De Angelis
 * Language          : ANSI C 
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "block2_helper.h"
#include "../finite_helper.h"

#define SERVERS_TWO_F1 4  /* number of servers time slot 1 */
#define SERVERS_TWO_F2 3  /* number of servers time slot 2 */
#define M2 20

/***************************** GLOBAL VARIABLES *************************************/

static int init = 1;

/* the next-event structure */
typedef struct {             
    double t;   /*   next event time      */
    int x;      /*   event status, 0 or 1 */
    int status; /* 0 = server down, 1 = server up */
    int disabled;
} event_list_two[MAX_SERVERS];

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

typedef struct _person {
    int disabled;
    struct _person *next;
} person;

static person *head;
static person *tmp;

static event_list_two event; /* The next-event list */

static int numberOfServers = SERVERS_TWO_F1; /* current number of servers */
static int newAvailableServers = 0;
static long totalArr = 0;

static long number = 0;   /* number in the node                 */
static long numberDis = 0;
static long numberNoDis = 0;

static long queue = 0;     /* number of jobs in the queue        */
static int e;              /* next event index                   */
static int s;              /* server index                       */

static long processedJobs = 0;     /* used to count processed jobs       */
static long processedJobsDis = 0;
static long processedJobsNoDis = 0;

static double area = 0.0;  /* time integrated number in the node */

static double depTime = 0.0; /* departure time */

static double service;
static double lastArrival = 0.0;
static double totalService = 0.0;
static double avgService = 0.0;
static double totalUtilization = 0.0;

static double avgnode = 0.0;

static FILE *fp;

static int disabled;
/************************************************************************************/

double get_service_block_two(void) {
    SelectStream(2);
    return (Exponential(M2));
}

int next_event_block_two(event_list_two event)
/* ---------------------------------------
 * return the index of the next event type
 * ---------------------------------------
 */
{
    int e;
    int i = 0;

    while (event[i].x == 0) { /* find the index of the first 'active' */
        i++;                /* element in the event list            */
        if (i == MAX_SERVERS)
            return -1;  /* If all servers are idle return -1 */
    }
    e = i;

    while (i < (MAX_SERVERS-1))
    {        /* now, check the others to find which  */
        i++; /* event type is most imminent          */
        if ((event[i].x == 1) && (event[i].t < event[e].t))
            e = i;
    }

    return (e);
}

int find_one_block_two(event_list_two event)
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
    while (i < (MAX_SERVERS-1))
    {        
        i++; 
        if (((event[i].x == 0) && (event[i].status == 1)) && (event[i].t < event[s].t))
            s = i;
    }

    return (s);
}

void change_servers_status_two(event_list_two event, int servers) {
    for (int s=0; s<MAX_SERVERS; s++) {
        if (s<servers)
            event[s].status = 1; 
        else 
            event[s].status = 0;
    }
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

static void init_block() {
    
    head = NULL;

    numberOfServers = SERVERS_TWO_F1; /* current number of servers */
    newAvailableServers = 0;
    totalArr = 0;

    number = 0;   /* number in the node                 */
    numberDis = 0;
    numberNoDis = 0;
    queue = 0;    /* number of jobs in the queue        */
    processedJobs = 0;    /* used to count processed jobs       */
    processedJobsDis = 0;
    processedJobsNoDis = 0;
    area = 0.0; /* time integrated number in the node */

    depTime = 0.0; /* departure time */

    lastArrival = 0.0;
    totalService = 0.0;
    avgService = 0.0;
    totalUtilization = 0.0;
    
    avgnode = 0.0;

    /* Initialize arrival event */
    t.current = START;

    /* Initialize server status */
    for (s = 0; s < MAX_SERVERS; s++)
    {
        event[s].t = START; /* this value is arbitrary because */
        event[s].x = 0;     /* all servers are initially idle  */
        if (s < numberOfServers)
            event[s].status = 1;  /* server up */
        else 
            event[s].status = 0;  /* server down */
        sum[s].service = 0.0;
        sum[s].served = 0;
    }

    init = 0;
}

static void process_arrival() {

    totalArr++;

    if (number <= numberOfServers) {
        /* se nel sistema ci sono al più tanti job quanti i server allora calcola un tempo di servizio */
        lastArrival = t.current;
        service = get_service_block_two();
        s = find_one_block_two(event); 
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].x = 1;
        event[s].disabled = disabled;
        delete_person();
    }
    else
        queue++;

    lastArrival = t.current;
}

static void process_departure() {

    //printf("\nInside departure... number: %ld | numberDis: %ld | numberNoDis: %ld | head: %p\n", number, numberDis, numberNoDis, head);

    processedJobs++;

    s = e;

    depTime = event[s].t;
    if ((queue > 0) && (event[s].status == 1))
    { /* se ci sono job in coda allora assegniamo un nuovo job
        con un nuovo tempo di servizio al
        server appena liberato */
        queue--;
        service = get_service_block_two();
        sum[s].service += service;
        sum[s].served++;
        event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].disabled = head -> disabled;
        delete_person();
    }
    else
    { /* altrimenti quel server resta idle */
        event[s].x = 0; 
    }

    /* Prepare le info di ritorno per l'orchestrator */
    departureInfo.blockNum = 2;
    departureInfo.time = depTime;
    departureInfo.disabled = disabled;

    //printf("\nExit departure...\n");
}

static void print_statistics() {
    printf("\nBLOCK 2 STATISTICS");

    printf("\n\nfor %ld jobs, disabled %ld, non disabled %ld\n", processedJobs, processedJobsDis, processedJobsNoDis);
    printf("  avg interarrivals .. = %6.2f\n", lastArrival / processedJobs);
    printf("  avg wait ........... = %6.2f\n", area / processedJobs);
    printf("  avg # in node ...... = %6.2f\n", area / t.current);
    
    printf("\n\n[BLOCCO2]: Processed jobs %ld, arrivals %ld\n", processedJobs, totalArr);

    /* Write statistics on file */
    fp = fopen(FILENAME_WAIT_BLOCK2, "a");
    fprintf(fp,"%6.6f\n", area / processedJobs);
    fclose(fp);

    double tempArea = area;
    for (s = 0; s < MAX_SERVERS; s++)     /* adjust area to calculate */
        tempArea -= sum[s].service;              /* averages for the queue   */
    
    printf("  avg delay .......... = %6.2f\n", tempArea / processedJobs);
    printf("  avg # in queue ..... = %6.2f\n", tempArea / t.current);
    printf("\nthe server statistics are:\n\n");
    printf("    server     utilization     avg service        share\n");
    
    for (s = 0; s < SERVERS_TWO_F1; s++) {
        printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / t.current,
               sum[s].service / sum[s].served,
               (double) sum[s].served / processedJobs);
               
        totalService += sum[s].service / sum[s].served;
        totalUtilization += sum[s].service / t.current;
    }

    //avgService = totalService / SERVERS_THREE;
    
    /*
    printf("\n   avg service ........ = %6.6f\n", avgService / SERVERS_THREE);
    printf("   avg utilization .... = %6.6f\n", totalUtilization / SERVERS_THREE);
    */

    /* Write statistics on file */
    fp = fopen(FILENAME_DELAY_BLOCK2, "a");
    fprintf(fp,"%6.6f\n", area / processedJobs);
    fclose(fp);

    // printf("\n");
}

void block2() 
{
    /* Check if initialization of structures is needed */
    if (init == 1) {
        init_block();
    }    

    /* Sampling avgnode and avgqueue */
    if (sampling == 1) {

        /* For global wait stats */
        if (processedJobs == 0) /* stats not yet ready */ {
            fp = fopen(FILENAME_AVGNODE_BLOCK2, "a");
            fprintf(fp,"%6.6f\n", avgnode);
            fclose(fp);
        }
        else {
            fp = fopen(FILENAME_AVGNODE_BLOCK2, "a");
            fprintf(fp,"%6.6f\n", area / t.current);
            fclose(fp);
        }
            
        return;
    }

    /* Check for the end of the simulation */
    if (endSimulation == 1) {
        update_next_event(2, INFINITY, -1, -1);
        print_statistics();
        init = 1;

        return;
    }

    /* Check for server configuration change */
    if (changeConfig == 1) {

        print_statistics();

        numberOfServers = SERVERS_TWO_F2;
        change_servers_status_two(event, numberOfServers);

        if (SERVERS_TWO_F2 > SERVERS_TWO_F1) {
            newAvailableServers = SERVERS_TWO_F2 - SERVERS_TWO_F1;
        }
        while (queue > 0) {
            if (newAvailableServers > 0) {
                service = get_service_block_two();
                s = find_one_block_two(event); 
                sum[s].service += service;
                sum[s].served++;
                event[s].t = lastArrival + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
                event[s].x = 1;
                event[s].disabled = head -> disabled;
                delete_person();
                queue--;
                newAvailableServers--;
            }
            else 
                break;
        }

        /* Update the most imminent event of this block */
        /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
        e = next_event_block_two(event); /* next event index */
        if (e != -1) {
            update_next_event(2, event[e].t, 1, event[e].disabled); /* There is a next event for this block, update the global_info */
        }
        else {
            update_next_event(2, INFINITY, 0, event[e].disabled);
        }

        return; 
    }

    //printf("\n-------- BLOCK 2 ACTIVATED --------\n");

    if (get_next_event_disabled(2))
        disabled = 1;
    else
        disabled = 0;

    /* Find next event index */
    if (get_next_event_type(2) == 0) { /* Next event is an arrival */
        t.next = get_next_event_time(2);
        number++;
    }
    else {  /* Next event is a completition, find the server that has finished */
        e = next_event_block_two(event);
        t.next = event[e].t;                   /* next event time  */
        number--;
    }

    area += (t.next - t.current) * number; /* update integral  */
    t.current = t.next;                    /* advance the clock*/

    /* For global wait statistics */
    if (processedJobs == 0) /* Statistics not yet ready */
        glblWaitBlockTwo = 0.0;
    else 
        glblWaitBlockTwo = area / processedJobs;

    if (get_next_event_type(2) == 0) { /* Process an arrival */
        //printf("\nBLOCK2: Processing an arrival...\n");
        if (disabled) 
            numberDis++;
        else
            numberNoDis++;
        insert_person();
        process_arrival();            
    }
    else { /* Process a departure from server s */
        //printf("\nBLOCK2: Processing a departure...\n");
        if (disabled) {
            processedJobsDis++;
            numberDis--;
        }
        else {
            processedJobsNoDis++;
            numberNoDis--;
        }
        process_departure();
        //printf("\nDeparture ok\n");
    }

    /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
    e = next_event_block_two(event); /* next event index */
    if (e != -1) {
        update_next_event(2, event[e].t, 1, event[e].disabled); /* There is a next event for this block, update the global_info */
    }
    else {
        update_next_event(2, INFINITY, -1, event[e].disabled);
    }
}