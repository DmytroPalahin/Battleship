CFLAGS = -Wall

VERSION = gtk+-3.0
CGTK = `pkg-config $(VERSION) --cflags`
LGTK = `pkg-config $(VERSION) --libs`

battleship.exe: battleship.o
	gcc $^ -o $@ $(LGTK)

battleship.o: battleship.c
	gcc -c $< -o $@ $(CFLAGS) $(CGTK)

clean:
	rm *~ *.o *.exe
