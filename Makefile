CC=gcc
CFLAGS=-lrt -m32
DEPS = proxy.h
OBJ = proxy.o

%.o: %.c $(DEPS)
	$(CC) $< -c -o $@ $(CFLAGS)

cs352proxy: $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJ) : proxy.h

