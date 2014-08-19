all:$(LIB)

FILE = $(shell find . -name "*.c")
OBJ=$(FILE:.c=.o)
$(LIB):$(OBJ)
	$(AR) -r $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
