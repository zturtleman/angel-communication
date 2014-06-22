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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h> // printf
#include <malloc.h> // free

#include <cstring>

#include "irc_backend.h"

IrcClient::IrcClient()
: nick( NULL ), sock( 0 ), connected( false ), msgnum( 0 )
{
	data[0] = 0;
}

IrcClient::~IrcClient()
{
	Disconnect( "Killed" );
	if ( nick ) {
		free( nick );
		nick = NULL;
	}
	if ( channel ) {
		free( channel );
		channel = NULL;
	}
}

bool IrcClient::Connect( const char *server, const char *port, const char *nick, const char *channel ) {
	int ret;
	struct addrinfo hints, *res;

	if ( connected ) {
		Disconnect( "connecting elsewhere" );
	}

	memset( &hints, 0, sizeof ( hints ) );

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ( ( ret = getaddrinfo( server, port, &hints, &res ) ) != 0 ) {
		printf( gai_strerror( ret ) );
		return false;
	}

	sock = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
	if ( ( ret = connect( sock, res->ai_addr, res->ai_addrlen ) ) != 0 ) {
		printf( gai_strerror( ret ) );
		return false;
	}

	freeaddrinfo( res );

	fcntl(sock, F_SETFL, O_NONBLOCK);

	// connected
	char buf[513];

	sprintf( buf, "USER %s 0 * :%s\r\n", nick, nick );
	send( sock, buf, strlen( buf ), 0 );

	sprintf( buf, "NICK %s\r\n", nick );
	send( sock, buf, strlen( buf ), 0 );

	// FIXME check for send failure?

	this->nick = strdup( nick );
	this->channel = strdup( channel );

	printf( "Connected to %s:%s\n", server, port );
	connected = true;

	return true;
}

void IrcClient::Update() {
	int newlen;
	char newbuf[513];
	char msg[513];
	char *buf, *eol, *p;

	if ( !connected ) {
		return;
	}

	while ( ( newlen = recv( sock, newbuf, sizeof (newbuf) - 1, 0 ) ) > 0 ) {
		newbuf[newlen] = 0;
		strcat( data, newbuf );

		eol = data;

		while ( 1 ) {
			buf = eol;
			p = strstr( eol, "\r\n" );

			if ( !p )
				break;

			p[0] = '\0';
			p[1] = '\0';
			if ( p[2] ) {
				p += 2; // point to start of next string or null-terminator
			}
			eol = p;

			printf("MESSAGE %d: %s\n", msgnum, buf );
			msgnum++;

		    if ( !strncmp( buf, "PING ", 5 ) ) {
		        buf[1] = 'O';
		        send( sock, buf, strlen(buf), 0 );
		    }
		    else if ( buf[0] == ':' ) {
				char *user = NULL;
				char *command = NULL;
				char *where = NULL;
				char *message = NULL;

				char *head = buf, *oldhead = buf, *end;
				for ( int i = 0; i < 3; i++ ) {
					head = strchr( head, ' ' );

					if ( !head )
						break;

					*head = '\0';
					head++;

					switch ( i ) {
						case 0:
							// user or server
							// :name!~ident@hostname or :blahblahserver
							user = oldhead+1; // skip ":"
							end = strchr( user, '!' );
							if ( end ) { *end = '\0'; } // from a user
							else { user = NULL; } // from a irc server
							break;
						case 1:
							command = oldhead;
							break;
						case 2:
							where = oldhead;

							// CTCP is formatted as :\x01VERSION\x01
							if ( head[0] == ':' && head[1] == 0x01 ) {
								//ctcp = head+2;
								//end = strchr( ctcp, 0x01 );
								//if ( end ) { *end = '\0'; }
								//else { ctcp = NULL; } // unknown data
							}
							// message
							else if ( head[0] == ':' ) {
								message = head+1;
							}
							break;
					}

					oldhead = head;
				}

				printf( "user=[%s], command=[%s], where=[%s], message=[%s]\n", user, command, where, message );

				if ( !command || !where ) {
					continue;
				}

				if ( !strcmp( command, "001" ) ) {
					sprintf(msg, "JOIN %s\r\n", this->channel );
					send( sock, msg, strlen(msg), 0 );
				}
				else if ( !strcmp( command, "PRIVMSG" ) ) {
					char *channelName;

					if ( !strcmp( where, nick ) ) {
						channelName = NULL; // direct message
					} else {
						channelName = where;
					}

					ANGEL_IRC_ReceiveMessage( nick, user, channelName, message );
				}
			}
		}

		if ( *eol ) {
			memmove( data, eol, strlen( eol ) + 1 );
		} else {
			data[0] = 0;
		}
    }
}

void IrcClient::Disconnect( const char *reason ) {
	char buf[513];

	if ( !connected ) {
		return;
	}

	sprintf( buf, "QUIT :%s\r\n", reason );
	send( sock, buf, strlen( buf ), 0 );

	close( sock );
	sock = 0;

	printf( "Disconnected (%s)\n", reason );
}

void IrcClient::SayTo( const char *target, const char *message ) {
	char msg[513];

	if ( !connected ) {
		return;
	}

	sprintf( msg, "PRIVMSG %s :%s\r\n", target, message );
	send( sock, msg, strlen(msg), 0 );
}

