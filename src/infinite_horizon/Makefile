# January 2, 2005
#
# Makefile for compiling accompanying software for
# "Discrete-Event Simulation: A First Course" by Park and Leemis
#

CC = gcc
CFLAGS = -g -Wall -Wextra
LDFLAGS = -lm

OBJFILES = block1 block2 block3 block4 block5 rgns.o infinite

all: $(OBJFILES)

infiniteF1: infinite.c infinite_helper.c block1_tickets/msq_block1.c block2_dis/ssq_block2.c block3_norm/msq_block3.c block4_contr/msq_block4.c block5_storage/msq_block5.c rngs.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

infiniteF2: infinite.c infinite_helper.c block1_tickets/msq_block1.c block2_dis/ssq_block2.c block3_norm/msq_block3.c block4_contr/msq_block4.c block5_storage/msq_block5.c rngs.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)	-D F

rngs.o: rngs.c rngs.h
	$(CC) -c $<


