ROOTNAME=sensorLib
CC=gcc
CFLAGS= -g -I.
DEPS = sensorThread.h
LIBS = -l pigpio -l pthread

OBJ = $(ROOTNAME).o sensorThread.o
#TESTOPTIONS = --leak-check=full

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ROOTNAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
clean:
	rm *.o $(ROOTNAME)
	
run: $(ROOTNAME)
		./$(ROOTNAME) $(TESTOPTIONS)
		
vrun: $(ROOTNAME)
		valgrind ./$(ROOTNAME) $(TESTOPTIONS)
		
