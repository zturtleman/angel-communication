#
# Angel Communication Makefile
#

COMPILE_PLATFORM=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]'|sed -e 's/\//_/g')
ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

CXXFLAGS= -g
LDFLAGS= -g
LDLIBS=

SRCS=cli/main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
BACKUP=$(subst .cpp,.cpp~,$(SRCS))

ifeq ($(PLATFORM),mingw32)
	BINEXT=.exe
else
	BINEXT=
endif

all: angelcli$(BINEXT)

angelcli$(BINEXT): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) angelcli$(BINEXT) ./.depend

include ./.depend

