#!/bin/bash

GREP=/bin/grep

# Call me nuts, I just wrote set functions in shell!
# -Robin H. Johnson <robbat2@gentoo.org>
set_union() {
	a="$1"
	b="$2"
	t="$a"
	for i in $b; do
		echo "${a}" | ${GREP} -Fwqs "${i}"
		# if not there, add
		[ "$?" -ne 0 ] && t="${t} ${i}"
	done
	echo "$t"
}

set_intersection() {
	a="$1"
	b="$2"
	t=''
	if [ -z "$a" -o -z "$b" ]; then
		t=''
	else
		for i in $a; do
			echo "${b}" | ${GREP} -Fwqs "${i}"
			# if there, add
			[ "$?" -eq 0 ] && t="${t} ${i}"
		done
	fi
	echo "$t"
}
set_complement() {
	a="$1"
	b="$2"
	t=''
	# check for special cases
	if [ -z "$a" ]; then
		t="$b"
	elif [ -z "$b" ]; then
		t=''
	# base case
	else
		for i in $a; do
			echo "${b}" | ${GREP} -Fwqs "${i}"
			# if not there, add
			[ "$?" -ne 0 ] && t="${t} ${i}"
		done
	fi
	echo "$t"
}
