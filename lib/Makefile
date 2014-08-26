all:$(LIB)

CMD=$(LIBDIR)/.tmp
FILE = $(shell find . -name "*.c")
OBJ=$(FILE:.c=.o)
$(LIB):$(OBJ)
	$(AR) -r $@ $^

%.o:%.c $(CMD)
	$(CC) $(CFLAGS) -c $< -o $@

INFO=$(shell cat $(CMD) 2>/dev/null)
$(CMD): FORCE
	-@if [ "$(CC) $(CFLAGS)" != "$(INFO)" ]; then \
		echo "$(CC) $(CFLAGS)" > $@; \
	fi

FORCE:          
PHONY += FORCE
.PHONY: $(PHONY)
