#!/usr/bin/env bash

exit_if_error() {
	local exit_code=$1
	shift
	[[ $exit_code ]] && ((exit_code !=0)) && { 
		printf 'ERROR: %s\n' "$@"	  
		exit "$exit_code"		  
	}
}

usage() {
	printf "usage: foundation-tool [-b|--branch=branch-name] [-u|--url=url]\n"
}

NEW_URL=""
NEW_BRANCH=""

CUR_URL=`git config --file .gitmodules --get submodule.ext/foundation.url`
CUR_BRANCH=`git config --file .gitmodules --get submodule.ext/foundation.branch`

for i in "$@"
do
	case $i in
		-b=*|--branch=*)
			NEW_BRANCH="${i#*=}"
			shift
			;;
		-u=*|--url=*)
			NEW_URL="${i#*=}"
			shift
			;;
		*)
			printf "unexpected option: ${i}\n"
			usage
			exit 1
			;;
	esac
done

printf "Basecode Foundation Library:\n"
printf "Current URL    = ${CUR_URL}\n"
printf "        Branch = ${CUR_BRANCH}\n"

if [[ -z ${NEW_URL} ]] && [[ -z ${NEW_BRANCH} ]]; then
	printf "\n"
	usage
	exit 1
fi

printf "New     " 

if [[ ! -z ${NEW_URL} ]]; then
	printf "URL    = ${NEW_URL}\n"
	git submodule set-url -- ext/foundation ${NEW_URL} || exit_if_error $? "git submodule set-url failed"
fi

if [[ ! -z ${NEW_BRANCH} ]]; then
	printf "Branch = ${NEW_BRANCH}\n"
	git submodule set-branch --branch ${NEW_BRANCH} -- ext/foundation || exit_if_error $? "git submodule set-branch failed"
	git submodule update --force --remote || exit_if_error $? "git submodule update --force --remote failed"
fi

