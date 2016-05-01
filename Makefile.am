ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS       = -Wall -Werror -Wextra
AM_CPPFLAGS     = -I$(top_builddir)/lib -I$(top_srcdir)/lib
EXTRA_DIST      = m4/gnulib-cache.m4
LDADD           = lib/libgnu.la
SUBDIRS         = lib

pkglibdir = $(libdir)/@PACKAGE@@VERSION@
pkgtcldir = $(pkglibdir)

pkglib_LTLIBRARIES = src/ssh/ssh.la
pkgtcl_DATA        = src/pkgIndex.tcl

src_ssh_ssh_la_CPPFLAGS = $(AM_CPPFLAGS) $(TCL_INCLUDES)
src_ssh_ssh_la_LDFLAGS  = -module $(TCL_STUB_LIB_SPEC) -lssh $(LDADD)
src_ssh_ssh_la_SOURCES  = src/ssh/bind.c     \
                          src/ssh/event.c    \
                          src/ssh/metadata.c \
                          src/ssh/oo.c       \
                          src/ssh/session.c  \
                          src/ssh/ssh.c      \
                          src/ssh/thread.c
