#!/bin/sh
ls /etc/runlevels/$1/ | sed 's,^,/etc/init.d/,g' | xargs ./builder.sh >readahead.runlevel-$1.list
