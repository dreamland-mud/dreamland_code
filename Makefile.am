SUBDIRS = src plug-ins 

AUTOMAKE_OPTIONS = foreign 1.4
ACLOCAL_AMFLAGS = -I admin

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in > acinclude.m4

changelog:
	@cd $(top_srcdir) && perl admin/cvs2cl.pl -g -z9

clean:
	@rm -f *.m4 *.in
	@rm -f configure *~ 
	@rm -rf autom4te.cache
        	
