CC = gcc
CFLAGS = -g -Wall -Wextra
LDFLAGS = -lm

simulF1:  simulator.c simulator_helper.c block1_tickets/msq_block1.c block2_dis/msq_block2.c block3_norm/msq_block3.c block4_contr/msq_block4.c block5_storage/msq_block5.c rngs.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

simulF2: simulator.c simulator_helper.c block1_tickets/msq_block1.c block2_dis/msq_block2.c block3_norm/msq_block3.c block4_contr/msq_block4.c block5_storage/msq_block5.c rngs.o
	$(CC) $(CFLAGS) $^ -D F -o $@ $(LDFLAGS)

rngs.o: rngs.c rngs.h
	$(CC) -c $<