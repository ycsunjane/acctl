all:acser
LIB:= $(LIBDIR)/$(LIB)

CFLAGS+=-DSERVER

FILE = $(shell find . -name "*.c")
OBJ = $(FILE:.c=.o)
acser:$(OBJ) $(LIB)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

export LIB LIBDIR CFLAGS
$(LIB): FORCE
	@$(MAKE) -C $(LIBDIR)

FORCE:          
PHONY += FORCE
.PHONY: $(PHONY)
