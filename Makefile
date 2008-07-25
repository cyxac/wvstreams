
.PHONY: configfile/tests
configfile/tests: $(patsubst %.cc,%,$(wildcard configfile/tests/*.cc))

TESTS+=$(patsubst %.cc,%,$(wildcard configfile/tests/*.cc))
TESTS+=$(patsubst %.cc,%,$(wildcard crypto/tests/*.cc))

crypto/tests/certtest: LDFLAGS+=-lssl
crypto/tests/cryptotest: LDFLAGS+=-lssl
crypto/tests/md5test: LDFLAGS+=-lssl
crypto/tests/reqtest: LDFLAGS+=-lssl
crypto/tests/ssltest: LDFLAGS+=-lssl
crypto/tests/sslsrvtest: LDFLAGS+=-lssl

dbus/tests: $(patsubst %.cc,%,$(wildcard dbus/tests/*.cc))

TESTS+=$(patsubst %.cc,%,$(wildcard dbus/tests/*.cc))

TARGETS+=examples/wvgrep/wvgrep examples/wvgrep/wvegrep

examples/wvgrep/wvgrep: examples/wvgrep/wvgrep.o $(LIBWVSTREAMS) $(LIBWVUTILS)

examples/wvgrep/wvegrep: examples/wvgrep/wvgrep
	ln -f $< $@

TESTS+=$(filter-out ipstreams/tests/wsd, $(patsubst %.cc,%,$(wildcard ipstreams/tests/*.cc)))
ifneq ("$(with_readline)", "no")
TESTS+=ipstreams/tests/wsd
endif

ipstreams/tests/xplctest: LIBS+=-lxplc-cxx -lxplc
ipstreams/tests/wsd: LIBS+=-lxplc-cxx -lxplc -lreadline

linuxstreams/tests: $(patsubst %.cc,%,$(wildcard linuxstreams/tests/*.cc))

TESTS+=$(patsubst %.cc,%,$(wildcard linuxstreams/tests/*.cc))

qt/wvqtstreamclone.o: include/wvqtstreamclone.moc
qt/wvqthook.o: include/wvqthook.moc

libwvqt.so-LIBS: $(LIBS_QT)

ifneq ("$(with_qt)", "no")
TESTS+=$(patsubst %.cc,%,$(wildcard qt/tests/*.cc))
endif

qt/tests/qtstringtest: libwvqt.a
qt/tests/%: LDLIBS+=libwvqt.a
qt/tests/%: LDLIBS+=-lqt-mt
# qt/tests/%: CPPFLAGS+=-I/usr/include/qt

streams/tests: $(patsubst %.cc,%,$(wildcard streams/tests/*.cc))


TESTS+=$(patsubst %.cc,%,$(wildcard streams/tests/*.cc))


CXXFLAGS+=-DWVSTREAMS_RELEASE=\"$(PACKAGE_VERSION)\"
DISTCLEAN+=uniconf/daemon/uniconfd.8

libuniconf.so libuniconf.a: \
	$(filter-out uniconf/daemon/uniconfd.o, \
	     $(call objects,uniconf/daemon))

ifeq ($(EXEEXT),.exe)
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o libwvwin32.a
else
uniconf/daemon/uniconfd: uniconf/daemon/uniconfd.o $(LIBUNICONF)
endif

%: %.in
	@sed -e "s/#VERSION#/$(PACKAGE_VERSION)/g" < $< > $@

.PHONY: uniconf/tests
uniconf/tests: $(patsubst %.cc,%,$(wildcard uniconf/tests/*.cc))

%: %.in
	@sed -e 's/#VERSION#/$(PACKAGE_VERSION)/g' < $< > $@

TESTS+=$(patsubst %.cc,%,$(wildcard uniconf/tests/*.cc)) uniconf/tests/uni

DISTCLEAN+=uniconf/tests/uni

TESTS+=$(patsubst %.cc,%,$(wildcard urlget/tests/*.cc))

urlget/tests/http2test: LDFLAGS+=-lssl -rdynamic


.PHONY: utils/tests
utils/tests: $(patsubst %.cc,%,$(wildcard utils/tests/*.cc))


TESTS+=$(patsubst %.cc,%,$(wildcard utils/tests/*.cc))

utils/tests/encodertest: LDFLAGS+=-lz

CPPFLAGS += -Iinclude -pipe
ARFLAGS = rs

DEBUG:=$(filter-out no,$(enable_debug))

# for O_LARGEFILE
CXXFLAGS+=${CXXOPTS}
CFLAGS+=${COPTS}
CXXFLAGS+=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS+=-D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

ifneq ($(DEBUG),)
CXXFLAGS+=-ggdb -DDEBUG$(if $(filter-out yes,$(DEBUG)), -DDEBUG_$(DEBUG))
CFLAGS+=-ggdb -DDEBUG$(if $(filter-out yes,$(DEBUG)), -DDEBUG_$(DEBUG))
endif

ifeq ("$(enable_debug)", "no")
#CXXFLAGS+=-fomit-frame-pointer
# -DNDEBUG is disabled because we like assert() to crash
#CXXFLAGS+=-DNDEBUG
#CFLAGS+=-DNDEBUG
endif

ifeq ("$(enable_fatal_warnings)", "yes")
CXXFLAGS+=-Werror
# FIXME: not for C, because our only C file, crypto/wvsslhack.c, has
#        a few warnings.
#CFLAGS+=-Werror
endif

ifneq ("$(enable_optimization)", "no")
CXXFLAGS+=-O2
#CXXFLAGS+=-felide-constructors
CFLAGS+=-O2
endif

ifneq ("$(enable_warnings)", "no")
CXXFLAGS+=-Wall -Woverloaded-virtual
CFLAGS+=-Wall
endif

ifeq ("$(enable_testgui)", "no")
WVTESTRUN=env
endif

ifeq ("$(enable_efence)", "yes")
LDLIBS+=-lefence
endif

ifneq ("$(with_bdb)", "no")
  libwvutils.so-LIBS+=-ldb
endif

ifneq ("$(with_qdbm)", "no")
  libwvutils.so-LIBS+=-L. -lqdbm
endif

libwvbase.so-LIBS+=-lxplc-cxx -lm
libwvbase.so:

ifneq ("$(with_openslp)", "no")
  libwvstreams.so: -lslp
endif

ifneq ("$(with_pam)", "no")
  libwvstreams.so: -lpam
endif

DEBUG:=$(filter-out no,$(enable_debug))

# debugging function
showvar = @echo \"'$(1)'\" =\> \"'$($(1))'\"
tbd = $(error "$@" not implemented yet)

# initialization
TARGETS:=
GARBAGE:=
DISTCLEAN:=
REALCLEAN:=
TESTS:=
NO_CONFIGURE_TARGETS:=

NO_CONFIGURE_TARGETS+=clean ChangeLog depend dust configure dist \
		distclean realclean

TARGETS += libwvbase.so libwvbase.a
TARGETS += libwvutils.so libwvutils.a
TARGETS += libwvstreams.so libwvstreams.a
TARGETS += libuniconf.so libuniconf.a
TARGETS += wvtestmain.o libwvtest.a
TARGETS += uniconf/daemon/uniconfd uniconf/tests/uni
TARGETS += crypto/tests/ssltest ipstreams/tests/unixtest
TARGETS += crypto/tests/printcert
ifneq ("$(with_dbus)", "no")
TARGETS += dbus/tests/wvdbus dbus/tests/wvdbusd
endif
ifneq ("$(with_readline)", "no")
TARGETS += ipstreams/tests/wsd
endif
GARBAGE += wvtestmain.o tmp.ini .wvtest-total

ifneq ("$(with_qt)", "no")
  TARGETS += libwvqt.so libwvqt.a
endif

ifneq ("$(with_dbus)", "no")
  TARGETS += libwvdbus.so libwvdbus.a
endif

TARGETS_SO := $(filter %.so,$(TARGETS))
TARGETS_A := $(filter %.a,$(TARGETS))

GARBAGE += $(wildcard lib*.so.*)

DISTCLEAN += autom4te.cache config.mk config.log config.status \
		include/wvautoconf.h config.cache reconfigure

REALCLEAN += stamp-h.in configure include/wvautoconf.h.in

CPPFLAGS += -Iinclude -pipe
ARFLAGS = rs
RELEASE?=$(PACKAGE_VERSION)

DEBUG:=$(filter-out no,$(enable_debug))

CXXFLAGS+=$(if $(filter-out yes,$(DEBUG)), -DDEBUG_$(DEBUG))
CFLAGS+=$(if $(filter-out yes,$(DEBUG)), -DDEBUG_$(DEBUG))

ifeq ("$(enable_fatal_warnings)", "yes")
CXXFLAGS+=-Werror
# FIXME: not for C, because our only C file, crypto/wvsslhack.c, has
#        a few warnings.
#CFLAGS+=-Werror
endif

ifeq ("$(enable_testgui)", "no")
WVTESTRUN=env
endif

ifneq ("$(with_xplc)", "no")
LIBS+=$(LIBS_XPLC) -lm
endif

libwvutils.so-LIBS+=$(LIBS_PAM)

BASEOBJS= \
	utils/wvbuffer.o utils/wvbufferstore.o \
	utils/wvcont.o \
	utils/wverror.o \
	streams/wvfdstream.o \
	utils/wvfork.o \
	utils/wvhash.o \
	utils/wvhashtable.o \
	utils/wvlinklist.o \
	utils/wvmoniker.o \
	utils/wvregex.o \
	utils/wvscatterhash.o utils/wvsorter.o \
	utils/wvstring.o utils/wvstringlist.o \
	utils/wvstringmask.o \
	utils/strutils.o \
	utils/wvtask.o \
	utils/wvtimeutils.o \
	streams/wvistreamlist.o \
	utils/wvstreamsdebugger.o \
	streams/wvlog.o \
	streams/wvstream.o \
	uniconf/uniconf.o \
	uniconf/uniconfgen.o uniconf/uniconfkey.o uniconf/uniconfroot.o \
	uniconf/unihashtree.o \
	uniconf/unimountgen.o \
	uniconf/unitempgen.o \
	utils/wvbackslash.o \
	utils/wvencoder.o \
	utils/wvtclstring.o \
	utils/wvstringcache.o \
	uniconf/uniinigen.o \
	uniconf/unigenhack.o \
	uniconf/unilistiter.o \
	streams/wvfile.o \
	streams/wvstreamclone.o  \
	streams/wvconstream.o \
	utils/wvcrashbase.o

TESTOBJS = utils/wvtest.o 

# print the sizes of all object files making up libwvbase, to help find
# optimization targets.
basesize:
	size --total $(BASEOBJS)

micro: micro.o libwvbase.so

libwvbase.a libwvbase.so: $(filter-out uniconf/unigenhack.o,$(BASEOBJS))
libwvbase.a: uniconf/unigenhack_s.o
libwvbase.so: uniconf/unigenhack.o
libwvbase.so: LIBS+=$(LIBXPLC)

libwvutils.a libwvutils.so: $(filter-out $(BASEOBJS) $(TESTOBJS),$(call objects,utils))
libwvutils.so: libwvbase.so
libwvutils.so: -lz -lcrypt

libwvstreams.a libwvstreams.so: $(filter-out $(BASEOBJS), \
	$(call objects,configfile crypto ipstreams \
		$(ARCH_SUBDIRS) streams urlget))
libwvstreams.so: libwvutils.so libwvbase.so
libwvstreams.so: LIBS+=-lz -lssl -lcrypto 

libuniconf.a libuniconf.so: $(filter-out $(BASEOBJS), \
	$(call objects,uniconf))
libuniconf.a: uniconf/uniconfroot.o
libuniconf.so: libwvstreams.so libwvutils.so libwvbase.so

libwvdbus.a libwvdbus.so: $(call objects,dbus)
libwvdbus.so: libwvstreams.so libwvutils.so libwvbase.so
libwvdbus.so: LIBS+=$(LIBS_DBUS)

libwvtest.a: wvtestmain.o $(TESTOBJS)

ifeq ("$(wildcard /usr/lib/libqt-mt.so)", "/usr/lib/libqt-mt.so")
  libwvqt.so-LIBS+=-lqt-mt
else 
  # RedHat has a pkgconfig file we can use to sort out this mess..
  ifeq ("$(wildcard /usr/lib/pkgconfig/qt-mt.pc)", "/usr/lib/pkgconfig/qt-mt.pc")
    libwvqt.so-LIBS+=`pkg-config --libs qt-mt`
  else
    libwvqt.so-LIBS+=-lqt
  endif
endif
libwvqt.a libwvqt.so: $(call objects,qt)
libwvqt.so: libwvutils.so libwvstreams.so libwvbase.so

libwvgtk.a libwvgtk.so: $(call objects,gtk)
libwvgtk.so: -lgtk -lgdk libwvstreams.so libwvutils.so libwvbase.so
WVSTREAMS=.
WVSTREAMS_SRC= # Clear WVSTREAMS_SRC so wvrules.mk uses its WVSTREAMS_foo
VPATH=$(libdir)
include wvrules.mk
override enable_efence=no

ifneq (${_WIN32},)
  $(error "Use 'make -f Makefile-win32' instead!")
endif

export WVSTREAMS

XPATH=include

SUBDIRS =

all: runconfigure xplc $(TARGETS)

.PHONY: xplc xplc/clean install-xplc
xplc:
xplc/clean:
install-xplc:

ifeq ("$(build_xplc)", "yes")

xplc:
	$(MAKE) -C xplc

xplc/clean:
	$(MAKE) -C xplc clean

install-xplc: xplc
	$(INSTALL) -d $(DESTDIR)$(includedir)/wvstreams/xplc
	$(INSTALL_DATA) $(wildcard xplc/include/xplc/*.h) $(DESTDIR)$(includedir)/wvstreams/xplc
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL_DATA) xplc/libxplc-cxx.a $(DESTDIR)$(libdir)

endif

.PHONY: clean depend dust kdoc doxygen install install-shared install-dev install-xplc uninstall tests dishes dist distclean realclean test

# FIXME: little trick to ensure that the wvautoconf.h.in file is there
.PHONY: dist-hack-clean
dist-hack-clean:
	@rm -f stamp-h.in

export AM_CFLAGS
AM_CFLAGS=-fPIC

# Comment this assignment out for a release.
ifdef PKGSNAPSHOT
SNAPDATE=+$(shell date +%Y%m%d)
endif

dist-hook: dist-hack-clean configure
	@rm -rf autom4te.cache
	@if test -d .xplc; then \
	    echo '--> Preparing XPLC for dist...'; \
	    $(MAKE) -C .xplc clean patch && \
	    cp -Lpr .xplc/build/xplc .; \
	fi

runconfigure: config.mk include/wvautoconf.h

ifndef CONFIGURING
configure=$(error Please run the "configure" script)
else
configure:=
endif

config.mk: configure config.mk.in
	$(call configure)

include/wvautoconf.h: include/wvautoconf.h.in
	$(call configure)

# FIXME: there is some confusion here
ifdef WE_ARE_DIST
aclocal.m4: acinclude.m4
	$(warning "$@" is old, please run "aclocal")

configure: configure.ac config.mk.in include/wvautoconf.h.in aclocal.m4
	$(warning "$@" is old, please run "autoconf")

include/wvautoconf.h.in: configure.ac aclocal.m4
	$(warning "$@" is old, please run "autoheader")
else
aclocal.m4: acinclude.m4
	aclocal
	@touch $@

configure: configure.ac include/wvautoconf.h.in aclocal.m4
	autoconf
	@rm -f config.mk include/wvautoconf.h
	@touch $@

include/wvautoconf.h.in: configure.ac aclocal.m4
	autoheader
	@touch $@
endif

ifeq ($(VERBOSE),)
define wild_clean
	@list=`echo $(wildcard $(1))`; \
		test -z "$${list}" || sh -c "rm -rf $${list}"
endef
else
define wild_clean
	@list=`echo $(wildcard $(1))`; \
		test -z "$${list}" || sh -cx "rm -rf $${list}"
endef
endif

realclean: distclean
	$(call wild_clean,$(REALCLEAN))


distclean: clean
	$(call wild_clean,$(DISTCLEAN))
	@rm -rf autom4te.cache
	@rm -f pkgconfig/*.pc
	@rm -f .xplc

clean: depend dust xplc/clean
	$(subdirs)
	$(call wild_clean,$(TARGETS) uniconf/daemon/uniconfd \
		$(GARBAGE) $(TESTS) tmp.ini \
		$(shell find . -name '*.o' -o -name '*.moc'))

depend:
	$(call wild_clean,$(shell find . -name '.*.d'))

dust:
	$(call wild_clean,$(shell find . -name 'core' -o -name '*~' -o -name '.#*') $(wildcard *.d))

kdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

ifeq ("$(with_readline)", "no")
install: install-shared install-dev install-xplc install-uniconfd
else
install: install-shared install-dev install-xplc install-uniconfd install-wsd
endif

install-shared: $(TARGETS_SO)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_SO); do \
	    $(INSTALL_PROGRAM) $$i.$(SO_VERSION) $(DESTDIR)$(libdir)/ ; \
	done
	$(INSTALL) -d $(DESTDIR)$(sysconfdir)
	$(INSTALL_DATA) uniconf/daemon/uniconf.conf $(DESTDIR)$(sysconfdir)/

install-dev: $(TARGETS_SO) $(TARGETS_A)
	$(INSTALL) -d $(DESTDIR)$(includedir)/wvstreams
	$(INSTALL_DATA) $(wildcard include/*.h) $(DESTDIR)$(includedir)/wvstreams
	$(INSTALL) -d $(DESTDIR)$(libdir)
	for i in $(TARGETS_A); do \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(libdir); \
	done
	cd $(DESTDIR)$(libdir) && for i in $(TARGETS_SO); do \
	    rm -f $$i; \
	    $(LN_S) $$i.$(SO_VERSION) $$i; \
	done
	$(INSTALL) -d $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(filter-out %-uninstalled.pc, $(wildcard pkgconfig/*.pc)) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) wvtesthelper wvtestmeter $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(libdir)/valgrind
	$(INSTALL) wvstreams.supp $(DESTDIR)$(libdir)/valgrind

uniconfd: uniconf/daemon/uniconfd uniconf/daemon/uniconfd.ini \
          uniconf/daemon/uniconfd.8

install-uniconfd: uniconfd uniconf/tests/uni uniconf/tests/uni.8
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) uniconf/tests/uni $(DESTDIR)$(bindir)/
	$(INSTALL) -d $(DESTDIR)$(sbindir)
	$(INSTALL_PROGRAM) uniconf/daemon/uniconfd $(DESTDIR)$(sbindir)/
	$(INSTALL) -d $(DESTDIR)$(localstatedir)/lib/uniconf
	touch $(DESTDIR)$(localstatedir)/lib/uniconf/uniconfd.ini
	$(INSTALL) -d $(DESTDIR)$(mandir)/man8
	$(INSTALL_DATA) uniconf/daemon/uniconfd.8 $(DESTDIR)$(mandir)/man8
	$(INSTALL_DATA) uniconf/tests/uni.8 $(DESTDIR)$(mandir)/man8

install-wsd: ipstreams/tests/wsd
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) ipstreams/tests/wsd $(DESTDIR)$(bindir)/

uninstall:
	$(tbd)

$(TESTS): $(LIBUNICONF) $(LIBWVTEST) $(LIBWVDBUS)
$(addsuffix .o,$(TESTS)):
tests: $(TESTS)

-include $(shell find . -name '.*.d') /dev/null

test: runconfigure all tests qtest

qtest: runconfigure all wvtestmain
	LD_LIBRARY_PATH="$(LD_LIBRARY_PATH):$(WVSTREAMS_LIB)" $(WVTESTRUN) $(MAKE) runtests

runtests:
	$(VALGRIND) ./wvtestmain '$(TESTNAME)'
ifeq ("$(TESTNAME)", "unitest")
	cd uniconf/tests && DAEMON=0 ./unitest.sh
	cd uniconf/tests && DAEMON=1 ./unitest.sh
endif

wvtestmain: \
	$(call objects, $(filter-out ./Win32WvStreams/%, \
		$(shell find . -type d -name t))) \
	$(LIBWVDBUS) $(LIBUNICONF) $(LIBWVSTREAMS) $(LIBWVTEST)

