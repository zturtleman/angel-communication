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
#define IRC_IDENT	"angelcom" // user identifier, part of host name shown to other users
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
	const char *conversationName = channel ? channel : from;

	for ( i = 0; i < numBots; i++ ) {
		if ( bots[i].getNick() == from ) {
			speaker = &bots[i];
			break;
		}
	}

	// Don't re-add bot messages (this would cause them to be sent to server again)
	if ( speaker ) {
		return;
	}

	// HACK: should have a persona for everyone?...
	user.updateNick( from );
	speaker = &user;

	if ( channel ) {
		// HACK: if there are multiple bots in the same channel and using same conversation,
		// the messages will be duplicated (so only add for first bot...)
		if ( bots[0].getNick().icompareTo( to ) != 0 )
			return;
	}

	for ( i = 0; i < numCons; i++ ) {
		if ( conlist[i].name == conversationName ) {
			printf( "%s <%s> %s\n", conversationName, from, message );
			conlist[i].con.addMessage( speaker, message );
			return;
		}
	}

	// Create new direct conversation
	if ( i < MAX_CONS ) {
		printf( "Started new IRC conversation (%s wants to chat with %s).\n", from, to );
		conlist[i].name = conversationName;
		conlist[i].con.addPersona( speaker );
		for ( int b = 0; b < numBots; b++ ) {
			if ( bots[b].getNick() == to ) {
				conlist[i].con.addPersona( &bots[b] );
				break;
			}
		}
		printf( "%s <%s> %s\n", conversationName, from, message );
		conlist[i].con.addMessage( speaker, message );
		numCons++;
	} else {
		// TODO: Try to free a unused direct conversation
		for ( int b = 0; b < numBots; b++ ) {
			if ( bots[b].getNick() == to ) {
				bot_irc[b].SayTo( conversationName, "Sorry, no available conversation slot." );
				printf( "WARNING: All IRC conversation slots full (%s wants to chat with %s).\n", from, to );
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
			printf( "%s <%s> %s\n", conlist[i].name.c_str(), irc->GetNick(), message );
			break;
		}
	}
}

// this is called when persona wants to change name
void ANGELC_PersonaRename( const char *oldnick, const char *newnick ) {
	for ( int b = 0; b < numBots; b++ ) {
		if ( bots[b].getNick() == oldnick ) {
			bot_irc[b].RequestNick( newnick );
			return;
		}
	}

	printf( "ANGELC_PersonaRename: Unhandled local rename. %s -> %s\n", oldnick, newnick );
}

// IRC server says someone renamed
// TODO: Update Conversation lastAddressees
// TODO: Support multiple IRC networks
void ANGEL_IRC_NickChange( const char *oldnick, const char *newnick ) {
	printf( "* %s renamed to %s\n", oldnick, newnick );

	for ( int b = 0; b < numBots; b++ ) {
		if ( bots[b].getNick() == oldnick ) {
			bots[b].updateNick( newnick );
			return;
		}
	}

	// Update direct conversation, so person can continue conversation instead of starting a new one.
	// WISH: Might be better to have multiple names attached to conversations? Rename, quit, then rejoin with original name will cause a new conversation to be created if they direct chat again.
	// WISH: Could want to attach old name if new name is attached. so if user pings out and reconnects while their ghost is still present (using a fallback name)
	// WISH:   then rename to original name, we can 'learn' their alternate name(s). Actually, that might be useful as a general thing not just direct conversations.
	// WISH:   Though, what to do if started conversation with alt-name then rename to name that already has a conversation? Dump the non-alt I guess or merge them (after there is stuff to merge).
	for ( int i = 0; i < numCons; i++ ) {
		if ( conlist[i].name == oldnick ) {
			conlist[i].name = newnick;
			break;
		}
	}
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

	user.updateNick( "User" );
	user.setGender( GENDER_MALE );
	user.setAutoChat( false );

	bots[numBots].updateNick( "Angel" );
	bots[numBots].setFullName( "Angelica Anarchy" );
	bots[numBots].setGender( GENDER_FEMALE );
	numBots++;

	if ( argc >= 2 && !strcmp( argv[1], "--two" ) ) {
		bots[numBots].updateNick( "Sera" );
		bots[numBots].setFullName( "Seraph Anarchy" );
		bots[numBots].setGender( GENDER_FEMALE );
		numBots++;
	} else {
		// HACK always add extra persona so bot knows channel is "group chat" mode...
		bots[numBots].updateNick( "Dummy" );
		bots[numBots].setAutoChat( false );
		conlist[0].con.addPersona( &bots[numBots] );
	}

	for ( int i = 0; i < numBots; i++ ) {
		conlist[0].con.addPersona( &bots[i] );
	}

	conlist[0].con.addPersona( &user );
	conlist[0].name = IRC_CHANNEL;
	numCons++;

	bot_irc[0].Connect( IRC_SERVER, IRC_PORT, bots[0].getNick().c_str(), IRC_IDENT, bots[0].getFullName().c_str(), IRC_CHANNEL );
	time_t connectTime = time( NULL );

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
					bot_irc[i].Connect( IRC_SERVER, IRC_PORT, bots[i].getNick().c_str(), IRC_IDENT, bots[i].getFullName().c_str(), IRC_CHANNEL );
					connectTime = time( NULL );
				}
			}

			if ( delay < 0 || ( botDelay >= 0 && botDelay < delay ) ) {
				delay = botDelay;
			}
		}

		// wake up to ping server if idle too long
		if ( delay < 0 || delay > IrcClient::IDLE_PING_SECONDS ) {
			delay = IrcClient::IDLE_PING_SECONDS;
		}

		ircIdle( delay );
	}

	// never reached
	return 0;
}

