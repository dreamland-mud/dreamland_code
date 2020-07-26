INCLUDES_SRC = \
-I$(srcdir) \
-I$(top_builddir)/src \
-I$(top_builddir) \
-I$(top_srcdir)/include \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/regex \
-I$(top_srcdir)/src/lang \
-I$(top_srcdir)/src/util \
-I$(top_srcdir)/src/xml \
-I$(top_srcdir)/src/plugin \
-I$(top_srcdir)/src/scheduler \
-I$(top_srcdir)/src/io \
-I$(top_srcdir)/src/core \
-I$(top_srcdir)/src/fenia \
-I$(top_srcdir)/src/l10n \
-I$(top_srcdir)/src/alloc \
-I$(top_srcdir)/src/flags \
-I$(top_srcdir)/src/gref \
-I$(top_srcdir)/src/process \
-I$(top_srcdir)/src/core/behavior \
-I$(top_srcdir)/src/core/scheduler \
-I$(top_srcdir)/src/core/clan \
-I$(top_srcdir)/src/core/fenia \
-I$(top_srcdir)/src/core/skills 

MAKEBITS = $(top_srcdir)/src/makebits.pl

.conf.cpp:
	$(MAKEBITS) -s $< -r -f $@

.conf.h:
	$(MAKEBITS) -s $< -h -r -f $(top_builddir)/src/$@

