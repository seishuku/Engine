TARGET=engine

# model loading/drawing
OBJS=3ds.o 3ds_gl.o
OBJS+=obj.o obj_gl.o
OBJS+=md5.o md5_gl.o
OBJS+=beam_gl.o
OBJS+=quad_gl.o
OBJS+=skybox_gl.o

# image loading
OBJS+=qoi.o dds.o tga.o image.o

# core stuff
OBJS+=engine.o opengl.o math.o linux_x11.o
OBJS+=font.o
OBJS+=camera.o

CC=gcc
CFLAGS=-Wall -O3 -std=c17 -I/usr/X11/include
LDFLAGS=-L/usr/X11/lib -lGL -lX11 -lm

all: $(TARGET)

debug: CFLAGS+= -DDEBUG -D_DEBUG -g -ggdb -O1
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.c: .o
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	-rm *.o
	-rm $(TARGET)
