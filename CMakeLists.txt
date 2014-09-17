cmake_minimum_required (VERSION 2.8)
project ( "Angel Communication" CXX )

option( BUILD_CLI "Build Angel Command-line Interface" 1 )
option( BUILD_IRC "Build Angel IRC client" 1 )
option( BUILD_TEST "Build Angel Lexer Test" 1 )

if (MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

set( FRAMEWORK_SRCS
	framework/conversation.cpp
	framework/string.cpp
	framework/lexer.cpp
	framework/sentence.cpp
	framework/persona.cpp
	framework/wordtypes.cpp
)

set( CLI_SRCS
	${FRAMEWORK_SRCS}
	cli/cli_main.cpp
)

set( IRC_SRCS
	${FRAMEWORK_SRCS}
	irc/irc_main.cpp
	irc/irc_backend.cpp
)

set( TEST_SRCS
	${FRAMEWORK_SRCS}
	test/test_main.cpp
)


if ( BUILD_CLI )
	add_executable(angelcli ${CLI_SRCS})

	if(WIN32)
		target_link_libraries(angelcli ws2_32)
	endif()
endif()

if ( BUILD_IRC )
	add_executable(angelirc ${IRC_SRCS})

	if(WIN32)
		target_link_libraries(angelirc ws2_32)
	endif()
endif()

if ( BUILD_TEST )
	add_executable(angeltest ${TEST_SRCS})
endif()

