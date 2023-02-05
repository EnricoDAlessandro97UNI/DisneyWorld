/* ------------------------------------------------------------------------- 
 * Name              : msq_block4_prio.c (Multi-Server Queue - priority)
 * Author            : Enrico D'Alessandro & Alessandro De Angelis
 * Language          : ANSI C  
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <math.h>

#include "block4_helper.h"
#include "../infinite_helper.h"

#ifndef F
#define SERVERS_FOUR 33     /* number of servers time slot 1 */
#else
#define SERVERS_FOUR 19     /* number of servers time slot 2 */
#endif

#define M4 75


/***************************** GLOBAL VARIABLES *************************************/

static int init = 1;

/* the next-event structure */
typedef struct {  
    double t;   /* next event time      */
    int x;      /* event status, 0 or 1 */
    int disabled;
} event_list_four[SERVERS_FOUR];

typedef struct
{
    double current; /* current time                       */
    double next;    /* next (most imminent) event time    */
} info;

static struct
{                   /* accumulated sums of  */
    double service; /*   service times      */
    long served;    /*   number served      */
} sum[SERVERS_FOUR];

typedef struct _person {
    int disabled;
    struct _person *next;
} person;

static person *headDis;
static person *headNoDis;
static person *tmp;

static event_list_four event; /* The next-event list */

static info nodis;
static info dis;

static long number = 0;   /* number in the node                 */
static long numberDis = 0;
static long numberNoDis = 0;

static int e;             /* next event index                   */
static int s;             /* server index                       */

static long processedJobs = 0;    /* used to count processed jobs       */
static long processedJobsDis = 0;
static long processedJobsNoDis = 0;

static double area = 0.0; /* time integrated number in the node */
static double areaDis = 0.0;
static double areaNoDis = 0.0;

static double depTime = 0.0; /* departure time */

static double service;
static double current;
static double next;
static double lastArrival = 0.0;
static double lastArrivalDis = 0.0;
static double lastArrivalNoDis = 0.0;
static double totalService = 0.0;
static double avgService = 0.0;
static double totalUtilization = 0.0;

static double avgnode = 0.0;
static double avgqueue = 0.0;
static double avgnodeclass1 = 0.0;
static double avgqueueclass1 = 0.0;
static double avgnodeclass2 = 0.0;
static double avgqueueclass2 = 0.0;

static FILE *fp;

static int disabled;
/************************************************************************************/

double get_service_block_four(void) {
    SelectStream(4);
    return (Exponential(M4));
}

int next_event_block_four(event_list_four event)
/* ---------------------------------------
 * return the index of the next event type
 * ---------------------------------------
 */
{
    int e;
    int i = 0;

    while (event[i].x == 0) { /* find the index of the first 'active' */
        i++;                /* element in the event list            */
        if (i == SERVERS_FOUR)
            return -1;  /* If all servers are idle return -1 */
    }
    e = i;

    while (i < (SERVERS_FOUR-1))
    {        /* now, check the others to find which  */
        i++; /* event type is most imminent          */
        if ((event[i].x == 1) && (event[i].t < event[e].t))
            e = i;
    }

    return (e);
}

int find_one_block_four(event_list_four event)
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

    while (i < (SERVERS_FOUR-1))
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

    if (headNoDis == NULL)
        headNoDis = node;
    else {
        tmp = headNoDis;
        while (tmp -> next != NULL) {
            tmp = tmp -> next;
        }
        tmp -> next = node;
    }
}

static void insert_person_dis() {
    person *node;
    node = (person*)malloc(sizeof(person));
    node -> disabled = disabled;
    node -> next = NULL;

    if (headDis == NULL)
        headDis = node;
    else {
        tmp = headDis;
        while (tmp -> next != NULL) {
            tmp = tmp -> next;
        }
        tmp -> next = node;
    }
}

static void delete_person() {
    person *tmp = headNoDis;
    headNoDis = headNoDis -> next;
    free(tmp);
}

static void delete_person_dis() {
    person *tmp = headDis;
    headDis = headDis -> next;
    free(tmp);
}

static void process_arrival() {
    if (number <= SERVERS_FOUR) {
        /* se nel sistema ci sono al più tanti job quanti i server allora calcola un tempo di servizio */
        service = get_service_block_four();
        s = find_one_block_four(event); 
        sum[s].service += service;
        sum[s].served++;
        event[s].t = current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        event[s].x = 1;
        event[s].disabled = disabled;
        
        /* il job è stato assegnato ad un server, lo elimino dalla coda degli arrivi */
        if (disabled)
            delete_person_dis();
        else
            delete_person();
    }
}

static void process_departure() {
    
    processedJobs++;

    s = e;

    depTime = event[s].t;
    if (number >= SERVERS_FOUR)
    { 
        service = get_service_block_four();
        sum[s].service += service;
        sum[s].served++;
        event[s].t = current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
        if (headDis != NULL) { /* assegno il server ad un dis */
            event[s].disabled = 1;
            delete_person_dis();
        }
        else if (headNoDis != NULL) { /* assegno il server ad un non dis */
            event[s].disabled = 0;
            delete_person();
        }
        else {
            printf("\n[BLOCK4]: Error departure\n");
            exit(-1);
        }
    }
    else
    { /* altrimenti quel server resta idle */
        event[s].x = 0; 
    }

    /* Prepare le info di ritorno per l'orchestrator */
    departureInfo.blockNum = 4;
    departureInfo.time = depTime;
    departureInfo.disabled = disabled;
}

static void print_statistics() {
    printf("\nBLOCK 4 STATISTICS:");

    printf("\n\nfor %ld jobs, lost %d, pushed to exit %d\n", processedJobs, block4Lost, block4ToExit);
    printf("for %ld jobs, disabled %ld, non disabled %ld\n", processedJobs, processedJobsDis, processedJobsNoDis);
    printf("  avg interarrivals .. = %6.2f\n", lastArrival / processedJobs);
    printf("  avg wait ........... = %6.2f\n", area / processedJobs);
    printf("  avg wait class 1 ........... = %6.2f\n", areaDis / processedJobsDis);
    printf("  avg wait class 2 ........... = %6.2f\n", areaNoDis / processedJobsNoDis);
    printf("  avg # in node ...... = %6.2f\n", area / current);
    printf("  avg # in node class 1 ...... = %6.2f\n", areaDis / dis.current);
    printf("  avg # in node class 2 ...... = %6.2f\n", areaNoDis / nodis.current);

    /* Write statistics on file */
    fp = fopen(FILENAME_WAIT_BLOCK4, "a");
    fprintf(fp,"%6.6f\n", area / processedJobs);
    fclose(fp);

    // fp = fopen(FILENAME_WAIT_BLOCK4_CLASS1, "a");
    // fprintf(fp,"%6.6f\n", areaDis / processedJobsDis);
    // fclose(fp);

    // fp = fopen(FILENAME_WAIT_BLOCK4_CLASS2, "a");
    // fprintf(fp,"%6.6f\n", areaNoDis / processedJobsNoDis);
    // fclose(fp);

    for (s = 0; s < SERVERS_FOUR; s++)     /* adjust area to calculate */
        area -= sum[s].service;              /* averages for the queue   */

    printf("\nthe server statistics are:\n\n");
    printf("    server     utilization     avg service        share\n");
    for (s = 0; s < SERVERS_FOUR; s++) {
        printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / current,
               sum[s].service / sum[s].served,
               (double) sum[s].served / processedJobs);
        totalService += sum[s].service / sum[s].served;
        totalUtilization += sum[s].service / current;
    }

    avgService = totalService / SERVERS_FOUR;

    printf("\n   avg service ........ = %6.6f\n", avgService / SERVERS_FOUR);
    printf("   avg delay .......... = %6.2f\n", area / processedJobs);
    printf("   avg delay class 1 .......... = %6.2f\n", (areaDis / processedJobsDis) - (avgService));
    printf("   avg delay class 2 .......... = %6.2f\n", (areaNoDis / processedJobsNoDis) - (avgService));
    printf("   avg # in queue ..... = %6.2f\n", area / current);
    printf("   avg # in queue class 1 ..... = %6.2f\n", ((areaDis / processedJobsDis) - (avgService)) * (processedJobsDis/lastArrival));
    printf("   avg # in queue class 2 ..... = %6.2f\n", ((areaNoDis / processedJobsNoDis) - (avgService)) * (processedJobsNoDis/lastArrival));
    printf("   avg utilization .... = %6.6f\n", totalUtilization / SERVERS_FOUR);

    /* Write statistics on file */
    fp = fopen(FILENAME_DELAY_BLOCK4, "a");
    fprintf(fp,"%6.6f\n", area / processedJobs);
    fclose(fp);

    // fp = fopen(FILENAME_DELAY_BLOCK4_CLASS1, "a");
    // fprintf(fp,"%6.6f\n", (areaDis / processedJobsDis) - (avgService));
    // fclose(fp);

    // fp = fopen(FILENAME_DELAY_BLOCK4_CLASS2, "a");
    // fprintf(fp,"%6.6f\n", (areaNoDis / processedJobsNoDis) - (avgService));
    // fclose(fp);

    printf("\n");
}

void block4() 
{
    /* Check if initialization of structures is needed */
    if (init == 1) {
        /* Initialize arrival event */
        nodis.current = START;
        dis.current = START;
        current = START;

        headNoDis = NULL;
        headDis = NULL;

        avgnode = 0.0;
        avgqueue = 0.0;
        avgnodeclass1 = 0.0;
        avgqueueclass1 = 0.0;
        avgnodeclass2 = 0.0;
        avgqueueclass2 = 0.0;

        number = 0;   /* number in the node                 */
        numberDis = 0;
        numberNoDis = 0;
        processedJobs = 0;    /* used to count processed jobs       */
        processedJobsDis = 0;
        processedJobsNoDis = 0;

        area = 0.0; /* time integrated number in the node */
        areaDis = 0.0;
        areaNoDis = 0.0;

        depTime = 0.0; /* departure time */

        current = 0.0;
        lastArrival = 0.0;
        lastArrivalDis = 0.0;
        lastArrivalNoDis = 0.0;
        totalService = 0.0;
        avgService = 0.0;
        totalUtilization = 0.0;

        /* Initialize server status */
        for (s = 0; s < SERVERS_FOUR; s++)
        {
            event[s].t = START; /* this value is arbitrary because */
            event[s].x = 0;     /* all servers are initially idle  */
            sum[s].service = 0.0;
            sum[s].served = 0;
        }
        init = 0;
    }

    /* Sampling avgnode and avgqueue */
    if (sampling == 1) {

        /* For global wait stats */
        if (processedJobs == 0) /* stats not yet ready */ {
            fp = fopen(FILENAME_AVGNODE_BLOCK4, "a");
            fprintf(fp,"%6.6f\n", avgnode);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK4, "a");
            fprintf(fp,"%6.6f\n", avgqueue);
            fclose(fp);
        }
        else {
            fp = fopen(FILENAME_AVGNODE_BLOCK4, "a");
            fprintf(fp,"%6.6f\n", area / current);
            fclose(fp);
            int tmpArea = area;
            for (s = 0; s < SERVERS_FOUR; s++) /* adjust area to calculate */
                tmpArea -= sum[s].service;        /* averages for the queue   */
            fp = fopen(FILENAME_AVGQUEUE_BLOCK4, "a");
            fprintf(fp,"%6.6f\n", tmpArea / current);
            fclose(fp);

            for (s = 0; s < SERVERS_FOUR; s++) {
                totalService += sum[s].service / sum[s].served;
                totalUtilization += sum[s].service / current;
            }

            avgService = totalService / SERVERS_FOUR;

            totalService = 0.0;
            totalUtilization = 0.0;
        }

        if (processedJobsDis == 0) {
            fp = fopen(FILENAME_AVGNODE_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", avgnodeclass1);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", avgqueueclass1);
            fclose(fp);
            fp = fopen(FILENAME_DELAY_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", 0.0);
            fclose(fp);
            fp = fopen(FILENAME_WAIT_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", 0.0);
            fclose(fp);
        }
        else {
            fp = fopen(FILENAME_AVGNODE_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", areaDis / dis.current);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", ((areaDis / processedJobsDis) - (avgService)) * (processedJobsDis/lastArrival));
            fclose(fp);
            fp = fopen(FILENAME_DELAY_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", (areaDis / processedJobsDis) - (avgService));
            fclose(fp);
            fp = fopen(FILENAME_WAIT_BLOCK4_CLASS1, "a");
            fprintf(fp,"%6.6f\n", areaDis / processedJobsDis);
            fclose(fp);
        }

        if (processedJobsNoDis == 0) {
            fp = fopen(FILENAME_AVGNODE_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", avgnodeclass2);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", avgqueueclass2);
            fclose(fp);
            fp = fopen(FILENAME_DELAY_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", 0.0);
            fclose(fp);
            fp = fopen(FILENAME_WAIT_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", 0.0);
            fclose(fp);
        }
        else {
            fp = fopen(FILENAME_AVGNODE_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", areaNoDis / nodis.current);
            fclose(fp);
            fp = fopen(FILENAME_AVGQUEUE_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", ((areaNoDis / processedJobsNoDis) - (avgService)) * (processedJobsNoDis/lastArrival));
            fclose(fp);
            fp = fopen(FILENAME_DELAY_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", (areaNoDis / processedJobsNoDis) - (avgService));
            fclose(fp);
            fp = fopen(FILENAME_WAIT_BLOCK4_CLASS2, "a");
            fprintf(fp,"%6.6f\n", areaNoDis / processedJobsNoDis);
            fclose(fp);
        }

        return;
    }

    /* Check for the end of the simulation */
    if (endSimulation == 1) {
        update_next_event(4, INFINITY, -1, -1);
        print_statistics();
        init = 1;

        return;
    }

    //printf("\n-------- BLOCK 4 ACTIVATED --------\n");

    if (get_next_event_disabled(4))
        disabled = 1;
    else 
        disabled = 0;

    /* Find next event index */
    if (get_next_event_type(4) == 0) { /* Next event is an arrival */
        if (disabled) {
            numberDis++;
            dis.next = get_next_event_time(4);
            next = dis.next;
            lastArrivalDis = dis.next;
        }
        else {
            numberNoDis++;
            nodis.next = get_next_event_time(4);
            next = nodis.next;
            lastArrivalNoDis = nodis.next;
        }
        lastArrival = next;
        number++;
    }
    else {  /* Next event is a completition, find the server that has finished */
        e = next_event_block_four(event);
        if (disabled) {
            numberDis--;
            dis.next = event[e].t; /* next event time  */
            next = dis.next;
        }
        else {
            numberNoDis--;
            nodis.next = event[e].t;
            next = nodis.next;
        }
        number--;
    }

    /* Total area */
    area += (next - current) * number; /* update integral  */
    current = next;                    /* advance the clock*/

    if (disabled) {
        areaDis += (dis.next - dis.current) * numberDis;
        dis.current = dis.next;
    }
    else {
        areaNoDis += (nodis.next - nodis.current) * numberNoDis;
        nodis.current = nodis.next;
    }

    /* For global wait stats */
    if (processedJobs == 0) /* stats not yet ready */
        glblWaitBlockFour = 0.0;
    else
        glblWaitBlockFour = area / processedJobs;

    if (get_next_event_type(4) == 0) { /* Process an arrival */
        //printf("\nBLOCK4: Processing an arrival...\n");
        if (disabled)
            insert_person_dis();
        else
            insert_person();
        process_arrival();
    }
    else { /* Process a departure from server s */
        //printf("\nBLOCK4: Processing a departure...\n");
        if (disabled)
            processedJobsDis++;      
        else 
            processedJobsNoDis++;

        process_departure();
    }

    /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
    e = next_event_block_four(event); /* next event index */
    if (e != -1) {
        update_next_event(4, event[e].t, 1, event[e].disabled); /* There is a next event for this block, update the global_info */
    }
    else {
        update_next_event(4, INFINITY, -1, event[e].disabled);
    }
}