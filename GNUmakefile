PROJECT_NAME := vnlog

ABI_VERSION  := 0
TAIL_VERSION := 1

LIB_SOURCES := *.c*
BIN_SOURCES := test/test1.c

TOOLS :=					\
  vnl-filter					\
  vnl-align					\
  vnl-sort					\
  vnl-join					\
  vnl-tail					\
  vnl-ts					\
  vnl-uniq                                      \
  vnl-gen-header				\
  vnl-make-matrix


# I construct the README.org from the template. The only thing I do is to insert
# the manpages. Note that this is more complicated than it looks:
#
# 1. The tools are written in perl and contain POD documentation
# 2. This documentation is stripped out here with pod2text, and included in the
#    README. This README is an org-mode file, and the README.template.org
#    container included the manpage text inside a #+BEGIN_EXAMPLE/#+END_EXAMPLE.
#    So the manpages are treated as a verbatim, unformatted text blob
# 3. Further down, the same POD is converted to a manpage via pod2man
define MAKE_README =
BEGIN									\
{									\
  for $$a (@ARGV)							\
  {									\
    $$c{$$a} = `pod2text $$a | mawk "/REPOSITORY/{exit} {print}"`;	\
  }									\
}									\
									\
while(<STDIN>)								\
{									\
  print s/xxx-manpage-(.*?)-xxx/$$c{$$1}/gr;				\
}
endef

README.org: README.template.org $(TOOLS)
	< $(filter README%,$^) perl -e '$(MAKE_README)' $(filter-out README%,$^) > $@
all: README.org



b64_cencode.o: CFLAGS += -Wno-implicit-fallthrough

# Make can't deal with ':' in filenames, so I hack it
coloncolon := __colon____colon__
doc: $(addprefix man1/,$(addsuffix .1,$(TOOLS)))  $(patsubst lib/Vnlog/%.pm,man3/Vnlog$(coloncolon)%.3pm,$(wildcard lib/Vnlog/*.pm))
.PHONY: doc

%/:
	mkdir -p $@

man1/%.1: % | man1/
	pod2man -r '' --section 1 --center "vnlog" $< $@
man3/Vnlog$(coloncolon)%.3pm: lib/Vnlog/%.pm | man3/
	pod2man -r '' --section 3pm --center "vnlog" $< $@
EXTRA_CLEAN += man1 man3

CFLAGS := -I. -std=gnu99 -Wno-missing-field-initializers

test/test1: test/test2.o
test/test1.o: test/vnlog_fields_generated1.h
test/test2.o: test/vnlog_fields_generated2.h
test/vnlog_fields_generated%.h: test/vnlog%.defs vnl-gen-header
	./vnl-gen-header < $< | perl -pe 's{vnlog/vnlog.h}{vnlog.h}' > $@
EXTRA_CLEAN += test/vnlog_fields_generated*.h test/*.got

# Set up the test suite to be runnable in parallel
test check:					\
   test/test_vnl-filter.pl.RUN			\
   test/test_vnl-sort.pl.RUN			\
   test/test_vnl-join.pl.RUN			\
   test/test_vnl-uniq.pl.RUN			\
   test/test_c_api.sh.RUN			\
   test/test_perl_parser.pl.RUN			\
   test/test_python2_parser.sh.RUN              \
   test/test_python3_parser.sh.RUN
	@echo "All tests in the test suite passed!"
.PHONY: test check
%.RUN: %
	$<
test/test_c_api.sh.RUN: test/test1
EXTRA_CLEAN += test/testdata_*


DIST_INCLUDE      := vnlog*.h
DIST_BIN          := $(TOOLS)
DIST_PERL_MODULES := lib/Vnlog
DIST_PY2_MODULES  := lib/vnlog.py
DIST_PY3_MODULES  := lib/vnlog.py

install: doc
DIST_MAN     := man1/ man3/







# The main mrbuild Makefile follows. Most of it isn't used here, but it's less
# maintenance to bring in the whole thing. Ideally I'd include an installed
# copy, but I don't want to introduce a dependency

# -*- Makefile -*-

# This is a part of the mrbuild project: https://github.com/dkogan/mrbuild
#
# Released under an MIT-style license. Modify and distribute as you like:
#
# Copyright 2016-2019 California Institute of Technology
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.



# This is a common Makefile that can be used as the core buildsystem for
# projects providing a library and some executables using this library. Please
# see README.build.org for the documentation.


# There are two ways to pass variables to make:
#
# make CFLAGS=-foo
#   and
# CFLAGS=-foo make
#
# The former creates a "command line" variable and the latter an
# "environment variable". In order to be able to modify a "command line"
# variable (to add other flags, say), one MUST use 'override'. So one would have to do
#
# override CFLAGS += -bar
#
# without the "override" nothing would happen. I want to avoid this rabbithole
# entirely, so I disallow "command line" variables for things that I modify.
#
# I only do this for xxxFLAGS becuase cross-building and installing in Debian
# uses Make in this way. Hopefully this is safe enough
$(foreach v,$(filter %FLAGS,$(.VARIABLES)),$(if $(filter command line,$(origin $v)), $(error '$v' not allowed as a make parameter. Please do "$v=xxxx make yyyy" instead of "make yyyy $v=xxxx")))



# Make sure I have the variables that must be defined. Libraries need an ABI and
# TAIL version, while other need a plain VERSION
MUST_DEF_VARIABLES        := PROJECT_NAME $(if $(LIB_SOURCES),ABI_VERSION TAIL_VERSION,VERSION)
$(foreach v,$(MUST_DEF_VARIABLES),$(if $($v),,$(error User MUST specify $v)))



# The default VERSION string that appears as a #define to each source file, and
# to any generated documentation (gengetopt and so on). The user can set this to
# whatever they like
VERSION ?= $(ABI_VERSION).$(TAIL_VERSION)

# Default compilers. By default, we use g++ as a linker
CC        ?= gcc
CXX       ?= g++
NVCC      ?= nvcc
CC_LINKER ?= $(CXX)

# used to make gcc output header dependency information. All source
# files generated .d dependency definitions that are included at the
# bottom of this file
CCXXFLAGS += -MMD -MP

# always building with debug information. This is stripped into the
# -dbg/-debuginfo packages by debhelper/rpm later
CCXXFLAGS += -g

# I want the frame pointer. Makes walking the stack WAY easier
CCXXFLAGS += -fno-omit-frame-pointer

# I look through my LIB_SOURCES and BIN_SOURCES. Anything that isn't a wildcard
# (has * or ?) should exist. If it doesn't, the user messed up and I flag it
get_no_wildcards          = $(foreach v,$1,$(if $(findstring ?,$v)$(findstring *,$v),,$v))
complain_if_nonempty      = $(if $(strip $1),$(error $2: $1))
complain_unless_all_exist = $(call complain_if_nonempty,$(call get_no_wildcards,$(filter-out $(wildcard $1),$1)),File not found: )
$(call complain_unless_all_exist,$(LIB_SOURCES) $(BIN_SOURCES))


LIB_SOURCES := $(wildcard $(LIB_SOURCES))
BIN_SOURCES := $(wildcard $(BIN_SOURCES))

LIB_OBJECTS := $(addsuffix .o,$(basename $(LIB_SOURCES)))
BIN_OBJECTS := $(addsuffix .o,$(basename $(BIN_SOURCES)))

SOURCE_DIRS := $(sort ./ $(dir $(LIB_SOURCES) $(BIN_SOURCES)))

# if the PROJECT_NAME is libxxx then LIB_NAME is libxxx
# if the PROJECT_NAME is xxx    then LIB_NAME is libxxx
LIB_NAME           := $(or $(filter lib%,$(PROJECT_NAME)),lib$(PROJECT_NAME))
LIB_TARGET_SO_BARE := $(LIB_NAME).so
LIB_TARGET_SO_ABI  := $(LIB_TARGET_SO_BARE).$(ABI_VERSION)
LIB_TARGET_SO_FULL := $(LIB_TARGET_SO_ABI).$(TAIL_VERSION)
LIB_TARGET_SO_ALL  := $(LIB_TARGET_SO_BARE) $(LIB_TARGET_SO_ABI) $(LIB_TARGET_SO_FULL)

BIN_TARGETS := $(basename $(BIN_SOURCES))


# all objects built for inclusion in shared libraries get -fPIC. We don't build
# static libraries, so this is 100% correct
$(LIB_OBJECTS): CCXXFLAGS += -fPIC

CCXXFLAGS += -DVERSION='"$(VERSION)"'




# These are here to process the options separately for each file being built.
# This allows per-target options to be set
#
# if no explicit optimization flags are given, optimize
define massageopts
$1 $(if $(filter -O%,$1),,-O3)
endef

# If no C++ standard requested, I default to c++0x
define massageopts_cxx
$(call massageopts,$1 $(if $(filter -std=%,$1),,-std=c++0x))
endef

define massageopts_c
$(call massageopts,$1)
endef



# define the compile rules. I need to redefine the rules here because my
# C..FLAGS variables are simple (immediately evaluated), but the user
# could have specified per-target flags that ALWAYS evaluate deferred-ly
#
# I add the warning options AT THE START of the flag list so that the user can
# override these
cc_build_rule = $(strip $(CXX)  $(call massageopts_cxx,-Wall -Wextra $(CXXFLAGS) $(CCXXFLAGS) $(CPPFLAGS))) -c -o $@ $<
c_build_rule  = $(strip $(CC)   $(call massageopts_c,  -Wall -Wextra $(CFLAGS)   $(CCXXFLAGS) $(CPPFLAGS))) -c -o $@ $<
cu_build_rule = $(strip $(NVCC) $(call massageopts_c,  -Wall -Wextra $(CUFLAGS)               $(CPPFLAGS))) -c -o $@ $<
cu_build_rule += --compiler-options "-Wall -Wextra $(CCXXFLAGS)"


%.o:%.C
	$(cc_build_rule)
%.o:%.cc
	$(cc_build_rule)
%.o:%.cpp
	$(cc_build_rule)
%.o:%.c
	$(c_build_rule)
%.o:%.cu
	$(cu_build_rule)
%.o: %.S
	$(CC) $(ASFLAGS) $(CPPFLAGS) -c -o $@ $<

# gengetopt rule. If we have GENGETOPT_OPTIONS, use those; otherwise use some
# sensible defaults. If we don't have -F set an --output-dir also
%.h %.c: %.ggo
	gengetopt -i $< \
	  $(if $(GENGETOPT_OPTIONS),$(if $(filter -F%,$(GENGETOPT_OPTIONS)),,--output-dir $(dir $<))) \
	  $(or $(GENGETOPT_OPTIONS), -C -u -g $(VERSION) -F $* args -f $(notdir $*) -a $(notdir $*))

# this is how you build QT4 UIs. There's a tool to generate headers from
# UI definitions. There's also a tool to generate metadata from QT
# classes. This must be compiled and linked in. QT4_MOCS is a list of
# all these extra objects needed by QT. Simply add $(QT4_MOCS) to your
# objects list and magic happens. Similar with QT4_UI_HEADERS
ui_%.h: %.ui
	uic-qt4 $< > $@
moc_%.cpp: %.h
	moc-qt4 $< > $@

ui_%.hh: %.ui
	uic-qt4 $< > $@
moc_%.cpp: %.hh
	moc-qt4 $< > $@


# Python docstring rules. I construct these from plain ASCII files to handle
# line wrapping
%.docstring.h: %.docstring
	< $^ sed 's/\\/\\\\/g; s/"/\\"/g; s/^/"/; s/$$/\\n"/;' > $@
EXTRA_CLEAN += *.docstring.h





# by default I build shared libraries only. We known how to build static
# libraries too, but I don't do it unless asked
all: $(if $(strip $(LIB_SOURCES)),$(LIB_TARGET_SO_ALL)) $(if $(strip $(BIN_SOURCES)),$(BIN_TARGETS))
.PHONY: all
.DEFAULT_GOAL := all

# use --default-symver if we've got it. *BSDs do not
LD_DEFAULT_SYMVER := $(shell ld --default-symver --version 1>/dev/null 2>/dev/null && echo -Wl,--default-symver)
$(LIB_TARGET_SO_FULL): LDFLAGS += -shared $(LD_DEFAULT_SYMVER) -fPIC -Wl,-soname,$(notdir $(LIB_TARGET_SO_BARE)).$(ABI_VERSION)


$(LIB_TARGET_SO_BARE) $(LIB_TARGET_SO_ABI): $(LIB_TARGET_SO_FULL)
	ln -fs $(notdir $(LIB_TARGET_SO_FULL)) $@

# Here instead of specifying $^, I do just the %.o parts and then the
# others. This is required to make the linker happy to see the dependent
# objects first and the dependency objects last. Same as for BIN_TARGETS
$(LIB_TARGET_SO_FULL): $(LIB_OBJECTS)
	$(CC_LINKER) $(LDFLAGS) $(filter %.o, $^) $(filter-out %.o, $^) $(LDLIBS) -o $@

# I make sure to give the .o to the linker before the .so and everything else.
# The .o may depend on the other stuff.
#
# The binaries get an RPATH (removed at install time). I use a relative RPATH
# (using $ORIGIN) to make the build reproducible: chrpath doesn't remove the
# actual RPATH string from the binary, and the local build directory gets
# embedded in the output
#
# Here $^ contains two flavors of LIB_TARGET (see next stanza), so I manually
# remove one
SPACE :=
SPACE := $(SPACE) $(SPACE)
dirs_to_dotdot = $(subst $(SPACE),/,$(patsubst %,..,$(subst /, ,$1)))
get_parentdir_relative_to_childdir = /$(call dirs_to_dotdot,$(patsubst $1%,%,$2))

$(BIN_TARGETS): %: %.o
	$(CC_LINKER) -Wl,-rpath='$$ORIGIN'$(call get_parentdir_relative_to_childdir,$(abspath .),$(dir $(abspath $@))) $(LDFLAGS) $(filter %.o, $^) $(filter-out $(LIB_TARGET_SO_ABI),$(filter-out %.o, $^)) $(LDLIBS) -o $@

# The binaries link with the DSO, if there is one. I need the libxxx.so to build
# the binary, and I need the libxxx.so.abi to run it.
$(BIN_TARGETS): $(if $(strip $(LIB_SOURCES)),$(LIB_TARGET_SO_BARE) $(LIB_TARGET_SO_ABI))




clean:
	rm -rf $(foreach d,$(SOURCE_DIRS),$(addprefix $d,*.a *.o *.so *.so.* *.d moc_* ui_*.h*)) $(BIN_TARGETS) $(foreach s,.c .h,$(addsuffix $s,$(basename $(shell find . -name '*.ggo')))) $(EXTRA_CLEAN)
distclean: clean

.PHONY: distclean clean



########################### installation business

ifneq (,$(filter install,$(MAKECMDGOALS)))
  ifeq  ($(strip $(DESTDIR)),)
    $(error Tried to make install without having DESTDIR defined \
"make install" is ONLY for package building. \
What are you trying to do?)
  endif

ifneq (,$(strip $(DIST_PY2_MODULES)))
PY2_MODULE_PATH := $(shell python2  -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")
$(if $(PY2_MODULE_PATH),,$(error "Couldn't find the python2 module path!"))
endif

ifneq (,$(strip $(DIST_PY3_MODULES)))
PY3_MODULE_PATH := $(shell python3  -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")
$(if $(PY3_MODULE_PATH),,$(error "Couldn't find the python3 module path!"))
endif

USE_DEBIAN_PATHS := $(wildcard /etc/debian_version)
ifneq (,$(USE_DEBIAN_PATHS))
  # we're a debian-ish box, use the multiarch dir
  DEB_HOST_MULTIARCH := $(shell dpkg-architecture -qDEB_HOST_MULTIARCH 2>/dev/null)
  USRLIB             := usr/lib/$(DEB_HOST_MULTIARCH)
else
  # we're something else. Do what CentOS does.
  # If /usr/lib64 exists, use that. Otherwise /usr/lib
  USRLIB := $(if $(wildcard /usr/lib64),usr/lib64,usr/lib)
endif

endif


# I process the simple wildcard exceptions on DIST_BIN and DIST_INCLUDE in a
# deferred fashion. The reason is that I wand $(wildcard) to run at install
# time, i.e. after stuff is built, and the files $(wildcard) is looking at
# already exist
DIST_BIN_ORIG     := $(DIST_BIN)
DIST_INCLUDE_ORIG := $(DIST_INCLUDE)

DIST_BIN     = $(filter-out $(wildcard $(DIST_BIN_EXCEPT)),		\
                  $(wildcard $(or $(DIST_BIN_ORIG),    $(BIN_TARGETS))))
DIST_INCLUDE = $(filter-out $(wildcard $(DIST_INCLUDE_EXCEPT) *.docstring.h), \
		  $(wildcard $(DIST_INCLUDE_ORIG)))

MANDIR := usr/share/man

# Generates the install rules. Arguments:
#   1. variable containing the being installed
#   2. target path they're being installed to
define install_rule
$(if $(strip $($1)),
	mkdir -p $2 &&									\
	cp -r $($1) $2 &&								\
	$(if $($(1)_EXCEPT_FINDSPEC),find $2 $($(1)_EXCEPT_FINDSPEC) -delete, true) )
endef

# Redhat wants the various manpage section get their own subdir for some reason
define move_manpages_for_redhat
  mkdir -p $(DESTDIR)/$(MANDIR)/man$1                        && \
  (mv $(DESTDIR)/$(MANDIR)/*.$1    $(DESTDIR)/$(MANDIR)/man$1 2>/dev/null || true) && \
  (mv $(DESTDIR)/$(MANDIR)/*.$1.gz $(DESTDIR)/$(MANDIR)/man$1 2>/dev/null || true) && \
  (rmdir $(DESTDIR)/$(MANDIR)/man$1                           2>/dev/null || true)
endef

ifneq ($(strip $(LIB_SOURCES)),)
install: $(LIB_TARGET_SO_ALL)
endif

install: $(BIN_TARGETS) $(DIST_DOC) $(DIST_MAN) $(DIST_DATA)

# using 'cp -P' instead of 'install' because the latter follows links unconditionally
ifneq ($(strip $(LIB_SOURCES)),)
	mkdir -p $(DESTDIR)/$(USRLIB)
	cp -P $(LIB_TARGET_SO_FULL)  $(DESTDIR)/$(USRLIB)
	ln -fs $(notdir $(LIB_TARGET_SO_FULL)) $(DESTDIR)/$(USRLIB)/$(notdir $(LIB_TARGET_SO_ABI))
	ln -fs $(notdir $(LIB_TARGET_SO_FULL)) $(DESTDIR)/$(USRLIB)/$(notdir $(LIB_TARGET_SO_BARE))
endif
	$(call install_rule,DIST_BIN,         $(DESTDIR)/usr/bin)
	$(call install_rule,DIST_INCLUDE,     $(DESTDIR)/usr/include/$(PROJECT_NAME))
	$(call install_rule,DIST_DOC,         $(DESTDIR)/usr/share/doc/$(PROJECT_NAME))

# I install the manpages normally. On Redhat I need to shuffle them into
# different subdirectories. This is incomplete: sections that aren't simply
# digits 1-9 will not be moved. "3perl" is an example I can think of that won't
# work here. Good-enough for now
	$(call install_rule,DIST_MAN,         $(DESTDIR)/$(MANDIR))
ifeq (,$(USE_DEBIAN_PATHS))
	$(foreach s,1 2 3 4 5 6 7 8 9,$(call move_manpages_for_redhat,$s) && ) true
endif

	$(call install_rule,DIST_DATA,        $(DESTDIR)/usr/share/$(PROJECT_NAME))
	$(call install_rule,DIST_PERL_MODULES,$(DESTDIR)/usr/share/perl5)
	$(call install_rule,DIST_PY2_MODULES, $(DESTDIR)/$(PY2_MODULE_PATH))
	$(call install_rule,DIST_PY3_MODULES, $(DESTDIR)/$(PY3_MODULE_PATH))

        # In filenames I rename __colon__ -> :
        # This is required because Make can't deal with : in rules
	for fil in `find $(DESTDIR) -name '*__colon__*'`; do mv $$fil `echo $$fil | sed s/__colon__/:/g`; done

        # Remove rpaths from everything. /usr/bin is allowed to fail because
        # some of those executables aren't ELFs. On the other hand, any .so we
        # find IS en ELF. Those could live in a number of places, since they
        # could be extension modules for the various languages, and I thus look
        # for those EVERYWHERE
ifneq ($(strip $(DIST_BIN)),)
	chrpath -d $(DESTDIR)/usr/bin/* 2>/dev/null || true
endif
	find $(DESTDIR) -name '*.so' | xargs chrpath -d

        # Any perl programs need their binary path stuff stripped out. This
        # exists to let these run in-tree, but needs to be removed at
        # install-time (similar to an RPATH)
ifneq ($(strip $(DIST_BIN)),)
	for fil in `find $(DESTDIR)/usr/bin -type f`; do head -n1 $$fil | grep -q '^#!.*/perl$$' && perl -n -i -e 'print unless /^\s* use \s+ lib \b/x' $$fil || true; done
endif
	test -e $(DESTDIR)/usr/share/perl5 && find $(DESTDIR)/usr/share/perl5 -type f | xargs perl -n -i -e 'print unless /^\s* use \s+ lib \b/x' || true

ifneq ($(strip $(DIST_BIN)),)
        # Everything executable needs specific permission bits
	chmod 0755 $(DESTDIR)/usr/bin/*
endif

.PHONY: install



# I want to keep all the intermediate files always
.SECONDARY:

# the header dependencies
-include $(addsuffix *.d,$(SOURCE_DIRS))
