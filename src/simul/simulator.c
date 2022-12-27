#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

#include "simulator_helper.h"
#include "block1_tickets/block1_helper.h"
#include "block2_dis/block2_helper.h"
#include "block3_norm/block3_helper.h"
#include "block4_contr/block4_helper.h"
#include "block5_storage/block5_helper.h"

/* ----------- GLOBAL EXTERN VARIABLES ----------- */
global_info globalInfo[6];
departure_info departureInfo;

int block4Lost;
int block4ToExit;

int endSimulation;
/* ----------------------------------------------- */

int main() {

    int rep = 1;
	int blockNumber;
    double arrival = START;
    int onlineTickets;
    double prob;
    int totExtArrivals = 0;
    int exited = 0;

    system("clear");

    PlantSeeds(SEED);

    /* Create files for statistics */
    create_statistics_files();

    printf("\nSIMULATION STARTED...\n");

    /* Init global_info structure */
    init_global_info_structure();
    
    totExtArrivals = 0;
    onlineTickets = 0;
    exited = 0;
    endSimulation = 0;
    block4Lost = 0;
    block4ToExit = 0;

    /* Set first external arrival and departure info */
    arrival = START;
    arrival = get_external_arrival(arrival, INT);
    update_next_event(0, arrival, 0);
    departureInfo.time = -1;

    printf("\nAfter the first get_external_arrival\n");
    for (int i=0; i<=5; i++) {
        printf("\n   Block %d: %6.2f | %d", i, globalInfo[i].time, globalInfo[i].eventType);
    }

    while (1) {
    
        blockNumber = get_next_event();  /* Takes the index of the block with the most imminent event */
        switch (blockNumber) {
            
            case 0: /* External arrival */

                //printf("\nThe orchestrator is generating a new external arrival...\n");
                
                totExtArrivals++;

                /* Check probability to route the external arrival */
                if (get_probability() > ONLINE_PROBABILITY) { /* ticket arrival */
                    /* Set arrival for Block 1 */
                    update_next_event(1, arrival, 0);
                } 
                else { /* online arrival */
                    onlineTickets++;
                    if (get_probability() <= DISABLED_PROBABILITY) { /* disabled person */
                        /* Set arrival for Block 2 */
                        update_next_event(2, arrival, 0);
                    }
                    else { /* non-disabled person */
                        /* Set arrival for Block 3 */
                        update_next_event(3, arrival, 0);
                    }
                }

                /* Genera il prossimo istante di arrivo ed aggiorna la globalInfo[0] */
                arrival = get_external_arrival(arrival, INT);
                if (arrival > STOP) {
                    update_next_event(0, INFINITY, -1);
                }   
                else {
                    update_next_event(0, arrival, 0);
                }
                continue;

            case 1:
                block1();
                break;

            case 2:
                block2();
                break;

            case 3:
                block3();
                break;

            case 4:
                block4();
                break;

            case 5:
                block5();
                break;

            case -1:
                printf("\n\n[ORCHESTRATOR]: All the blocks have finished! \n");
                goto exit;
                break;

            default:
                printf("\n[WARNING]: Default case\n");
                exit(-1);
        }

        if (departureInfo.time != -1) {  /* vuol dire che c'è stata una partenza dal blocco che ha appena terminato quindi un arrivo nel blocco successivo */

            if (departureInfo.blockNum == 1) { /* Departure from block 1 to block 2 or 3 */

                /* Check probability to route a departure from block 1 */
                if (get_probability() <= DISABLED_PROBABILITY) { /* disabled person */
                    /* Set arrival for Block 2 */
                    update_next_event(2, departureInfo.time, 0);
                }
                else { /* non-disabled person */
                    /* Set arrival for Block 3 */
                    update_next_event(3, departureInfo.time, 0);
                }
            }
            else if ((departureInfo.blockNum == 2) || (departureInfo.blockNum == 3)) { /* Departure from block 2 or 3 to block 4 */
                update_next_event(4, departureInfo.time, 0);
            }
            else if (departureInfo.blockNum == 4) { /* Departure from block 4 */
                prob = get_probability();
                if (prob <= STORAGE_PROBABILITY) { /* Departure from block 4 to block 5 */
                    update_next_event(5, departureInfo.time, 0);
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

            if ((totExtArrivals == exited) && (get_next_event_time(0) == INFINITY)) { /* If all the jobs have been processed, the simulation ends */
                printf("\n[ORCHESTRATOR]: All the jobs have been processed, the simulation ends, the orchestrator unlocks and wait all the blocks/n");
                printf("\n[ORCHESTRATOR]: totExtArrivals: %d | exited: %d | online tickets: %d\n", totExtArrivals, exited, onlineTickets);
                
                endSimulation = 1;
                for (int i=0; i<=5; i++)
                        printf("\n   Block %d: %6.2f | %d", i, globalInfo[i].time, globalInfo[i].eventType);

                printf("\n\n| ------------------------------- STATISTICS ------------------------------- |\n");
                block1();
                block2();
                block3();
                block4();
                block5();
                printf("\n| -------------------------------------------------------------------------- |\n\n");
            }
        }
    }

exit:    
    return 0;
}