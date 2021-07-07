CC = g++
CFLAGS = -g -O2
LIBS = -lm -lpthread -lX11
LDFLAGS = -L/usr/X11R6/lib

default:
	$(CC) -o carve *.cpp $(CFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f carve
