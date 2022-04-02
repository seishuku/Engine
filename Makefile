TARGET=engine
OBJS=3ds.o dds.o engine.o font.o image.o math.o opengl.o tga.o

CC=gcc
CFLAGS=-Wall -O3 -c -I/usr/X11/include -g
LDFLAGS=-L/usr/X11/lib -lGL -lX11 -lm

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.c: .o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm *.o
	-rm $(TARGET)
