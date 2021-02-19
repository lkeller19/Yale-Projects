CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -g3

Rank: rank_main.o lugraph.o gmap.o string_key.o mergesort.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

rank_main.o: lugraph.h

lugraph.o: lugraph.h mergesort.h gmap.h string_key.h

gmap.o: gmap.h

string_key.o: string_key.h

mergesort.o: mergesort.h
