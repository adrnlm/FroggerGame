CC = -gcc
LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT
CFLAGS += -D__APPLE__ -Wno-deprecated-declarations
PROG = frogger

file:
	$(CC) $(LDFLAGS) $(CFLAGS) frogger.c -o $(PROG) -lm
