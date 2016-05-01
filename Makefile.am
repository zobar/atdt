ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS       = -Wall -Werror -Wextra

pkglibdir = $(libdir)/@PACKAGE@@VERSION@
pkgtcldir = $(pkglibdir)

pkglib_LTLIBRARIES = src/ssh/ssh.la
pkgtcl_DATA        = src/pkgIndex.tcl

src_ssh_ssh_la_CPPFLAGS = $(TCL_INCLUDES)
src_ssh_ssh_la_LDFLAGS  = -module $(TCL_STUB_LIB_SPEC) -lssh
src_ssh_ssh_la_SOURCES  = src/ssh/bind.c     \
                          src/ssh/event.c    \
                          src/ssh/metadata.c \
                          src/ssh/oo.c       \
                          src/ssh/session.c  \
                          src/ssh/ssh.c      \
                          src/ssh/thread.c
