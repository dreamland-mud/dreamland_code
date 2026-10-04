SUBDIRS = src plug-ins 

AUTOMAKE_OPTIONS = foreign 1.4
ACLOCAL_AMFLAGS = -I admin

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in > acinclude.m4

changelog:
	@cd $(top_srcdir) && perl admin/cvs2cl.pl -g -z9

# Wipe the generated build system: acinclude.m4, aclocal.m4, Makefile.in,
# configure and the autoconf cache. Only "make -f Makefile.git" puts them
# back, so this is a maintainer's tool, not part of a build.
#
# It used to be named "clean", which overrode automake's own clean target.
# An ordinary "make clean" at the top of the tree therefore deleted configure,
# aclocal.m4, acinclude.m4 and the top-level Makefile.in, config.h.in and
# stamp-h.in. The per-directory Makefile.in files survive -- the globs are not
# recursive -- but that does not help: without configure and config.h.in the
# tree cannot be configured at all, and since none of these files are in git,
# the next build has to regenerate the build system before it can compile
# anything. That is the state the regeneration cascade starts from.
maintainer-wipe:
	@rm -f *.m4 *.in
	@rm -f configure *~
	@rm -rf autom4te.cache

.PHONY: maintainer-wipe changelog
        	
