#!/bin/sh
ls /etc/runlevels/$1/ | sed 's,^,/etc/init.d/,g' | xargs ./build-file-list.sh 
#>readahead.runlevel-$1.list
