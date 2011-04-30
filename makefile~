CC = g++
CFLAGS = -g -Wall -O0
LDFLAGS = -g -lpthread

COORD = coordinator
COORD_OBJ = coordinator.o socket.o 

DSM = dsm
DSM_OBJ = dsm.o socket.o 

all: $(DSM) $(COORD)

$(DSM): $(DSM_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(COORD): $(COORD_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.cc
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	-rm -rf *.o $(COORD) $(DSM) 
