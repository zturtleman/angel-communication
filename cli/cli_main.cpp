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
#ifdef _WIN32
#include <winsock2.h>
#else
#include <termios.h>
#include <sys/select.h>
#include <unistd.h>

struct termios oldt;
#endif

#include "../framework/angel.h"

using namespace AngelCommunication;

Persona user, bot, bot2;

void ANGELC_PrintMessage( const AngelCommunication::Conversation *con, const AngelCommunication::Persona *speaker, const char *message ) {
	if ( !strncmp( message, "/me", 3 ) && ( message[3] == ' ' || message[3] == '\0' ) ) {
		printf("* %s%s\n", speaker->getNick().c_str(), &message[3] );
	} else {
		printf("%s> %s\n", speaker->getNick().c_str(), message );
	}
}

// this is called when persona wants to change name
void ANGELC_PersonaRename( const char *oldnick, const char *newnick ) {
	if ( user.getNick() == newnick
		|| bot.getNick() == newnick
		|| bot2.getNick() == newnick ) {
		printf("Nick \"%s\" is already in use.\n", newnick );
		return;
	}

	if ( user.getNick() == oldnick )
		user.updateNick( newnick );
	else if ( bot.getNick() == oldnick )
		bot.updateNick( newnick );
	else if ( bot2.getNick() == oldnick )
		bot2.updateNick( newnick );

	printf("* %s is now known as %s\n", oldnick, newnick );
}

bool charAvailable( float waitInSeconds ) {
	fd_set rfds;
	struct timeval tv, *ptv;
	int retval;

	double fractpart, intpart;

	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	if ( waitInSeconds >= 0 ) {
		fractpart = modf(waitInSeconds , &intpart);

		/* Wait up to half a second. */
		tv.tv_sec = intpart;
		tv.tv_usec = 1000000.0f * fractpart;

		ptv = &tv;
	} else {
		// wait for key press, ignore time
		ptv = NULL;
	}

	retval = select(1, &rfds, NULL, NULL, ptv);

	if (retval == -1)
		return 0;

	if (FD_ISSET(0, &rfds))
		return 1;

	return 0;
}

void cliShutdown() {
	printf("\rQuiting Angel Communication\n");
	fflush(stdout);
#ifndef _WIN32
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
#endif
}

void sighandler( int signum ) {
	cliShutdown();
	exit( 1 );
}

int main( int argc, char **argv )
{
#ifndef _WIN32
	struct termios newt;

	tcgetattr( STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	//newt.c_lflag &= ~(ICANON|ECHO|IEXTEN);
	/* also set TIME to 0 and MIN to 1 (getc will block until there 
		is atleast 1 char avalible, and return as soon as there is)*/
	//newt.c_cc[VTIME] = 0;
	//newt.c_cc[VMIN] = 1;
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);
#endif

	printf("Angel Communication CLI\n");
	printf("Type 'quit' to exit Angel Communication.\n");

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	Conversation room;

	bot.updateNick( "Angel" );
	bot.setFullName( "Angelica Anarchy" );
	bot.setGender( GENDER_FEMALE );
	room.addPersona( &bot );

	if ( argc >= 2 && !strcmp( argv[1], "--two" ) ) {
		bot2.updateNick( "Sera" );
		bot2.setFullName( "Seraph Anarchy" );
		bot2.setGender( GENDER_FEMALE );
		room.addPersona( &bot2 );
	}

	user.updateNick( "User" );
	user.setGender( GENDER_MALE );
	user.setAutoChat( false );
	room.addPersona( &user );


	std::string text;
	int ch;

	while (1)
	{
		// sleep until bots wants to think or key press.
		float delay = -1, botDelay;

		botDelay = bot.getSleepTime();
		if ( delay < 0 || ( botDelay >= 0 && botDelay < delay ) ) {
			delay = botDelay;
		}

		botDelay = bot2.getSleepTime();
		if ( delay < 0 || ( botDelay >= 0 && botDelay < delay ) ) {
			delay = botDelay;
		}

		if ( charAvailable( delay ) )
			ch = getchar();
		else
			ch = EOF;

		//printf("hit %d\n", ch);

		if (ch == '\n')
		{
			if ( text == "quit" )
			{
				cliShutdown();
				return 0;
			}

			if (text.size() > 0)
			{
				printf("\r");
				fflush(stdout);

				if ( !strncmp( text.c_str(), "/nick ", 6 ) ) {
					user.tryNick( &text[6] );
				} else {
					room.addMessage( &user, text.c_str() );
				}

				text.clear();
			}
		}
		else if ( ch == 127 ) // Mac OS X 'delete' (usually called backspace)
		{
			if ( text.size() > 0 )
				text.erase(text.size()-1);
			printf( "\r%s \b", text.c_str() );
			fflush(stdout);
		}
		else if ( ch != EOF && ( ch > 32 || ch == ' ' ) )
		{
			text.push_back(ch);
			putchar(ch);
			fflush(stdout);
		}

		bot.think();
		bot2.think();
	}

	// never reached
	return 0;
}

