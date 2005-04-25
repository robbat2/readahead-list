#!/bin/bash
# Copyright 2005 Robin H. Johnson <robbat2@orbis-terrarum.net>
# Distributed under the terms of the GNU General Public License v2
# $Header: /code/convert/cvsroot/infrastructure/readahead-list/contrib/scripts/set-math.sh,v 1.3 2005/04/25 21:43:31 robbat2 Exp $

COMM=`which comm`

# Call me nuts, I just wrote set functions in shell!
# -Robin H. Johnson <robbat2@orbis-terrarum.net>
set_helper_comm() {
	opt="$1"
	a="$2"
	b="$3"
	tmpbase="$(mktemp)"
	tmp_a=${tmpbase}.a
	tmp_b=${tmpbase}.b
	echo "$a" | xargs -n1 | sort | uniq >$tmp_a
	echo "$b" | xargs -n1 | sort | uniq >$tmp_b
	#echo "comm ${opt} ${tmp_a} ${tmp_b} | xargs" 1>&2
	t="$(${COMM} ${opt} ${tmp_a} ${tmp_b} | xargs)"
	rm -f ${tmpbase}*
	echo $t
}

set_union() {
	a="$1"
	b="$2"
	set_helper_comm '' "$a" "$b"
}

set_intersection() {
	a="$1"
	b="$2"
	set_helper_comm '-12' "$a" "$b"
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
		t="$(set_helper_comm '-13' "$a" "$b")"
	fi
	echo "$t"
}

set_test() {
	a="a b c d e"
	b="d e f g h"
	union="$(set_union "$a" "$b")"
	intersection="$(set_intersection "$a" "$b")"
	complement="$(set_complement "$a" "$b")"
	echo "Union: ${union}"
	echo "Intersection: ${intersection}"
	echo "Complement: ${complement}"
}
