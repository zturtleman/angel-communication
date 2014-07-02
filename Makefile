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

FRAMEWORK_SRCS = \
	framework/conversation.cpp \
	framework/string.cpp \
	framework/lexer.cpp \
	framework/persona.cpp \
	framework/wordtypes.cpp

CLI_SRCS = $(FRAMEWORK_SRCS) \
	cli/main.cpp

IRC_SRCS = $(FRAMEWORK_SRCS) \
	irc/irc_main.cpp \
	irc/irc_backend.cpp

TEST_SRCS = $(FRAMEWORK_SRCS) \
	test/test_main.cpp

CLI_OBJS=$(subst .cpp,.o,$(CLI_SRCS))
IRC_OBJS=$(subst .cpp,.o,$(IRC_SRCS))
TEST_OBJS=$(subst .cpp,.o,$(TEST_SRCS))

ifeq ($(PLATFORM),mingw32)
	BINEXT=.exe
else
	BINEXT=
endif

all: angelcli$(BINEXT) angelirc$(BINEXT) angeltest$(BINEXT)

angelcli$(BINEXT): $(CLI_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(CLI_OBJS) $(LDLIBS)

angelirc$(BINEXT): $(IRC_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(IRC_OBJS) $(LDLIBS)

angeltest$(BINEXT): $(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(TEST_OBJS) $(LDLIBS)

depend: .depend

.depend: $(CLI_SRCS) $(IRC_SRCS) $(FRAMEWORK_SRCS)
	$(RM) ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(CLI_OBJS) $(IRC_OBJS)

dist-clean: clean
	$(RM) angelcli$(BINEXT) angelirc$(BINEXT) angeltest$(BINEXT) ./.depend

include ./.depend

