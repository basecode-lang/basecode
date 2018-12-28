#!/usr/bin/env bash
CPUPROFILE=$1.prof DYLD_INSERT_LIBRARIES=/usr/local/lib/libtcmalloc_and_profiler.dylib ../bin/bc $@
pprof --pdf ../bin/bc $1.prof > $1.pdf