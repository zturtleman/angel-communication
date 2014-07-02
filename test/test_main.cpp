/*
Angel Communication
Copyright (C) 2013-2014 Zack Middleton <zturtleman@gmail.com>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <iostream>
#include <cmath>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include <fstream>

#include "../framework/angel.h"

using namespace AngelCommunication;

void ANGELC_PrintMessage( const AngelCommunication::Conversation *con, const AngelCommunication::Persona *speaker, const char *message ) {
	if ( !strncmp( message, "/me", 3 ) && ( message[3] == ' ' || message[3] == '\0' ) ) {
		printf("* %s%s\n", speaker->getName().c_str(), &message[3] );
	} else {
		printf("%s> %s\n", speaker->getName().c_str(), message );
	}
}

// this is called when persona wants to change name
void ANGELC_PersonaRename( const char *oldname, const char *newname ) {
	printf("* %s is now known as %s\n", oldname, newname );
}

void sighandler( int signum ) {
	printf("Caught signal %d\n", signum );
	exit( 1 );
}

int main( int argc, char **argv )
{
	const char *filename = "test/test.txt";
	Lexer lexer;

	printf( "Angel Communication Non-interactive Lexer Test Edition\n" );

	signal( SIGINT, sighandler );
	signal( SIGTERM, sighandler );

	if ( argc > 1 ) {
		filename = argv[1];
	}

	std::ifstream input( filename );

	if ( !input.good() ) {
		printf( "Failed to open %s\n", filename );
		return 1;
	}

	printf( "Opened %s\n", filename );

	for( std::string line; std::getline( input, line ); )
	{
		if ( line.size() == 0 )
			continue;

		printf( "IN : %s\n", line.c_str() );

		lexer.clear();
		lexer.parse( line.c_str() );

		printf( "OUT: %s\n", lexer.toString().c_str() );

		for ( int i = 0; i < lexer.getNumTokens(); i++ ) {
			printf( "  TOKEN %02d: %s\n", i, lexer[i].c_str() );
		}
	}

	return 0;
}

