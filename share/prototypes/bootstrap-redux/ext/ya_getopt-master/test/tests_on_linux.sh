#!/bin/sh

if test "`uname`" != Linux; then
    echo This script runs only on Linux.
    exit 1
fi

run_test() {
    echo -n .
    # Collect outputs of GNU C Library getopt.
    env OPTSTRING="$1" ./getopt_test $2 2> getopt_test.out
    env OPTSTRING="$1" ./getopt_long_test $2 2> getopt_long_test.out
    env OPTSTRING="$1" ./getopt_long_only_test $2 2> getopt_long_only_test.out
    # Collect outputs of ya_getopt.
    env OPTSTRING="$1" ./ya_getopt_test $2 2> ya_getopt_test.out
    env OPTSTRING="$1" ./ya_getopt_long_test $2 2> ya_getopt_long_test.out
    env OPTSTRING="$1" ./ya_getopt_long_only_test $2 2> ya_getopt_long_only_test.out
    # Compare the output files respectively.
    if ! cmp getopt_test.out ya_getopt_test.out >/dev/null; then
	echo
	echo OPTSTRING=\'$1\' ARGS=\'$2\' ... FAILED
	diff -u getopt_test.out ya_getopt_test.out | head -20
	exit 1
    fi
    if ! cmp getopt_long_test.out ya_getopt_long_test.out >/dev/null; then
	echo
	echo OPTSTRING=\'$1\' ARGS=\'$2\' ... FAILED
	diff -u getopt_long_test.out ya_getopt_long_test.out | head -20
	exit 1
    fi
    if ! cmp getopt_long_only_test.out ya_getopt_long_only_test.out >/dev/null; then
	echo
	echo OPTSTRING=\'$1\' ARGS=\'$2\' ... FAILED
	diff -u getopt_long_only_test.out ya_getopt_long_only_test.out | head -20
	exit 1
    fi
}

run_tests() {
    for optstring in "ab:" ":ab:" "ab::" ":ab::" "+ab:" "+:ab:" ":+ab:" "-ab:" "-+ab:" "+-ab:" "-:ab:" ":-ab:"; do
	for args in "" \
	    "foo" \
	    "-" \
	    "-c" \
	    "-b foo" \
	    "-abfoo" \
	    "-a -b foo bar" \
	    "-b" \
	    "-acb" \
	    "-abc" \
	    "--foo" \
	    "--bar" \
	    "--baz" \
	    "--qux" \
	    "--quux" \
	    "--foo val" \
	    "--bar val" \
	    "--baz val" \
	    "--qux val" \
	    "--quux val" \
	    "--foo=val" \
	    "--bar=val" \
	    "--baz=val" \
	    "--qux=val" \
	    "--quux=val" \
	    "-foo" \
	    "-bar" \
	    "-baz" \
	    "-qux" \
	    "-quux" \
	    "-foo val" \
	    "-bar val" \
	    "-baz val" \
	    "-qux val" \
	    "-quux val" \
	    "-foo=val" \
	    "-bar=val" \
	    "-baz=val" \
	    "-qux=val" \
	    "-quux=val" \
	    "-afoo" \
	    "-abar" \
	    "-abaz" \
	    "-aquux" \
	    "-afoo val" \
	    "-abar val" \
	    "-abaz val" \
	    "-aquux val" \
	    "-afoo=val" \
	    "-abar=val" \
	    "-abaz=val" \
	    "-aquux=val" \
	    "-a foo -b bar" \
	    "-a -- -b bar" \
	    "-a --- -b bar" \
	    "-a foo bar baz -a" \
	    "-a foo bar baz -aa" \
	    "-a foo bar baz -a qux" \
	    "-a foo bar baz -a qux quux" \
	    "-a foo bar baz -a qux quux -a" \
	    "-b arg foo bar baz -b arg" \
	    "-b arg foo bar baz -b arg qux" \
	    "-b arg foo bar baz -b arg qux quux" \
	    "-barg foo bar baz -barg" \
	    "-barg foo bar baz -barg qux" \
	    "-barg foo bar baz -barg qux quux" \
	    "-a foo -b bar -a baz -c qux" \
	    "foo -b" \
	    "foo - -b bar" \
	    ; do
	    run_test "$optstring" "$args"
	done
    done
}

echo ===== Change optind and/or optstring while parsing arguments  =====
unset POSIXLY_CORRECT
run_test "a1" "foo -1a"
run_test "a1" "foo bar baz qux -1a"
run_test "a0" "foo -0a"
run_test "a0" "foo bar baz qux -0a"
run_test "C:0" "-C+C:0 foo -0"
run_test "C:1" "-C+C:1 foo -1"
run_test "C:0" "-C-C:0 foo -0"
run_test "C:1" "-C-C:1 foo -1"
echo

echo ===== with POSIXLY_CORRECT =====
POSIXLY_CORRECT=
export POSIXLY_CORRECT
run_tests
echo

echo ===== without POSIXLY_CORRECT =====
unset POSIXLY_CORRECT
run_tests
echo

echo PASS ALL
