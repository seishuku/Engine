TARGET=engine
OBJS=3ds.o 3ds_gl.o
OBJS+=obj.o obj_gl.o
OBJS+=md5.o md5_gl.o
OBJS+=dds.o tga.o image.o
OBJS+=engine.o opengl.o math.o linux_x11.o
OBJS+=font.o
OBJS+=camera.o
OBJS+=beam_gl.o
OBJS+=quad_gl.o
OBJS+=skybox_gl.o

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
