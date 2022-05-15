TARGET=engine

# model loading/drawing
OBJS=model/3ds.o model/3ds_gl.o
OBJS+=model/obj.o model/obj_gl.o
OBJS+=model/md5.o model/md5_gl.o
OBJS+=model/beam_gl.o
OBJS+=model/quad_gl.o
OBJS+=model/skybox_gl.o

# image loading
OBJS+=image/qoi.o
OBJS+=image/dds.o
OBJS+=image/tga.o
OBJS+=image/image.o

# core stuff
OBJS+=system/linux_x11.o
OBJS+=opengl/opengl.o
OBJS+=math/math.o
OBJS+=font/font.o
OBJS+=camera/camera.o
OBJS+=engine.o

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
	-rm camera/*.o
	-rm font/*.o
	-rm image/*.o
	-rm math/*.o
	-rm model/*.o
	-rm opengl/*.o
	-rm system/*.o
	-rm $(TARGET)
