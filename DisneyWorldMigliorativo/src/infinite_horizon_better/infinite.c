#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "infinite_helper.h"
#include "block1_tickets/block1_helper.h"
#include "block2_dis/block2_helper.h"
#include "block3_norm/block3_helper.h"
#include "block4_contr/block4_helper.h"
#include "block5_storage/block5_helper.h"

/* ----------- GLOBAL EXTERN VARIABLES ----------- */
global_info globalInfo[6];
departure_info departureInfo;

int endSimulation;

int block4Lost;
int block4ToExit;

double glblWaitBlockOne;
double glblWaitBlockTwo;
double glblWaitBlockThree;
double glblWaitBlockFour;
double glblWaitBlockFive;

double sampling;
/* ----------------------------------------------- */

int main() {

    FILE *fp;

    int rep = 1;
	int blockNumber;
    double arrival = START;
    int onlineTickets;
    int onlineTicketsDis;
    int onlineTicketsNoDis;
    int ticketDis;
    int ticketNoDis;
    int dis;
    int noDis;
    double prob;
    int totExtArrivals = 0;
    int exited = 0;
    int counter = 1;
    int currentSamplingInterval = B;
    double glblWait = 0.0;

    PlantSeeds(SEED);

    /* Create files for statistics */
    create_statistics_files();

    fp = fopen(FILENAME_WAIT_GLOBAL, "a");

    system("clear");
    
    /* La simulazione finirà quando il numero di job processati sarà esattamente uguale a quelli previsti */
    /* Devo mettere un evento ogni volta che vengono processati B jobs */
    /* La simulazione per quei B jobs deve essere ripetuta per REP volte 256 */
    //while (exited < N) { /* exited < N*/

    rep = 1;
    while (rep <= REP) {

        /* Init global_info structure */
        init_global_info_structure();

        counter = 1;
        currentSamplingInterval = B;

        totExtArrivals = 0;
        onlineTickets = 0;
        onlineTicketsDis = 0;
        onlineTicketsNoDis = 0;
        ticketDis = 0;
        ticketNoDis = 0;
        dis = 0;
        noDis = 0;
        exited = 0;
        endSimulation = 0;

        sampling = 0;

        block4Lost = 0;
        block4ToExit = 0;

        glblWait = 0.0;
        glblWaitBlockOne = 0.0;
        glblWaitBlockTwo = 0.0;
        glblWaitBlockThree = 0.0;
        glblWaitBlockFour = 0.0;
        glblWaitBlockFive = 0.0;    

        /* Set first external arrival and departure info */
        arrival = START;
        arrival = get_external_arrival(arrival, INT);
        update_next_event(0, arrival, 0, -1);
        departureInfo.time = -1;

        printf("\n| ----------------------------------------------------------------------------------------- |\n");
        printf("\nSIMULATION REP: %d | INITIAL_SAMPLING: %d\n", rep, currentSamplingInterval);

        while (1) {
            
            sampling = 0;
            // printf("\nTotal Jobs: %d\n", totExtArrivals);
            // for (int i=0; i<=5; i++)
            //             printf("\n   Block %d: %6.2f | %d", i, globalInfo[i].time, globalInfo[i].eventType);

            blockNumber = get_next_event();  /* Takes the index of the block with the most imminent event */
            switch (blockNumber) {
                
                case 0: /* External arrival */
                    
                    totExtArrivals++;

                    /* Check probability to route the external arrival */
                    if (get_probability() > ONLINE_PROBABILITY) { /* ticket arrival */
                        /* Set arrival for Block 1 */
                        if (get_probability() <= DISABLED_PROBABILITY) { /* disabled person */
                            dis++;
                            ticketDis++;
                            /* Set arrival for Block 1 */
                            update_next_event(1, arrival, 0, 1);
                        }
                        else { /* non-disabled person */
                            noDis++;
                            ticketNoDis++;
                            /* Set arrival for Block 1 */
                            update_next_event(1, arrival, 0, 0);
                        }
                    } 
                    else { /* online arrival */
                        onlineTickets++;
                        if (get_probability() <= DISABLED_PROBABILITY) { /* disabled person */
                            onlineTicketsDis++;
                            dis++;
                            /* Set arrival for Block 2 */
                            update_next_event(2, arrival, 0, 1);
                        }
                        else { /* non-disabled person */
                            onlineTicketsNoDis++;
                            noDis++;
                            /* Set arrival for Block 3 */
                            update_next_event(3, arrival, 0, 0);
                        }
                    }

                    /* Generate the next external arrival instant */
                    arrival = get_external_arrival(arrival, INT);
                    if (totExtArrivals == N) { /* No more external arrivals */
                        update_next_event(0, INFINITY, -1, -1);
                    }   
                    else {
                        update_next_event(0, arrival, 0, -1);
                    }

                    continue;

                case 1: /* Block1 */
                    block1();
                    break;

                case 2: /* Block2 */
                    block2();
                    break;

                case 3: /* Block3 */
                    block3();
                    break;

                case 4: /* Block4 */
                    block4();
                    break;

                case 5: /* Block5 */
                    block5();
                    break;

                case -1: /* Execution finished */
                    printf("\n\n[ORCHESTRATOR]: All the blocks have finished! \n");
                    goto nextRep;
                    break;

                default:
                    continue;
            }

            /* Se il blocco che ha appena terminato ha processato
            * una partenza, questa deve essere posta come arrivo per il blocco
            * successivo. Ogni blocco deve quindi 'restituire' qualcosa al
            * orchestrator in modo tale che esso sappia se deve inserire un nuovo
            * arrivo nella lista di un blocco */
            if (departureInfo.time != -1) {  /* vuol dire che c'è stata una partenza dal blocco che ha appena terminato quindi un arrivo nel blocco successivo */

                if (departureInfo.blockNum == 1) { /* Departure from block 1 to block 2 or 3 */
                    
                    /* Check if disabled */
                    if (departureInfo.disabled) {
                        /* Set arrival for Block 2 */
                        update_next_event(2, departureInfo.time, 0, 1);
                    }
                    else {
                        /* Set arrival for Block 3 */
                        update_next_event(3, departureInfo.time, 0, 0);
                    }
                }
                else if (departureInfo.blockNum == 2) { /* Departure from block 2 to block 4 */
                    update_next_event(4, departureInfo.time, 0, 1);
                }
                else if (departureInfo.blockNum == 3) { /* Departure from 3 to block 4 */
                    update_next_event(4, departureInfo.time, 0, 0);
                }
                else if (departureInfo.blockNum == 4) { /* Departure from block 4 */
                    prob = get_probability();
                    if (prob <= STORAGE_PROBABILITY) { /* Departure from block 4 to block 5 */
                        update_next_event(5, departureInfo.time, 0, -1);
                    } 
                    else if ((prob > STORAGE_PROBABILITY) && (prob <= LOST_PROBABILITY)) { /* Departure from block 4 to the exit */
                        block4ToExit++;
                        exited++;
                    }
                    else { /* Departure from block 4 to the exit */
                        block4Lost++;
                        exited++;
                    }
                } else { /* Departure from block 5 to the exit */
                    exited++;
                }

                departureInfo.time = -1; /* In any case resets departure info to -1 */

                /* Check if stats update is needed */
                if (exited == currentSamplingInterval) {
                    //printf("\n  -> CURRENT SAMPLING: %d\n", currentSamplingInterval);
                    glblWait = glblWaitBlockOne + glblWaitBlockTwo + glblWaitBlockThree + glblWaitBlockFour + glblWaitBlockFive;

                    /* Write stats on file */
                    fprintf(fp,"%6.6f\n", glblWait);
                    
                    sampling = 1;
                    block1();
                    block2();
                    block3();
                    block4();
                    block5();
                    sampling = 0;

                    /* update next sampling */
                    counter++;
                    currentSamplingInterval = B*counter;
                }

                /* if the number of processed jobs is equal to the jobs exited the system 
                   and if there are no more arrivals from outside the simulation ends */
                if ((totExtArrivals == exited) && (get_next_event_time(0) == INFINITY)) { /* If all the jobs have been processed, the simulation ends */
                    printf("\n[ORCHESTRATOR]: All the jobs have been processed, the simulation ends, the orchestrator unlocks and wait all the blocks\n");
                    printf("[ORCHESTRATOR]: totExtArrivals: %d | exited: %d | online tickets: %d\n", totExtArrivals, exited, onlineTickets);
                    printf("[ORCHESTRATOR]: Dis: %d | NoDis: %d | onlineTickDis: %d | onlineTickNoDis: %d | ticketsDis: %d | ticketsNoDis: %d\n", dis, noDis, onlineTicketsDis, onlineTicketsNoDis, ticketDis, ticketNoDis);

                    endSimulation = 1;
                    block1();
                    block2();
                    block3();
                    block4();
                    block5();   

                    // for (int i=0; i<=5; i++)
                    //     printf("\n   Block %d: %6.2f | %d", i, globalInfo[i].time, globalInfo[i].eventType);

                }
            }
        }

    nextRep:       

        printf("  -> SUCCESS\n");
        
        printf("\n| ----------------------------------------------------------------------------------------- |\n");

        rep++;
    }

    printf("\n");

    fclose(fp);

    return 0;
}