DESTDIR ?=
PREFIX  ?= /usr/local

CFLAGS += -fPIC -Wall -Wextra
LDLIBS += -ljson-c

lib_OBJ  = mustach.o mustach-json-c.o
tool_OBJ = mustach.o mustach-json-c.o mustach-tool.o
HEADERS  = mustach.h mustach-json-c.h

all: mustach libmustach.so

install: mustach libmustach.so
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/mustach
	install -m0755 mustach       $(DESTDIR)$(PREFIX)/bin/
	install -m0755 libmustach.so $(DESTDIR)$(PREFIX)/lib/
	install -m0644 $(HEADERS)    $(DESTDIR)$(PREFIX)/include/mustach

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/mustach
	rm -f $(DESTDIR)$(PREFIX)/lib/libmustach.so
	rm -rf $(DESTDIR)$(PREFIX)/include/mustach

mustach: $(tool_OBJ)
	$(CC) $(LDFLAGS) -o mustach $(tool_OBJ) $(LDLIBS)

libmustach.so: $(lib_OBJ)
	$(CC) $(LDFLAGS) -shared -o libmustach.so $(lib_OBJ) $(LDLIBS)

mustach.o:      mustach.h
mustach-json.o: mustach.h mustach-json-c.h
mustach-tool.o: mustach.h mustach-json-c.h

test: mustach
	@$(MAKE) -C test1 test
	@$(MAKE) -C test2 test
	@$(MAKE) -C test3 test
	@$(MAKE) -C test4 test
	@$(MAKE) -C test5 test
	@$(MAKE) -C test6 test

clean:
	rm -f mustach libmustach.so *.o
	@$(MAKE) -C test1 clean
	@$(MAKE) -C test2 clean
	@$(MAKE) -C test3 clean
	@$(MAKE) -C test4 clean
	@$(MAKE) -C test5 clean
	@$(MAKE) -C test6 clean

.PHONY: test clean install uninstall
