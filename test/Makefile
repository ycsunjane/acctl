all:dllser dllcli

LIB:= $(LIBDIR)/$(LIB)

dllser:dllser.o $(LIB)
	$(CC) $^ $(LDFLAGS) -o $@

dllcli:dllcli.o $(LIB)
	$(CC) $^ $(LDFLAGS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
