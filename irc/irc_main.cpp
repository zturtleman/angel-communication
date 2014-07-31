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
#include <sys/select.h>
#endif

#include "irc_backend.h"

#include "../framework/angel.h"

using namespace AngelCommunication;

// FIXME: these should be in a config file
#define IRC_SERVER	"wizard.local"
#define IRC_PORT	"6667"
#define IRC_CHANNEL	"#sandbox"
#define IRC_CONNECT_DELAY 20 // wait 20 seconds between connecting each bot


Persona user; // repersents all irc users... should probably have a persona for each?

#define MAX_BOTS 2
IrcClient bot_irc[MAX_BOTS];
Persona bots[MAX_BOTS];
int numBots = 0;

#define MAX_CONS 8
class ConList {
	public:
		String		name;
		Conversation con;
};

ConList conlist[MAX_CONS];

int numCons = 0;

// if from starts with "#" it's from a channel
void ANGEL_IRC_ReceiveMessage( const char *to, const char *from, const char *channel, const char *message )
{
	int i;
	Persona *speaker = NULL;

	for ( i = 0; i < numBots; i++ ) {
		if ( bots[i].getName() == from ) {
			speaker = &bots[i];
			break;
		}
	}

	if ( !speaker ) {
		// HACK: should have a persona for everyone?...
		user.updateName( from );
		speaker = &user;
	}

	if ( channel ) {
		// HACK: if there are multiple bots in the same channel and using same conversation,
		// the messages will be duplicated (so only add for first bot...)
		if ( bots[0].getName().icompareTo( to ) != 0 )
			return;

		// Don't re-add bot messages (this would cause them to be sent to server again)
		if ( speaker != &user ) {
			return;
		}

		for ( i = 0; i < numCons; i++ ) {
			if ( conlist[i].name == channel ) {
				conlist[i].con.addMessage( speaker, message );
				break;
			}
		}

		if ( i != numCons )
			return;
	}

	// Create new direct conversation
	if ( i < MAX_CONS ) {
		conlist[i].name = from;
		conlist[i].con.addPersona( speaker );
		for ( int b = 0; b < numBots; b++ ) {
			if ( bots[b].getName() == to ) {
				conlist[i].con.addPersona( &bots[b] );
				break;
			}
		}
		conlist[i].con.addMessage( speaker, message );
		numCons++;
	} else {
		// TODO: Try to free a unused direct conversation
		for ( int b = 0; b < numBots; b++ ) {
			if ( bots[b].getName() == to ) {
				bot_irc[b].SayTo( channel ? channel : from, "Sorry, no available conversation slot." );
				break;
			}
		}
	}
}

void ANGELC_PrintMessage( const AngelCommunication::Conversation *con, const AngelCommunication::Persona *speaker, const char *message )
{
	IrcClient *irc = NULL;

	for ( int b = 0; b < numBots; b++ ) {
		if ( &bots[b] == speaker ) {
			irc = &bot_irc[b];
			break;
		}
	}

	// ignore users
	if ( !irc )
		return;

	for ( int i = 0; i < numCons; i++ ) {
		if ( &conlist[i].con == con ) {
			irc->SayTo( conlist[i].name.c_str(), message );
			break;
		}
	}
}

// this is called when persona wants to change name
void ANGELC_PersonaRename( const char *oldname, const char *newname ) {
	for ( int b = 0; b < numBots; b++ ) {
		if ( bots[b].getName() == oldname ) {
			bot_irc[b].RequestNick( newname );
			return;
		}
	}

	printf( "ANGELC_PersonaRename: Unhandled local rename. %s -> %s\n", oldname, newname );
}

// IRC server says someone renamed
void ANGEL_IRC_NickChange( const char *oldname, const char *newname ) {
	for ( int b = 0; b < numBots; b++ ) {
		if ( bots[b].getName() == oldname ) {
			bots[b].updateName( newname );
			return;
		}
	}

	printf( "ANGEL_IRC_NickChange: Unhandled rename. %s -> %s\n", oldname, newname );
}

// wait until a set time passes or there is new socket data
void ircIdle( float waitInSeconds ) {
	fd_set rfds;
	struct timeval tv, *ptv;
	int retval;
	int highestSock = 0;

	double fractpart, intpart;

	/* Watch sockets to see when it has input. */
	FD_ZERO( &rfds );
	for ( int i = 0; i < numBots; i++ ) {
		// FIXME: check if connected!
		int sock = bot_irc[i].GetSocket();
		FD_SET( sock, &rfds );
		if ( sock+1 > highestSock ) {
			highestSock = sock+1;
		}
	}

	if ( waitInSeconds >= 0 ) {
		fractpart = modf(waitInSeconds , &intpart);

		/* Wait up to half a second. */
		tv.tv_sec = intpart;
		tv.tv_usec = 1000000.0f * fractpart;

		ptv = &tv;
	} else {
		// wait for socket data, ignore time
		ptv = NULL;
	}

	retval = select(highestSock, &rfds, NULL, NULL, ptv);

	// FIXME?: select failed
	//if (retval == -1)
	//	return;
}

void sighandler( int signum ) {
	for ( int i = 0; i < numBots; i++ ) {
		bot_irc[i].Disconnect( "Bye" );
	}

	exit( 1 );
}

int main( int argc, char **argv )
{
	printf(ANGEL_IRC_VERSION "\n");
	printf("Use ctrl-C to exit.\n");

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	user.updateName( "User" );
	user.setGender( GENDER_MALE );
	user.setAutoChat( false );

	bots[numBots].updateName( "Angel" );
	bots[numBots].setGender( GENDER_FEMALE );
	numBots++;

	if ( argc >= 2 && !strcmp( argv[1], "--two" ) ) {
		bots[numBots].updateName( "Sera" );
		bots[numBots].setGender( GENDER_FEMALE );
		numBots++;
	} else {
		// HACK always add extra persona so bot knows channel is "group chat" mode...
		bots[numBots].updateName( "Dummy" );
		bots[numBots].setAutoChat( false );
		conlist[0].con.addPersona( &bots[numBots] );
	}

	for ( int i = 0; i < numBots; i++ ) {
		conlist[0].con.addPersona( &bots[i] );
	}

	conlist[0].con.addPersona( &user );
	conlist[0].name = IRC_CHANNEL;
	numCons++;

	bot_irc[0].Connect( IRC_SERVER, IRC_PORT, bots[0].getName().c_str(), IRC_CHANNEL );
	std::time_t	connectTime = std::time( NULL );

	while (1)
	{
		for ( int i = 0; i < numBots; i++ ) {
			bot_irc[i].Update();
			bots[i].think();
		}

		// sleep until bots wants to think or receive socket data.
		float delay = -1, botDelay;

		for ( int i = 0; i < numBots; i++ ) {
			if ( bot_irc[i].Connected() ) {
				botDelay = bots[i].getSleepTime();
			} else {
				botDelay = ( connectTime + IRC_CONNECT_DELAY ) - time( NULL );

				if ( botDelay <= 0 ) {
					bot_irc[i].Connect( IRC_SERVER, IRC_PORT, bots[i].getName().c_str(), IRC_CHANNEL );
					connectTime = time( NULL );
				}
			}

			if ( delay < 0 || ( botDelay >= 0 && botDelay < delay ) ) {
				delay = botDelay;
			}
		}

		ircIdle( delay );
	}

	// never reached
	return 0;
}

