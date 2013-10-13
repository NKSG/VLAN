CC=gcc
CFLAGS=-lrt
DEPS = proxy.h
OBJ = proxy.o

%.o: %.c $(DEPS)
    $(CC) $< -c -o $@ $(CFLAGS)

proxy: $(OBJ)
    $(CC) $^ -o $@ $(CFLAGS)

$(OBJ) : proxy.h

