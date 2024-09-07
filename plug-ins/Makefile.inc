
AM_CPPFLAGS =  -Wall
AM_LDFLAGS = -Wl,--no-undefined

CXXLD = $(CXX)

LIBS = $(top_builddir)/src/libdreamland_impl.la $(top_builddir)/src/libdreamland.la -lm 


data_DATA = $(lib_LTLIBRARIES:.la=.xml)

libdir=$(libexecdir)/plugins
datadir=$(libexecdir)/plugins

.la.xml:
	@echo "Generating $@ from $<"
	@echo '<?xml version="1.0" encoding="KOI8-U"?>' > $@
	@echo '<so>' >> $@
	@echo '    <depends>' >> $@
	@. $<; for i in $$dependency_libs; do \
	    case $$i in \
		*/libdreamland_impl.la) ;;\
		*/libdreamland.la) ;;\
		*/libstdc++.la) ;;\
		/usr/lib/*) ;;\
		/usr/local/lib/*) ;;\
		*.la) basename $$i | sed -e 's!^lib!        <node>!' \
					 -e 's!\.la$$!</node>!' >> $@ ;; \
	    esac; \
	done
	@echo '    </depends>' >> $@
	@echo '</so>' >> $@

INCLUDES_ANATOLIA= \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/alignment \
-I$(top_srcdir)/plug-ins/command \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/fight_core \
-I$(top_srcdir)/plug-ins/wearlocation \
-I$(top_srcdir)/plug-ins/race \
-I$(top_srcdir)/plug-ins/movement \
-I$(top_srcdir)/plug-ins/occupations \
-I$(top_srcdir)/plug-ins/anatolia/core \
-I$(top_srcdir)/plug-ins/anatolia

INCLUDES_FIGHT = \
$(INCLUDES_ANATOLIA) \
-I$(top_srcdir)/plug-ins/profession \
-I$(top_srcdir)/plug-ins/fight

INCLUDES_AI = \
$(INCLUDES_FIGHT) \
-I$(top_srcdir)/plug-ins/occupations \
-I$(top_srcdir)/plug-ins/traverse \
-I$(top_srcdir)/plug-ins/ai

LIBADD_ANATOLIA_CORE = \
$(top_builddir)/plug-ins/system/libsystem.la \
$(top_builddir)/plug-ins/system/libscheduler_task_round.la \
$(top_builddir)/plug-ins/command/libcommand.la \
$(top_builddir)/plug-ins/descriptor/libdescriptor.la \
$(top_builddir)/plug-ins/output/liboutput.la \
$(top_builddir)/plug-ins/interpret/libinterpret.la \
$(top_builddir)/plug-ins/loadsave/libloadsave.la \
$(top_builddir)/plug-ins/iomanager/libiomanager.la \
$(top_builddir)/plug-ins/movement/libmovement.la \
$(top_builddir)/plug-ins/wearlocation/libwearlocation.la \
$(top_builddir)/plug-ins/fight_core/libfight_core.la \
$(top_builddir)/plug-ins/anatolia/core/libanatoliacore.la 

LIBADD_ANATOLIA = \
$(LIBADD_ANATOLIA_CORE) \
$(top_builddir)/plug-ins/command/libcommand.la \
$(top_builddir)/plug-ins/help/libhelp.la \
$(top_builddir)/plug-ins/anatolia/libanatolia.la \
$(top_builddir)/plug-ins/occupations/liboccupations.la \
$(top_builddir)/plug-ins/race/librace.la 

LIBADD_FIGHT = \
$(LIBADD_ANATOLIA) \
$(top_builddir)/plug-ins/profession/libprofession.la \
$(top_builddir)/plug-ins/fight/libfight.la 

LIBADD_AI = \
$(LIBADD_FIGHT) \
$(top_builddir)/plug-ins/traverse/libtraverse.la \
$(top_builddir)/plug-ins/ai/libai.la 

