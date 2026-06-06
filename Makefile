CC=clang
INC=./include/
OPTFLAGS = -g
WARNFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror

ASANFLAGS  = -fsanitize=address,undefined
ASANFLAGS += -fno-common
ASANFLAGS += -fno-omit-frame-pointer

default: test

.PHONY: test
test: clean
	@if [ ! -d bin ]; then mkdir bin; fi
	@$(CC) $(OPTFLAGS) $(WARNFLAGS) $(CFLAGS) -I$(INC) test/*.c test/vendor/*.c src/*.c -o bin/test
	@./bin/test

.PHONY: clean
clean:
	@if [ -f bin/test ] ; then rm bin/test; fi
	@if [ -f bin/memcheck ] ; then rm bin/memcheck; fi

.PHONY: memcheck
memcheck: test/*.c src/*.c
	@mkdir -p ./bin
	@$(CC) $(ASANFLAGS) $(OPTFLAGS) $(WARNFLAGS) $(CFLAGS) -I$(INC) test/*.c test/vendor/*.c src/*.c -o bin/memcheck $(LIBS)
	@./bin/memcheck
	@echo "Memory check passed"

.PHONY: format
format:
	@clang-format -style=file -i src/* include/*
