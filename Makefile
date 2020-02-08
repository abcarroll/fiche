# for debug add -g -O0 to line below
#CC=clang
prefix=/usr/local/bin
CFLAGS+=-g -pthread -O2 -Wall -Wextra -Wpedantic -Wstrict-overflow -fno-strict-aliasing -std=gnu11 -g -O0
SYSTEMD_PATH=/lib/systemd/system/

all:
	mkdir -p dist/
	${CC} main.c fiche.c -lz $(CFLAGS) -lbsd -o dist/fiche
	${CC} cleaner.c $(CFLAGS) -o dist/cleaner

install: fiche
	install -m 0755 dist/fiche $(prefix)
	cp extras/systemd/* $(SYSTEMD_PATH)

clean:
	rm -f fiche cleaner

.PHONY: clean
