ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS       = -Wall -Werror -Wextra

pkgscriptdir = $(pkglibdir)
tcldir       = $(datarootdir)/tcltk
pkgtcldir    = $(tcldir)/$(PACKAGE)$(PACKAGE_VERSION)

pkgtcl_DATA = src/pkgIndex.tcl

pkglib_LTLIBRARIES      = src/ssh/ssh.la
src_ssh_ssh_la_CPPFLAGS = $(TCL_INCLUDES)
src_ssh_ssh_la_LDFLAGS  = -module $(TCL_STUB_LIB_SPEC) -lssh
src_ssh_ssh_la_SOURCES  = src/ssh/bind.c     \
                          src/ssh/metadata.c \
                          src/ssh/session.c  \
                          src/ssh/ssh.c
