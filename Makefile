TOPDIR=.

DEBUG=0
PREFIX=/usr/local
LIBDIR=${PREFIX}/lib
INCDIR=${PREFIX}/include/wvstreams
MANDIR=${PREFIX}/man

INCFILES=$(wildcard include/*.h)
INCOUT=$(addprefix $(INCDIR)/,$(INCFILES))
LIBFILES=libwvutils.a libwvutils.so libwvstreams.a libwvstreams.so \
	 libwvcrypto.a libwvcrypto.so

#
#
# No user serviceable parts beyond this point!
#
#

export DEBUG

CC=${CXX}
#CXXOPTS += -fno-implement-inlines

export CC CXX CXXOPTS

-include wvrules.mk

SUBDIRS=utils streams configfile ipstreams crypto Docs

all: include $(SUBDIRS) $(LIBFILES)

include:
	rm -rf $@
	mkdir $@.new
	cd $@.new && for d in $(SUBDIRS); do ln -s ../$$d/*.h .; done
	mv $@.new $@

$(wildcard *.so) $(wildcard *.a): Makefile

libwvutils.so: utils/utils.libs -lz
libwvutils.a: utils/utils.libs

libwvstreams.so-LIBS=libwvutils.so
libwvstreams.so: ipstreams/ipstreams.libs
libwvstreams.a: ipstreams/ipstreams.libs

libwvcrypto.so-LIBS=libwvstreams.so -lssl -lcrypto
libwvcrypto.so: crypto/crypto.libs
libwvcrypto.a: crypto/crypto.libs

wvrules.mk:
	[ -e ../../wvrules.mk ] && ln -s ../../wvrules.mk .

genkdoc:
	kdoc -f html -d Docs/kdoc-html --name wvstreams --strip-h-path */*.h

doxygen:
	doxygen

install: all
	[ -d ${LIBDIR} ] || install -d ${LIBDIR}
	[ -d ${INCDIR} ] || install -d ${INCDIR}
	@set -x; for d in ${INCFILES}; do \
		install -m 0644 $$d ${INCDIR}; \
	done
	for d in ${LIBFILES} wvrules.mk; do \
		install -m 0644 $$d ${LIBDIR}; \
	done
	#strip --strip-debug ${LIBDI../wvstreams/libwvstreams.a

uninstall:
	cd ${LIBDIR}; rm -f ${LIBFILES}
	rm -f ${INCOUT}
	-rmdir ${INCDIR}

clean:
	rm -rf include Docs/doxy-html Docs/kdoc-html
	$(subdirs)
	[ -L wvrules.mk ] && rm -f wvrules.mk
