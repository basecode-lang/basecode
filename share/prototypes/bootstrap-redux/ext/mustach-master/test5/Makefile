.PHONY: test clean

test: ../mustach
	@echo starting test
	@valgrind ../mustach json must > resu.last 2> vg.last
	@sed -i 's:^==[0-9]*== ::' vg.last
	@diff resu.ref resu.last && echo "result ok" || echo "ERROR! Result differs"
	@diff vg.ref vg.last && echo "memory ok" || echo "ERROR! Memory differs"

clean:
	rm -f resu.last vg.last

