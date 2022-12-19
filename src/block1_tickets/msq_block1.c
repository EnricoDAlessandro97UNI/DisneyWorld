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
#include <pthread.h>

#include "../orchestrator_helper.h"
#include "block1_helper.h"

#define SERVERS_ONE 20  /* number of servers */
#define M1 120

/* the next-event structure */
typedef struct {  
    double t;   /* next event time      */
    int x;      /* event status, 0 or 1 */
} event_list_one[SERVERS_ONE];

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

void *block1()
{   
    FILE *fp;

    struct
    {
        double current; /* current time                       */
        double next;    /* next (most imminent) event time    */
    } t;

    struct
    {                   /* accumulated sums of  */
        double service; /*   service times      */
        long served;    /*   number served      */
    } sum[SERVERS_ONE];

    event_list_one event; /* The next-event list */

    long number = 0;   /* number in the node                 */
    int e;             /* next event index                   */
    int s;             /* server index                       */
    long index = 0;    /* used to count processed jobs       */
    double area = 0.0; /* time integrated number in the node */

    //double tmpArea = 0.0;

    double depTime = 0.0; /* departure time */
    int forwarded = 0;

    double service;
    double lastArrival = 0.0;
    double totalService = 0.0;
    double avgService = 0.0;
    double totalUtilization = 0.0;

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

    struct sembuf oper;
    /* Unlock the orchestrator */
    oper.sem_num = 0;
    oper.sem_op = 1;
    oper.sem_flg = 0;
    semop(mainSem, &oper, 1);

    while (1) 
    {
        /* Wait for the start from the orchestrator */
        oper.sem_num = 0;
        oper.sem_op = -1;
        oper.sem_flg = 0;
        semop(sem, &oper, 1);

        /* Check for the end of the simulation */
        if (endSimulation == 1) {
            update_next_event(1, INFINITY, -1);
            oper.sem_num = 0;
            oper.sem_op = 1;
            oper.sem_flg = 0;
            semop(mainSem, &oper, 1);
            break;
        }

        //printf("\n-------- BLOCK 1 --------\n");

        /* Find next event index */
        if (get_next_event_type(1) == 0) { /* Next event is an arrival */
            t.next = get_next_event_time(1);
        }
        else {  /* Next event is a completition, find the server that has finished */
            e = next_event_block_one(event);
            t.next = event[e].t;  /* next event time  */
        }
        
        area += (t.next - t.current) * number; /* update integral  */
        t.current = t.next;                    /* advance the clock*/

        if (get_next_event_type(1) == 0) { /* Process an arrival */

            //printf("\nBLOCK1: Processing an arrival...\n");
            number++;

            if (number <= SERVERS_ONE) {
                /* se nel sistema ci sono al più tanti job quanti i server allora calcola un tempo di servizio */
                lastArrival = t.current;
                service = get_service_block_one();
                s = find_one_block_one(event); 
                sum[s].service += service;
                sum[s].served++;
                event[s].t = t.current + service; /* Aggiorna l'istante del prossimo evento su quel server (partenza) */
                event[s].x = 1;
            }

            lastArrival = t.current;
        }
        else { /* Process a departure from server s */

            //printf("\nBLOCK1: Processing a departure...\n");
            index++;
            number--; /* il job è stato completato */
            s = e;

            //printf("\tDeparture: %6.2f\n", event[s].t);
            depTime = event[s].t;
            if (number >= SERVERS_ONE)
            { /* se ci sono job in coda allora assegniamo un nuovo job
             con un nuovo tempo di servizio al
             server appena liberato */
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

        /* L' orchestrator deve sapere quale sarà il prossimo evento di questo blocco */
        e = next_event_block_one(event); /* next event index */
        if (e != -1) {
            update_next_event(1, event[e].t, 1); /* There is a next event for this block, update the global_info */
        }
        else {
            update_next_event(1, INFINITY, 0);
        }
        
        //printf("--------------------------\n\n");

        oper.sem_num = 0;
        oper.sem_op = 1;
        oper.sem_flg = 0;
        semop(mainSem, &oper, 1);
    }

    //printf("BLOCK1: Terminated, waiting for the orchestrator...\n");
    /* attendi che l'orchestrator ti dia il via libera per stampare le statistiche*/
    oper.sem_num = 0;
    oper.sem_op = -1;
    oper.sem_flg = 0;
    semop(sem, &oper, 1);

    printf("\nBLOCK 1 STATISTICS:");
    printf("\n\nfor %ld jobs, forwarded %d\n", index, forwarded);
    printf("  avg interarrivals .. = %6.6f\n", lastArrival / index);
    printf("  avg wait ........... = %6.6f\n", area / index);
    printf("  avg # in node ...... = %6.6f\n", area / t.current);

    /* Write statistics on file */
    fp = fopen(FILENAME_WAIT_BLOCK1, "a");
    fprintf(fp,"%6.6f\n", area / index);
    fclose(fp);

    for (s = 0; s < SERVERS_ONE; s++) /* adjust area to calculate */
        area -= sum[s].service;        /* averages for the queue   */

    printf("  avg delay .......... = %6.6f\n", area / index);
    printf("  avg # in queue ..... = %6.6f\n", area / t.current);
    printf("\nthe server statistics are:\n\n");
    printf("    server     utilization     avg service        share\n");
    for (s = 0; s < SERVERS_ONE; s++) {
        printf("%8d %14.3f %15.2f %15.3f\n", s, sum[s].service / t.current,
               sum[s].service / sum[s].served,
               (double)sum[s].served / index);
        totalService += sum[s].service / sum[s].served;
        totalUtilization += sum[s].service / t.current;
    }
    
    avgService = totalService / SERVERS_ONE;

    printf("\n   avg service ........ = %6.6f\n", avgService / SERVERS_ONE);
    printf("   avg utilization .... = %6.6f\n", totalUtilization / SERVERS_ONE);

    /* Write statistics on file */
    fp = fopen(FILENAME_DELAY_BLOCK1, "a");
    fprintf(fp,"%6.6f\n", area / index);
    fclose(fp);

    printf("\n");
    
    pthread_exit((void *)0);
}