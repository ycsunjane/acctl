TOPDIR = $(CURDIR)
SCRIPT_DIR = $(TOPDIR)/scripts
export TOPDIR

ifeq ("$(origin V)", "command line")
  BUILD_VERBOSE = $(V)
endif
ifndef BUILD_VERBOSE
  BUILD_VERBOSE = 0
endif
ifeq ($(BUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
  MAKEFLAGS += --no-print-directory
endif

ifneq ($(findstring s,$(MAKEFLAGS)),)
  quiet=silent_
endif
export quiet Q BUILD_VERBOSE 

echo := :
quiet_echo := echo
silent_echo := :
echo := $($(quiet)echo)
include $(SCRIPT_DIR)/Kbuild.include


# gnu utils
CC = gcc 
LD = ld
AR = ar
INSTALL = install

# cross utils
TARGET=

export CC LD AR INSTALL TARGET

INC = -I$(TOPDIR)/include
CFLAGS = -Wall -Wno-unused-function -g -O0 $(INC) -DDEBUG
LDFLAGS = -lpthread
export CFLAGS LDFLAGS

all:acser apcli test

LIB = libapctl.o
LIBDIR = $(TOPDIR)/lib
export LIB LIBDIR

ACDIR = $(TOPDIR)/ac
acser: 
	@$(MAKE) -C $(ACDIR)

APDIR = $(TOPDIR)/ap
apcli:
	@$(MAKE) -C $(APDIR)

TESTDIR = $(TOPDIR)/test
test: FORCE
	@$(MAKE) -C $(TESTDIR)

tags: FORCE
	@find  . -name "*.h" -o -name "*.c" -o -name "*.s" > cscope.files
	@cscope -bkq -i cscope.files
	@ctags -L cscope.files

clean:
	@rm -rf `find . -name "*.o"` $(ACDIR)/acser $(APDIR)/apctl
	@rm -rf `find . -name "*.so"` 
	@rm -rf $(TESTDIR)/dllser $(TESTDIR)/dllcli
FORCE:          
	                
PHONY += deps clean FORCE
.PHONY: $(PHONY)
