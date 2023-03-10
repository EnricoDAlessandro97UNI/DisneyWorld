#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "finite_helper.h"
#include "block1_tickets/block1_helper.h"
#include "block2_dis/block2_helper.h"
#include "block3_norm/block3_helper.h"
#include "block4_contr/block4_helper.h"
#include "block5_storage/block5_helper.h"

/* ----------- GLOBAL EXTERN VARIABLES ----------- */
global_info globalInfo[8];
departure_info departureInfo;

int endSimulation;
int changeConfig;

int block4Lost;
int block4ToExit;

double glblWaitBlockOne;
double glblWaitBlockTwo;
double glblWaitBlockThree;
double glblWaitBlockFour;
double glblWaitBlockFive;

int sampling;
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
    double currentSamplingInterval = SAMPLING;
    double glblWait = 0.0;

    PlantSeeds(SEED);

    /* Create files for statistics */
    create_statistics_files();

    fp = fopen(FILENAME_WAIT_GLOBAL, "a");

    system("clear");
    
    rep = 1;
    while (rep <= REP) {

        counter = 1;
        currentSamplingInterval = SAMPLING;

        printf("\n| ----------------------------------------------------------------------------------------- |\n");
        printf("\nSIMULATION REP: %d\n", rep);

        /* Init global_info structure */
        init_global_info_structure();

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
        changeConfig = 0;
        
        block4Lost = 0;
        block4ToExit = 0;

        glblWait = 0.0;
        glblWaitBlockOne = 0.0;
        glblWaitBlockTwo = 0.0;
        glblWaitBlockThree = 0.0;
        glblWaitBlockFour = 0.0;
        glblWaitBlockFive = 0.0;   

        sampling = 0; 

        /* Set first external arrival and departure info */
        arrival = START;
        arrival = get_external_arrival(arrival, INT1);
        update_next_event(0, arrival, 0, -1);
        departureInfo.time = -1;

        while (1) {
            
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
                    if (changeConfig == 2)
                        arrival = get_external_arrival(arrival, INT2);
                    else
                        arrival = get_external_arrival(arrival, INT1);

                    if (arrival > STOP) { /* No more external arrivals */
                        update_next_event(0, INFINITY, -1, -1);
                    }   
                    else {
                        update_next_event(0, arrival, 0, -1);
                    }
                    continue;

                case 1: /* Block 1 */
                    block1();
                    break;

                case 2: /* Block 2 */
                    block2();
                    break;

                case 3: /* Block 3 */
                    block3();
                    break;

                case 4: /* Block 4 */
                    block4();
                    break;

                case 5: /* Block 5 */
                    block5();
                    break;

                case 6: /* Event update stats */

                    glblWait = glblWaitBlockOne + glblWaitBlockTwo + glblWaitBlockThree + glblWaitBlockFour + glblWaitBlockFive;

                    /* Write stats on file */
                    //fp = fopen(FILENAME_WAIT_GLOBAL, "a");
                    fprintf(fp,"%6.6f\n", glblWait);
                    //fclose(fp);

                    sampling = 1;
                    block1();
                    block2();
                    block3();
                    block4();
                    block5();
                    sampling = 0;

                    counter++;
                    currentSamplingInterval = SAMPLING*counter;
                    if (currentSamplingInterval > STOP) 
                        update_next_event(6, INFINITY, -1, -1);
                    else 
                        update_next_event(6, currentSamplingInterval, -1, -1);
                    continue;

                case 7:
                    /* Event change time slot */
                    printf("\n[ORCHESTRATOR]: Change time slot | totExtArrivals: %d | exited: %d\n", totExtArrivals, exited);

                    changeConfig = 1;

                    /* Sblocco tutti i blocchi in ordine in modo che cambino configurazione dei server */
                    block1();
                    block2();
                    block3();
                    block4();
                    block5();

                    update_next_event(7, INFINITY, -1, -1);

                    changeConfig = 2;

                    continue;

                case -1:
                    /* Execution finished */
                    printf("\n\n[ORCHESTRATOR]: All the blocks have finished! | totExtArrivals: %d | exited: %d\n", totExtArrivals, exited);
                    goto nextRep;
                    break;

                default:
                    printf("\n[WARNING]: Default case\n");
                    exit(-1);
            }

            /* Se il blocco che ha appena terminato ha processato
            * una partenza, questa deve essere posta come arrivo per il blocco
            * successivo. Ogni blocco deve quindi 'restituire' qualcosa al
            * orchestrator in modo tale che esso sappia se deve inserire un nuovo
            * arrivo nella lista di un blocco */
            if (departureInfo.time != -1) {  /* vuol dire che c'?? stata una partenza dal blocco che ha appena terminato quindi un arrivo nel blocco successivo */

                if (departureInfo.blockNum == 1) { /* Departure from block 1 to block 2 or 3 */
                    if (departureInfo.disabled) { /* disabled person */
                        /* Set arrival for Block 2 */
                        update_next_event(2, departureInfo.time, 0, 1);
                    }
                    else { /* non-disabled person */
                        /* Set arrival for Block 3 */
                        update_next_event(3, departureInfo.time, 0, 0);
                    }
                }
                else if (departureInfo.blockNum == 2) { /* Departure from block 2 to block 4 */
                    update_next_event(4, departureInfo.time, 0, 1);
                }
                else if (departureInfo.blockNum == 3) { /* Departure from block 3 to block 4 */
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
            }

            /* if the number of processed jobs is equal to the jobs exited the system 
                   and if there are no more arrivals from outside the simulation ends */
            if ((totExtArrivals == exited) && (get_next_event_time(0) == INFINITY)) {
                printf("\n[ORCHESTRATOR]: All the jobs have been processed, the simulation ends, the orchestrator unlocks and wait all the blocks\n");
                printf("[ORCHESTRATOR]: totExtArrivals: %d | exited: %d | online tickets: %d\n", totExtArrivals, exited, onlineTickets);
                printf("[ORCHESTRATOR]: Dis: %d | NoDis: %d | onlineTickDis: %d | onlineTickNoDis: %d | ticketsDis: %d | ticketsNoDis: %d\n", dis, noDis, onlineTicketsDis, onlineTicketsNoDis, ticketDis, ticketNoDis);
                
                endSimulation = 1;
                block1();
                block2();
                block3();
                block4();
                block5();
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