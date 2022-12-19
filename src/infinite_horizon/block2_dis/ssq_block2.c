/* ------------------------------------------------------------------------- 
 * Name            : ssq_block2.c  (Single Server Queue)
 * Author          : Enrico D'Alessandro & Alessandro De Angelis
 * Language        : ANSI C
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <math.h>
#include <sys/sem.h>
#include "../infinite_helper.h"

#define M2 12

double get_service_block_two() {
    SelectStream(2);
    return (Exponential(M2));
}

void *block2() {

    FILE *fp;

    struct {
        double arrival;                 /* next arrival time                   */
        double completion;              /* next completion time                */
        double current;                 /* current time                        */
        double next;                    /* next (most imminent) event time     */
        double last;                    /* last arrival time                   */
    } t;
    struct {
        double node;                    /* time integrated number in the node  */
        double queue;                   /* time integrated number in the queue */
        double service;                 /* time integrated number in service   */
    } area = {0.0, 0.0, 0.0};
    long index = 0;                  /* used to count departed jobs         */
    long number = 0;                  /* number in the node                  */

    double lastArrival = 0.0;

    t.current = START;              /* set the clock                         */
    t.arrival = INFINITY;           /* schedule the first arrival            */
    t.completion = INFINITY;        /* the first event can't be a completion */

    int nextEvent;        /* Next event type */
    double depTime = 0;

    struct sembuf oper;
    /* Unlock the orchestrator */
    oper.sem_num = 0;
    oper.sem_op = 1;
    oper.sem_flg = 0;
    semop(mainSem, &oper, 1);

    while (1) {

        /* Wait for the start from the orchestrator */
        oper.sem_num = 1;
        oper.sem_op = -1;
        oper.sem_flg = 0;
        semop(sem, &oper, 1);

        /* Check for the end of the simulation */
        if (endSimulation == 1) {
            update_next_event(2, INFINITY, -1);
            oper.sem_num = 0;
            oper.sem_op = 1;
            oper.sem_flg = 0;
            semop(mainSem, &oper, 1);
            break;
        }

        /* Check for server configuration changes */
        if (changeConfig == 1) {
            /* Unlock the orchestrator */
            oper.sem_num = 0;
            oper.sem_op = 1;
            oper.sem_flg = 0;
            semop(mainSem, &oper, 1);

            continue;
        }

        //printf("\n-------- BLOCK 2 --------\n");

        nextEvent = get_next_event_type(2);
        t.next = get_next_event_time(2);  /* next event time */

        if (number > 0 && t.next != INFINITY) {  /* update integrals  */
            area.node += (t.next - t.current) * number;
            area.queue += (t.next - t.current) * (number - 1);
            area.service += (t.next - t.current);
        }
        t.current = t.next; /* advance the clock */

        /* For global wait statistics */
        if (index == 0) /* Statistics not yet ready */
            glblWaitBlockTwo = 0.0;
        else 
            glblWaitBlockTwo = area.node / index;

        if (nextEvent == 0) { /* Process an arrival */

            number++;

            t.arrival = t.current;
            lastArrival = t.arrival;

            if (number == 1)
                t.completion = t.current + get_service_block_two();
        } 
        else { /* Process a completion */

            depTime = t.current;

            index++;
            number--;
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

        update_next_event(2, t.completion, 1);

        //printf("--------------------------\n\n");

        oper.sem_num = 0;
        oper.sem_op = 1;
        oper.sem_flg = 0;
        semop(mainSem, &oper, 1);
    }

    //printf("\nBLOCK2: Terminated, waiting for the orchestrator...\n");
    oper.sem_num = 1;
    oper.sem_op = -1;
    oper.sem_flg = 0;
    semop(sem,&oper,1);

    /*
    printf("\nBLOCK 2 STATISTICS:");
    printf("\nfor %ld jobs\n", index);
    printf("   average interarrival time = %6.2f\n", lastArrival / index);
    printf("   average wait ............ = %6.2f\n", area.node / index);
    printf("   average delay ........... = %6.2f\n", area.queue / index);
    printf("   average service time .... = %6.2f\n", area.service / index);
    printf("   average # in the node ... = %6.2f\n", area.node / t.current);
    printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
    printf("   utilization ............. = %6.2f\n", area.service / t.current);
    */

    /* Write statistics on files */
    fp = fopen(FILENAME_WAIT_BLOCK2, "a");
    fprintf(fp,"%6.6f\n", area.node / index);
    fclose(fp);

    fp = fopen(FILENAME_DELAY_BLOCK2, "a");
    fprintf(fp,"%6.6f\n", area.queue / index);
    fclose(fp);

    return (0);
}
