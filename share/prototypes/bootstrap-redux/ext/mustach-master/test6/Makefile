.PHONY: test clean

test-custom-write: test-custom-write.c ../mustach-json-c.h ../mustach-json-c.c ../mustach.h ../mustach.c
	@echo building test-custom-write
	$(CC) $(CFLAGS) -g -o test-custom-write test-custom-write.c  ../mustach.c  ../mustach-json-c.c -ljson-c

test: test-custom-write
	@echo starting test
	@valgrind ./test-custom-write json -U must -l must -x must > resu.last 2> vg.last
	@sed -i 's:^==[0-9]*== ::' vg.last
	@diff resu.ref resu.last && echo "result ok" || echo "ERROR! Result differs"
	@diff vg.ref vg.last && echo "memory ok" || echo "ERROR! Memory differs"

clean:
	rm -f resu.last vg.last test-custom-write

