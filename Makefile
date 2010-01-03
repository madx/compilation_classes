CC = gcc
CFLAGS = -Wall -ansi -pedantic -g
ifeq (${DEBUG},1)
	CFLAGS += -g
endif

SRC = src/ast.c src/lexer.c src/parser.c src/symbol.c src/compile.c src/vm.c

OBJ = ${SRC:src/%.c=obj/%.o}

REV = `git log --pretty=format:%h -n1`

.PHONY: all clean dist

all: alvc

rev: alvc
	@cp bin/alvc bin/alvc-$(REV)
	@echo created alvc-$(REV)

alvc: ${OBJ}
	@mkdir -p bin/
	@echo CC src/alvc.c
	$(CC) $(CFLAGS) -o bin/alvc $^ src/alvc.c
	@echo

test_lexer: ${OBJ}
	@mkdir -p bin/
	@echo CC test/lexer.c
	$(CC) $(CFLAGS) -Isrc/ -o bin/test_lexer $^ test/lexer.c

test_parser: ${OBJ}
	@mkdir -p bin/
	@echo CC test/parser.c
	$(CC) $(CFLAGS) -Isrc/ -o bin/test_parser $^ test/parser.c

clear:
	rm -rf obj/ bin/
clean:
	rm -rf obj/

obj/%.o: src/%.c
	@mkdir -p obj/
	@echo CC $< "->" $@
	- $(CC) -c $(CFLAGS) -o $@ $<
	@echo
