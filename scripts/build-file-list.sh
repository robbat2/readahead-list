#!/bin/bash
source set-math.sh

EXCLUDE_REGEX='^/(sys|dev|proc|var/run|var/log)|\.keep$|,v$'

listbin_script() {
	base="$(echo $1 | sed -n -e '1s,^#!, ,gp' | awk '{print $1}')"
	#echo listbin_script $* $base 1>&2
	#list="$( cat $1 | perl -e 's/#(.*)//g' | perl -p -e "s/'(.*?)'//gs;s/\"(.*?)\"//gs" | tr -s '">&()=*{}$' ' ' | xargs -n1  |egrep -v "${EXCLUDE_REGEX}" | sort | uniq)"
	list="${base} $( cat $1 | perl -p -e 's/#(.*)//g' | tr -s "\n'" ' ' |  tr -s '">&()=*{}$' ' ' | xargs -n1  |egrep -v "${EXCLUDE_REGEX}" | sort | uniq)"
	#echo "LIST: $list" 1>&2
	for li in ${list}; do
		n="$(listbin_valid "$li")"
		[ -n "$n" ] && echo $n
	done
	#sed -e 's,#.*,,g' -e 's,".*\?",,g' -e "s,'.*\?',,g" -e 'y,()=,   ,'  <$1
	#| grep '/' #| xargs -n1 #| egrep '/' |egrep -v "${EXCLUDE_REGEX}" | sort | uniq
}
listbin_single() {
	#echo listbin_single $* 1>&2
	f="$1"
	list=""
	f="$(listbin_valid "$f")"
	if [ -n "$f" ]; then
		list="$f"
		# is this a script of some sort
		if [ "$(head -n1 $f 2>/dev/null| cut -c1-2)" == "#!" ]; then
			list="$list $(listbin_script $f)"
		else
		# nope, not a script, ldd time
		# this ensures we get the libraries we need
			list="$list $(ldd "$f" 2>/dev/null | egrep -v linux-gate.so.1 | xargs -n1 | grep '^/')"
		fi
	fi
	list="$(echo ${list} | xargs -n1 | sort | uniq)"
	echo "${list}"
}
listbin_valid() {
	#echo listbin_valid $* 1>&2
	[ -z "$1" ] && return
	f="$(echo "/$1" | tr -s '/')"
	valid=0
	if [ -f "$f" -a ! -d "$f" -a ! -b "$f" -a ! -c "$f" ]; then
		valid=1
		a=$f
	else
		f="$(which --skip-alias --skip-functions --skip-tilde -- $1 2>/dev/null)"
		if [ -f "$f" -a ! -d "$f" -a ! -b "$f" -a ! -c "$f" ]; then
			valid=1
			a=$f
		fi
	fi
	[ $valid -eq 1 ] && echo $a
}

listbin() {
	#echo listbin $* 1>&2
	list=""
	for i in $* ; do
		list="$(listbin_single "$i")" 
		list="$(echo ${list} | xargs -n1 | sort | uniq)"
	done 
	echo "$list"
}
listbin_recursive_worker() {
	#echo listbin_recursive_worker $2 1>&2
	curr="${1}"
	me="${2}"
	# base case, check for self valid and self not in list
	if [ -n "$(listbin_valid "${me}")" -a -z "$(set_intersection "${curr}" "${me}" )" ]; then
		echo "DOING: ${me}" 1>&2
		# otherwise we do recursion
		# add self to list to avoid recursion
		curr="${curr} $me"
		# grab everything it calls
		new="$(listbin $me)"
		# get new items only
		#new="$(set_complement "${curr}" "${new}")"
		new="$(set_complement "${new}" "${curr}")"
		# recurse into new items
		for i in $new; do
			#echo "DOING: $i" 1>&2
			curr="$(listbin_recursive_worker "${curr}" "$i")"
			curr="$(echo ${curr} | xargs -n1 | sort | uniq)"
		done
	fi
	curr="$(echo ${curr} | xargs -n1 | sort | uniq)"
	echo "${curr}"
}
listbin_recursive() {
	echo listbin_recursive $* 1>&2
	curr=""
	for i in $*; do
		curr="$(listbin_recursive_worker "$curr" $i)"
	done
	curr="$(echo ${curr} | xargs -n1 | sort | uniq)"
	echo "${curr}"
}

#res="$(listbin_recursive /sbin/rc)"
#echo RESULTS:
[ -n "$*" ] && res="$(listbin_recursive $*)"
echo "$res" | xargs -n1

# vim: ts=4 sw=4:
