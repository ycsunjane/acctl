all:$(LIB)
PASSWORD=wirelesser
ifndef PASSWORD 
PASSWORD := $(shell read -p "please input chap passwd:" PASSWD; echo $$PASSWD)
endif
CFLAGS += -DPASSWD=\"$(PASSWORD)\"
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
