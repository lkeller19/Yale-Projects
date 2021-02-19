CC=gcc
CFLAGS= -Wall -std=c99 -g3 -pedantic 

all: Cooccur GmapUnit CooccurUnit

Cooccur: cooccur.o gmap.o cooccur_main.o string_key.o gmap_test_functions.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

GmapUnit: gmap.o gmap_unit.o string_key.o gmap_test_functions.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

CooccurUnit: cooccur.o cooccur_unit.o string_key.o gmap_test_functions.o gmap.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

cooccur.o: cooccur.h gmap.h string_key.h

coocur_unit.o: gmap_test_functions.h cooccur.h

gmap_unit.o: gmap.h gmap_test_functions.h string_key.h

cooccur_main.o: cooccur.h

gmap.o: gmap.h

gmap_test_functions.o: gmap_test_functions.h

