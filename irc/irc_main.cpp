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

#include "irc_backend.h"

#include "../framework/angel.h"

using namespace AngelCommunication;

// FIXME: these should be in a config file
#define IRC_SERVER	"wizard.local"
#define IRC_PORT	"6667"
#define IRC_CHANNEL	"#sandbox"


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

	user.setName( from ); // HACK

	// if there are multiple bots in the same channel and using same conversation,
	// the messages will be duplicated (so only add for first bot...)
	if ( channel && bots[0].getName() == to ) {
		for ( i = 0; i < numCons; i++ ) {
			if ( conlist[i].name == channel ) {
				conlist[i].con.addMessage( &user, message );
				break;
			}
		}

		if ( i != numCons )
			return;
	}

	// Create new direct conversation
	if ( i < MAX_CONS ) {
		conlist[i].name = from;
		conlist[i].con.addPersona( &user );
		for ( int b = 0; b < numBots; b++ ) {
			if ( bots[b].getName() == to ) {
				conlist[i].con.addPersona( &bots[b] );
				break;
			}
		}
		conlist[i].con.addMessage( &user, message );
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

	user.setName( "User" );
	user.setGender( GENDER_MALE );
	user.setAutoChat( false );

	bots[numBots].setName( "Angel" );
	bots[numBots].setGender( GENDER_FEMALE );
	numBots++;

	// Having two IRC clients doesn't seem to work.
	//bots[numBots].setName( "Sera" );
	//bots[numBots].setGender( GENDER_FEMALE );
	//numBots++;

	for ( int i = 0; i < numBots; i++ ) {
		conlist[0].con.addPersona( &bots[i] );
	}

	// add a third persona so bot knows it's group chat...
	Persona dummy;
	dummy.setName( "Dummy" );
	conlist[0].con.addPersona( &dummy );

	conlist[0].con.addPersona( &user );
	conlist[0].name = IRC_CHANNEL;
	numCons++;

	for ( int i = 0; i < numBots; i++ ) {
		bot_irc[i].Connect( IRC_SERVER, IRC_PORT, bots[i].getName().c_str(), IRC_CHANNEL );
	}

	while (1)
	{
		for ( int i = 0; i < numBots; i++ ) {
			bot_irc[i].Update();
			bots[i].think();
		}
	}

	// never reached
	return 0;
}

