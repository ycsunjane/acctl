all:acser

LIB:= $(LIBDIR)/$(LIB)

FILE = $(shell find . -name "*.c")
OBJ = $(FILE:.c=.o)
acser:$(OBJ) $(LIB)
	$(CC) $^ $(LDFLAGS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

