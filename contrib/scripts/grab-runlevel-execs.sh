#!/bin/sh
# Copyright 2005 Robin H. Johnson <robbat2@orbis-terrarum.net>
# Distributed under the terms of the GNU General Public License v2
# $Header: /code/convert/cvsroot/infrastructure/readahead-list/contrib/scripts/grab-runlevel-execs.sh,v 1.3 2005/04/25 21:43:31 robbat2 Exp $
ls /etc/runlevels/$1/ | sed 's,^,/etc/init.d/,g' | xargs ./build-file-list.sh
#>readahead.runlevel-$1.list
