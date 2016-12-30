EXTRA_INCLUDE=
EXTRA_LIBDIR=
ARGS=-Wall -O4

TMP=/dev/shm


all:	clock luminace

$(TMP)/clock.o:	clock.c config.h
	gcc -c $< -lwiringPi -o $@ $(ARGS) $(EXTRA_INCLUDE) -std=gnu11

$(TMP)/luminace.o:	luminace.c config.h
	gcc -c $< -lwiringPi -o $@ $(ARGS) $(EXTRA_INCLUDE)  -std=c99


clock:	$(TMP)/clock.o 
		gcc $^ -lwiringPi -lrt -o $@ $(ARGS) $(EXTRA_LIBDIR) 

luminace:	$(TMP)/luminace.o 
		gcc $^ -lwiringPi -o $@ $(ARGS) $(EXTRA_LIBDIR)

run:	clock
	sudo ./clock -f

clean:
	rm -f $(TMP)/*.o
	rm -f clock
	rm -f luminace

