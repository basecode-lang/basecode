PROGS = getopt_test getopt_long_test getopt_long_only_test ya_getopt_test ya_getopt_long_test ya_getopt_long_only_test
CFLAGS = -Wall

all: $(PROGS)

check: $(PROGS)
	@./tests_on_linux.sh

getopt_test: getopt_test.c
	$(CC) $(CFLAGS) -o getopt_test getopt_test.c

getopt_long_test: getopt_long_test.c
	$(CC) $(CFLAGS) -o getopt_long_test getopt_long_test.c

getopt_long_only_test: getopt_long_only_test.c
	$(CC) $(CFLAGS) -o getopt_long_only_test getopt_long_only_test.c

ya_getopt_test: getopt_test.c ../ya_getopt.c ../ya_getopt.h
	$(CC) $(CFLAGS) -o ya_getopt_test -DUSE_YA_GETOPT -I.. getopt_test.c  ../ya_getopt.c

ya_getopt_long_test: getopt_long_test.c ../ya_getopt.c ../ya_getopt.h
	$(CC) $(CFLAGS) -o ya_getopt_long_test -DUSE_YA_GETOPT -I.. getopt_long_test.c  ../ya_getopt.c

ya_getopt_long_only_test: getopt_long_only_test.c ../ya_getopt.c ../ya_getopt.h
	$(CC) $(CFLAGS) -o ya_getopt_long_only_test -DUSE_YA_GETOPT -I.. getopt_long_only_test.c  ../ya_getopt.c

clean:
	$(RM) $(PROGS) getopt_test.out getopt_long_test.out getopt_long_only_test.out ya_getopt_test.out ya_getopt_long_test.out ya_getopt_long_only_test.out
