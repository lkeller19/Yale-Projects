CC=gcc
CFLAGS= -Wall -std=c99 -g3 -pedantic 

all: BlottoGA Unit

BlottoGA: blotto_ga.o arrays.o blotto.o ga_helpers.o strategy.o population.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

Unit: ga_unit.o ga_helpers.o population.o strategy.o arrays.o blotto.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

arrays.o: arrays.h

blotto_ga.o: arrays.h population.h ga_helpers.h strategy.h

strategy.o: strategy.h blotto.h arrays.h

population.o: population.h strategy.h 

ga_unit.o: ga_helpers.h strategy.h population.h 

ga_helpers.o: ga_helpers.h strategy.h population.h

blotto.o: blotto.h