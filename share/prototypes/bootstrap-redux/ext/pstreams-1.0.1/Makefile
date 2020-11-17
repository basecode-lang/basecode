# PStreams Makefile

#        Copyright (C) 2001 - 2017 Jonathan Wakely
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#

# TODO configure script (allow doxygenating of EVISCERATE functions)

OPTIM= -O1 -g3
EXTRA_CXXFLAGS=

CFLAGS=-pedantic -Wall -Wextra -Wpointer-arith -Wcast-qual -Wcast-align -Wredundant-decls -Wshadow $(OPTIM)
CXXFLAGS=$(CFLAGS) -std=c++98 -Woverloaded-virtual

prefix = /usr/local
includedir = $(prefix)/include
INSTALL = install
INSTALL_DATA = $(INSTALL) -p -v -m 0644

SOURCES = pstream.h
GENERATED_FILES = ChangeLog MANIFEST
EXTRA_FILES = AUTHORS LICENSE_1_0.txt Doxyfile INSTALL Makefile README \
	    mainpage.html test_pstreams.cc test_minimum.cc pstreams-devel.spec

DIST_FILES = $(SOURCES) $(GENERATED_FILES) $(EXTRA_FILES)

VERS := $(shell awk -F' ' '/^\#define *PSTREAMS_VERSION/{ print $$NF }' pstream.h)

all: check-werror

check-werror:
	@rm -f test_pstreams test_minimum
	@$(MAKE) check EXTRA_CXXFLAGS=-Werror

check: test_pstreams test_minimum | pstreams.wout
	@for test in $^ ; do echo $$test ; ./$$test >/dev/null 2>&1 || echo "$$test EXITED WITH STATUS $$?" ; done

test run_tests: check

test_%: test_%.cc pstream.h
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(LDFLAGS) -o $@ $<

MANIFEST: Makefile
	@for i in $(DIST_FILES) ; do echo "pstreams-$(VERS)/$$i" ; done > $@

docs: pstream.h mainpage.html
	@doxygen Doxyfile

mainpage.html: pstream.h Makefile
	@perl -pi -e "s/^(<p>Version) [0-9\.]*(<\/p>)/\1 $(VERS)\2/" $@

ChangeLog:
	@if [ -d .git ]; then git log --no-merges | grep -v '^commit ' > $@ ; fi

dist: pstreams-$(VERS).tar.gz pstreams-docs-$(VERS).tar.gz

srpm: pstreams-$(VERS).tar.gz
	@rpmbuild -ts $<

pstreams-$(VERS):
	@ln -s . $@

pstreams-$(VERS).tar.gz: pstream.h $(GENERATED_FILES) pstreams-$(VERS)
	@tar -czvf $@ `cat MANIFEST`

pstreams-docs-$(VERS):
	@ln -s doc/html $@

pstreams-docs-$(VERS).tar.gz: docs pstreams-docs-$(VERS)
	@tar -czvhf $@ pstreams-docs-$(VERS)

TODO : pstream.h mainpage.html test_pstreams.cc
	@grep -nH TODO $^ | sed -e 's@ *// *@@' > $@

clean:
	@rm -f  test_minimum test_pstreams
	@rm -rf doc TODO $(GENERATED_FILES)
	@rm -f  *.tar.gz pstreams-$(VERS) pstreams-docs-$(VERS)

install:
	@install -d $(DESTDIR)$(includedir)/pstreams
	@$(INSTALL_DATA) pstream.h $(DESTDIR)$(includedir)/pstreams/pstream.h

pstreams.wout:
	@echo "Wide Load" | iconv -f ascii -t UTF-32 > $@

.PHONY: TODO check test run_tests dist srpm
.INTERMEDIATE: $(GENERATED_FILES) pstreams-$(VERS) pstreams-docs-$(VERS)

