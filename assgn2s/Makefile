CC = mpicc
CFLAGS = -O2 -Wall -std=c11

all: pc_model wp_model

pc_model: pc_model.c
	$(CC) $(CFLAGS) -o pc_model pc_model.c

wp_model: wp_model.c
	$(CC) $(CFLAGS) -o wp_model wp_model.c

clean:
	rm -f pc_model wp_model *.o
