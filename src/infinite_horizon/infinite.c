#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "infinite_helper.h"
#include "block1_tickets/block1_helper.h"
#include "block2_dis/block2_helper.h"
#include "block3_norm/block3_helper.h"
#include "block4_contr/block4_helper.h"
#include "block5_storage/block5_helper.h"

/* ----------- GLOBAL EXTERN VARIABLES ----------- */
pthread_t tid[5];
global_info globalInfo[8];
departure_info departureInfo;

int sem;
int mainSem;

int endSimulation;
int changeConfig;

int block4Lost;
int block4ToExit;

double glblWaitBlockOne;
double glblWaitBlockTwo;
double glblWaitBlockThree;
double glblWaitBlockFour;
double glblWaitBlockFive;

int updateStatistics;
/* ----------------------------------------------- */

/* Creation of threads for block management */
void create_threads() {

    int ret;

    ret = pthread_create(&tid[0], NULL, block1, NULL);
    if(ret != 0){
        perror("pthread create error\n");
        exit(1);
    }

    ret = pthread_create(&tid[1], NULL, block2, NULL);
    if(ret != 0){
        perror("pthread create error\n");
        exit(1);
    }
    
    ret = pthread_create(&tid[2], NULL, block3, NULL);
    if(ret != 0){
        perror("pthread create error\n");
        exit(1);
    }   
    
    ret = pthread_create(&tid[3], NULL, block4, NULL);
    if(ret != 0){
        perror("pthread create error\n");
        exit(1);
    }

    ret = pthread_create(&tid[4], NULL, block5, NULL);
    if(ret != 0){
        perror("pthread create error\n");
        exit(1);
    }
}


int main() {

    FILE *fp;

    int rep = 1;
	int blockNumber;
    double arrival = START;
    double prob;
    int totExtArrivals = 0;
    int exited = 0;
    int counter = 1;
    double currentSamplingInterval = SAMPLING;
    double glblWait = 0.0;

    struct sembuf oper;

    key_t key = IPC_PRIVATE;
    key_t mkey = IPC_PRIVATE;

    system("clear");

    /* Initializes main semaphore */
    mainSem = semget(mkey,1,IPC_CREAT|0666);
    if(mainSem == -1) {
        printf("semget error\n");
        exit(EXIT_FAILURE);
    }

    /* Initializes semaphore for threads */
    sem = semget(key,5,IPC_CREAT|0666);
    if(sem == -1) {
        printf("semget error\n");
        exit(EXIT_FAILURE);
    }

    PlantSeeds(SEED);

    /* Create files for statistics */
    create_statistics_files();

    rep = 1;
    while (rep <= REP) {

        counter = 1;
        currentSamplingInterval = SAMPLING;

        printf("\nSIMULATION REP: %d | SAMPLING: %6.2f\n", rep, currentSamplingInterval);

        /* Init global_info structure */
        init_global_info_structure();

        semctl(mainSem, 0, SETVAL, 0);
        for (int i=0; i<5; i++)
            semctl(sem, i, SETVAL, 0);

        totExtArrivals = 0;
        exited = 0;
        endSimulation = 0;
        changeConfig = 0;
        
        glblWait = 0.0;
        glblWaitBlockOne = 0.0;
        glblWaitBlockTwo = 0.0;
        glblWaitBlockThree = 0.0;
        glblWaitBlockFour = 0.0;
        glblWaitBlockFive = 0.0;    

        /* Creation of threads */
        create_threads();

        /* Waiting for threads creation */
        oper.sem_num = 0;
        oper.sem_op = -5;
        oper.sem_flg = 0;
        semop(mainSem,&oper,1);

        /* Set first external arrival and departure info */
        arrival = START;
        arrival = get_external_arrival(arrival, INT1);
        update_next_event(0, arrival, 0);
        departureInfo.time = -1;

        while (1) {
        
            blockNumber = get_next_event();  /* Takes the index of the block with the most imminent event */
            switch (blockNumber) {
                
                case 0: /* External arrival */
                    
                    totExtArrivals++;

                    /* Check probability to route the external arrival */
                    if (get_probability() > ONLINE_PROBABILITY) { /* ticket arrival */
                        /* Set arrival for Block 1 */
                        update_next_event(1, arrival, 0);
                    } 
                    else { /* online arrival */
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
                    if (changeConfig == 2)
                        arrival = get_external_arrival(arrival, INT2);
                    else
                        arrival = get_external_arrival(arrival, INT1);

                    if (arrival > STOP) {
                        update_next_event(0, INFINITY, -1);
                    }   
                    else {
                        update_next_event(0, arrival, 0);
                    }
                    continue;

                case 1:
                    /* Unlock block 1 */
                    oper.sem_num = 0;
                    oper.sem_op = 1;
                    oper.sem_flg = 0;
                    semop(sem,&oper,1);
                    break;

                case 2:
                    /* Unlock block 2 */
                    oper.sem_num = 1;
                    oper.sem_op = 1;
                    oper.sem_flg = 0;
                    semop(sem,&oper,1);
                    break;

                case 3:
                    /* Unlock block 3 */
                    oper.sem_num = 2;
                    oper.sem_op = 1;
                    oper.sem_flg = 0;
                    semop(sem,&oper,1);
                    break;

                case 4:
                    /* Unlock block 4 */
                    oper.sem_num = 3;
                    oper.sem_op = 1;
                    oper.sem_flg = 0;
                    semop(sem,&oper,1);
                    break;

                case 5:
                    /* Unlock block 5 */
                    oper.sem_num = 4;
                    oper.sem_op = 1;
                    oper.sem_flg = 0;
                    semop(sem,&oper,1);
                    break;

                case 6:
                    /* Event update statistics */
                    // Senza sbloccare i thread l'orchestrator si troverà i valori dei tempi di risposta globali aggiornati agli ultimi istanti
                    // Si calcola il tempo globale e lo scrive su file
                    //printf("\n  -> CURRENT SAMPLING: %6.2f\n", currentSamplingInterval);
                    glblWait = glblWaitBlockOne + glblWaitBlockTwo + glblWaitBlockThree + glblWaitBlockFour + glblWaitBlockFive;

                    /* Fare un IF per controllare chi è il figlio di puttana che è rimasto ad infinito */
                    //printf("    glblWaitBlockOne: %6.2f\n", glblWaitBlockOne);
                    if (glblWaitBlockOne == INFINITY) {
                        printf("\nBLOCCO 1\n");
                        exit(EXIT_FAILURE);
                    }
                    //printf("    glblWaitBlockTwo: %6.2f\n", glblWaitBlockTwo);
                    if (glblWaitBlockTwo == INFINITY) {
                        printf("\nBLOCCO 2\n");
                        exit(EXIT_FAILURE);
                    }
                    //printf("    glblWaitBlockThree: %6.2f\n", glblWaitBlockThree);
                    if (glblWaitBlockThree == INFINITY) {
                        printf("\nBLOCCO 3\n");
                        exit(EXIT_FAILURE);
                    }
                    //printf("    glblWaitBlockFour: %6.2f\n", glblWaitBlockFour);
                    if (glblWaitBlockFour == INFINITY) {
                        printf("\nBLOCCO 4\n");
                        exit(EXIT_FAILURE);
                    }
                    //printf("    glblWaitBlockFive: %6.2f\n", glblWaitBlockFive);
                    if (glblWaitBlockFive == INFINITY) {
                        printf("\nBLOCCO 5\n");
                        exit(EXIT_FAILURE);
                    }

                    /* Write statistics on file */
                    fp = fopen(FILENAME_WAIT_GLOBAL, "a");
                    fprintf(fp,"%6.6f\n", glblWait);
                    fclose(fp);
                    // Dopodichè aggiorna il tempo di campionamento successivo e, se maggiore di stop, porre l'evento successivo ad infinito
                    counter++;
                    currentSamplingInterval = SAMPLING*counter;
                    if (currentSamplingInterval > STOP) 
                        update_next_event(6, INFINITY, -1);
                    else 
                        update_next_event(6, currentSamplingInterval, -1);
                    continue;

                case 7:
                    /* Event change time slot */
                    changeConfig = 1;

                    /* Sblocco tutti i thread in ordine in modo che cambino configurazione dei server */
                    for (int i=0; i<5; i++) {  // ***************** reimpostare a 5 *************************
                        oper.sem_num = i;
                        oper.sem_op = 1;
                        oper.sem_flg = 0;
                        semop(sem, &oper, 1);
                    }

                    /* Waiting for threads change configuration */
                    oper.sem_num = 0;
                    oper.sem_op = -5;  // ***************** reimpostare a 5 *************************
                    oper.sem_flg = 0;
                    semop(mainSem,&oper,1);

                    update_next_event(7, INFINITY, -1);
                    changeConfig = 2;

                    continue;

                case -1:
                    /* Execution finished */
                    //printf("\n\n[ORCHESTRATOR]: All the blocks have finished! \n");
                    goto nextRep;
                    break;

                default:
                    continue;
            }

            /* Waiting for thread operation */
            oper.sem_num = 0;
            oper.sem_op = -1;
            oper.sem_flg = 0;
            semop(mainSem, &oper, 1);

            /* Se il blocco che ha appena terminato ha processato
            * una partenza, questa deve essere posta come arrivo per il blocco
            * successivo. Ogni thread deve quindi 'restituire' qualcosa al
            * orchestrator in modo tale che esso sappia se deve inserire un nuovo
            * arrivo nella lista di un blocco */
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
                    //printf("\n[ORCHESTRATOR]: All the jobs have been processed, the simulation ends, the orchestrator unlocks and wait all the threads\n");
                    printf("\n\n[ORCHESTRATOR]: totExtArrivals: %d | exited: %d \n", totExtArrivals, exited);
                    endSimulation = 1;
                    unlock_waiting_threads();
                    oper.sem_num = 0;
                    oper.sem_op = -5;
                    oper.sem_flg = 0;
                    semop(mainSem, &oper, 1);
                }
            }
        }

    nextRep:
        /* Sblocco tutti i thread in ordine in modo che terminino */
        for (int i=0; i<5; i++) {
            oper.sem_num = i;
            oper.sem_op = 1;
            oper.sem_flg = 0;
            semop(sem, &oper, 1);
            pthread_join(tid[i], NULL);
            //printf("  BLOCK %d JOINED\n", i+1);
        }

        printf("  -> SUCCESS\n");
        
        rep++;  
    }

    printf("\n");

    return 0;
}