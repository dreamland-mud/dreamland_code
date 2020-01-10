dnl Process this file with autoconf to produce a configure script.

AC_INIT([dreamland], [0.1])
AC_CONFIG_SRCDIR(Makefile.am)
AC_CONFIG_AUX_DIR(admin)
AC_CONFIG_MACRO_DIRS([admin])
AC_CANONICAL_SYSTEM
AM_CONFIG_HEADER(config.h)
AC_PREFIX_DEFAULT(`pwd`)
AM_INIT_AUTOMAKE

dnl Check for additional compiler/linker options
AC_CHECK_OPTIMIZATION
   
AM_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

dnl Checks for programs.
AC_EXEEXT
AC_PROG_CC
AC_PROG_CXX

AC_LANG_CPLUSPLUS

AM_PROG_LEX
AC_PROG_YACC
AC_PROG_AWK
AC_PROG_RANLIB

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS( unistd.h )
AC_CHECK_HEADERS( netinet/in.h arpa/inet.h sys/socket.h semaphore.h )
AC_CHECK_HEADERS( dlfcn.h sys/types.h )


dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_DLSIZE_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T

AC_CHECK_TIME_T

dnl Checks for library functions.
AC_CHECK_BDB

AC_CHECK_MINGW

AC_FUNC_CLOSEDIR_VOID
AC_FAKE_CLOSEDIR_VOID

AC_CHECK_FUNCS( socket )
AC_CHECK_FUNCS( accept )

dnl Checks for libraries.
AC_CHECK_LIB(m, main)
AC_CHECK_LIB(dl, dlopen)

AC_CHECK_THREADS
AC_CHECK_MCCP
AC_CHECK_GC
AC_CONVERT_CHARSET
#AM_ICONV_LINK

dnl Additional options
AC_CHECK_FIGHT_STUB
AC_CHECK_AI_STUB

dnl Creating installation tree
test "x$prefix" = xNONE && prefix=$ac_default_prefix
AC_CREATE_DIR(bin)
AC_CREATE_DIR(lib)
AC_CREATE_DIR(libexec)
AC_CREATE_DIR(etc)
AC_CREATE_DIR(share)

AC_CREATE_DIR(var/run)
AC_CREATE_DIR(var/log)
AC_CREATE_DIR(var/misc)
AC_CREATE_DIR(var/misc/typos)
AC_CREATE_DIR(var/misc/bugs)
AC_CREATE_DIR(var/misc/ideas)
AC_CREATE_DIR(var/misc/nohelp)
AC_CREATE_DIR(var/misc/discord)
AC_CREATE_DIR(var/misc/api)
AC_CREATE_DIR(var/misc/help)

AC_CREATE_DIR(var/db/fenia)
AC_CREATE_DIR(var/db/clan-data)
AC_CREATE_DIR(var/db/gquests)

AC_CREATE_DIR(var/db/languages)
cp "$srcdir/admin/scripts/ahenn.txt"    "$prefix/var/db/languages/ahenn.txt"

AC_CREATE_DIR(var/db/notes/change)
AC_CREATE_DIR(var/db/notes/crime)
AC_CREATE_DIR(var/db/notes/dream)
AC_CREATE_DIR(var/db/notes/idea)
AC_CREATE_DIR(var/db/notes/news)
AC_CREATE_DIR(var/db/notes/note)
AC_CREATE_DIR(var/db/notes/penalty)
AC_CREATE_DIR(var/db/notes/qnote)
AC_CREATE_DIR(var/db/notes/story)

AC_CREATE_DIR(var/db/player/backup)
AC_CREATE_DIR(var/db/backup)
AC_CREATE_DIR(var/db/remorts)
AC_CREATE_DIR(var/db/delete)
AC_CREATE_DIR(var/db/oldstyle/player)
AC_CREATE_DIR(var/db/oldstyle/remorts)
AC_CREATE_DIR(var/db/oldstyle/delete)
AC_CREATE_DIR(var/db/oldstyle/backup)

AC_CREATE_DIR_16(var/db/saved/charmed)
AC_CREATE_DIR_16(var/db/saved/objects)
AC_CREATE_DIR_16(var/db/saved/mobiles)

AC_CREATE_FILE(etc/dreamland.xml)
AC_CREATE_FILE(libexec/plugin.xml)
AC_PLAYER_COPY(kadm)
AC_PLAYER_COPY(cadmus)
cp "$srcdir/admin/startup-local"    "$prefix/bin/startup-local"

if ! test -f "$prefix/var/db/fenia/fenia" 
then
    cat "$srcdir/admin/fenia/fenia.tbz2" | (cd "$prefix/var/db" && tar xvjf -)
fi

AC_CONFIG_COMMANDS([src/moc/moc], [
    AC_MSG_NOTICE([Configuring meta object compiller...])
    ac_moc_dir_pop=`pwd`
    cd $ac_dir
    ac_moc_cmd="/bin/sh $ac_srcdir/configure --build=$ac_moc_build"
    AC_MSG_NOTICE([Running command $ac_moc_cmd]);
    eval $ac_moc_cmd || exit 1
    AC_MSG_NOTICE([Building meta object compiller...])
    make || exit 1
    cd $ac_moc_dir_pop
], [ac_moc_build=$build])

AC_OUTPUT( \
Makefile \
src/Makefile \
src/core/Makefile \
src/core/behavior/Makefile \
src/core/fenia/Makefile \
src/fenia/Makefile \
src/core/scheduler/Makefile \
src/core/skills/Makefile \
src/core/clan/Makefile \
src/io/Makefile \
src/lang/Makefile \
src/moc/Makefile \
src/plugin/Makefile \
src/scheduler/Makefile \
src/util/Makefile \
src/regex/Makefile \
src/xml/Makefile \
src/l10n/Makefile \
src/alloc/Makefile \
src/flags/Makefile \
src/gref/Makefile \
src/process/Makefile \
plug-ins/Makefile \
plug-ins/interpret/Makefile \
plug-ins/descriptor/Makefile \
plug-ins/ed/Makefile \
plug-ins/ed/lib/Makefile \
plug-ins/servlets/Makefile \
plug-ins/iomanager/Makefile \
plug-ins/admin/Makefile \
plug-ins/comm/Makefile \
plug-ins/system/Makefile \
plug-ins/areas/Makefile \
plug-ins/olc/Makefile \
plug-ins/mlove/Makefile \
plug-ins/gquest/Makefile \
plug-ins/gquest/core/Makefile \
plug-ins/gquest/command/Makefile \
plug-ins/gquest/gangsters/Makefile \
plug-ins/gquest/invasion/Makefile \
plug-ins/gquest/rainbow/Makefile \
plug-ins/quest/Makefile \
plug-ins/quest/core/Makefile \
plug-ins/quest/command/Makefile \
plug-ins/quest/killquest/Makefile \
plug-ins/quest/staffquest/Makefile \
plug-ins/quest/butcherquest/Makefile \
plug-ins/quest/healquest/Makefile \
plug-ins/quest/kidnapquest/Makefile \
plug-ins/quest/stealquest/Makefile \
plug-ins/quest/locatequest/Makefile \
plug-ins/quest/bigquest/Makefile \
plug-ins/questreward/Makefile \
plug-ins/misc_behaviors/Makefile \
plug-ins/magic/Makefile \
plug-ins/race/Makefile \
plug-ins/race_aptitude/Makefile \
plug-ins/groups/Makefile \
plug-ins/learn/Makefile \
plug-ins/clan/Makefile \
plug-ins/clan/core/Makefile \
plug-ins/clan/impl/Makefile \
plug-ins/clan/skill/Makefile \
plug-ins/clan/command/Makefile \
plug-ins/clan/behavior/Makefile \
plug-ins/skills_impl/Makefile \
plug-ins/mansion/Makefile \
plug-ins/feniaroot/Makefile \
plug-ins/web/Makefile \
plug-ins/remort/Makefile \
plug-ins/anatolia/Makefile \
plug-ins/anatolia/core/Makefile \
plug-ins/note/Makefile \
plug-ins/cards/Makefile \
plug-ins/services/Makefile \
plug-ins/services/core/Makefile \
plug-ins/services/command/Makefile \
plug-ins/services/misc/Makefile \
plug-ins/services/petshop/Makefile \
plug-ins/services/shop/Makefile \
plug-ins/communication/Makefile \
plug-ins/traverse/Makefile \
plug-ins/loadsave/Makefile \
plug-ins/social/Makefile \
plug-ins/command/Makefile \
plug-ins/ai/Makefile \
plug-ins/languages/Makefile \
plug-ins/languages/core/Makefile \
plug-ins/languages/impl/Makefile \
plug-ins/movement/Makefile \
plug-ins/fight_core/Makefile \
plug-ins/profession/Makefile \
plug-ins/riding/Makefile \
plug-ins/hometown/Makefile \
plug-ins/wearlocation/Makefile \
plug-ins/selfrate/Makefile \
plug-ins/liquid/Makefile \
plug-ins/religion/Makefile \
plug-ins/desire/Makefile \
plug-ins/alignment/Makefile \
plug-ins/ban/Makefile \
plug-ins/fight/Makefile \
plug-ins/updates/Makefile \
plug-ins/occupations/Makefile \
plug-ins/output/Makefile \
plug-ins/follow/Makefile \
plug-ins/follow/core/Makefile \
plug-ins/follow/command/Makefile \
plug-ins/debug/Makefile \
plug-ins/help/Makefile \
plug-ins/druids/Makefile \
plug-ins/craft/Makefile \
plug-ins/gmcp/Makefile \
plug-ins/bonus/Makefile \
plug-ins/searcher/Makefile \
)

echo ""
echo ""

if test "$WITH_MCCP" = "no"; then
    echo "Warning: MCCP support is disabled!"
fi


