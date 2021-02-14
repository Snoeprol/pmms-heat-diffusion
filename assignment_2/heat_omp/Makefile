CURR_DIR=$(notdir $(basename $(shell pwd)))
PRJ=$(CURR_DIR)
SRC=$(filter-out $(wildcard ref*.c), $(wildcard *.c))
OBJ=$(patsubst %.c,%.o,$(SRC))

CC=gcc
INCLUDES=-I../../include
ifndef DEBUG
CFLAGS=-O3 -std=gnu99 -fopenmp
LIB=
else
CFLAGS=-O0 -g3 -std=gnu99
LIB=
endif

all: $(PRJ)

$(PRJ): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ $(LIB)

clean:
	-rm -f $(OBJ) $(PRJ)

run:
	./heat_omp -n 2500 -m 2500 -c 2500x2500.pgm -t 2500x2500.pgm -i 100
