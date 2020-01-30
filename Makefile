ALL_OBJ != find src -type f -name '*.o'
ALL_SRC != find src include -type f -name '*.h' -or -name '*.c'
ALL_C != find src -type f -name '*.c'
ALL_STUB != find stub/enabled -name '*.makefile'

export TMP_PREFIX ?=  prefix

export PREFIX ?= /usr/local

export CFLAGS += -Iinclude

.PHONY: $(ALL_STUB) all install release debug build

all: release

release:
	CFLAGS="-DNDEBUG $(CFLAGS)" $(MAKE) build

debug:
	CFLAGS="-g $(CFLAGS)" $(MAKE) build

install:
	TMP_PREFIX="$(PREFIX)" $(MAKE) release

build: $(ALL_STUB)

enable-program:
	(cd stub && sh enable.sh)

create-program:
	(cd stub && sh create.sh)

enable-test:
	(cd test-scripts && sh enable.sh)

create-test:
	(cd test-scripts && sh create.sh)

run-tests:
	(cd test-scripts && sh exec.sh)

help:
	@echo make all - build all programs
	@echo make help - show this message
	@echo make clean - remove intermediate build files
	@echo create-program - create a program recipe
	@echo enable-program - enable a program recipe
	@echo create-test - create a test script
	@echo enable-test - enable a test script

$(ALL_STUB): make/depend.makefile
	STUB=$(@) make -f make/build.makefile

make/depend.makefile: $(ALL_SRC) 
	touch make/depend.makefile
	makedepend -f make/depend.makefile -- $(CFLAGS) -- $(ALL_C)

clean:
	rm -f make/depend.makefile make/depend.makefile.bak $(ALL_OBJ)
# DO NOT DELETE
