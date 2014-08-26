all:apctl

LIB:= $(LIBDIR)/$(LIB)

CFLAGS+=-DCLIENT

FILE = $(shell find . -name "*.c")
OBJ = $(FILE:.c=.o)
apctl:$(OBJ) $(LIB)
	$(CC) $^ $(LDFLAGS) -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

export LIB LIBDIR CFLAGS
$(LIB): FORCE
	@$(MAKE) -C $(LIBDIR)

FORCE:          
PHONY += FORCE
.PHONY: $(PHONY)
