# Copyright (C) 2004 Robin Hugh Johnson <robbat2@orbis-terrarum.net>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

man_MANS = filelist-order.1 readahead-list.8
EXTRA_DIST = filelist-order.1.in readahead-list.8.in
CLEANFILES = $(man_MANS)
SUFFIXES = .in

.in:
	@RM@ -f $@ $@-t
	@SED@ \
	  -e 's!@''PACKAGE_VERSION''@!@PACKAGE_VERSION@!' \
	  -e 's!@''sysconfdir''@!@sysconfdir@!' \
	  -e 's!@''bindir''@!$(bindir)!g' \
	  -e 's!@''libexecdir''@!$(libexecdir)!g' \
	  -e 's!@''localstatedir''@!$(localstatedir)!g' \
	$< > $@-t
	@MV@ $@-t $@

