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
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <stdio.h> // printf
#include <errno.h> // errno

#include <cstring>

#include "irc_backend.h"

#ifndef _WIN32
#define closesocket(x) close(x)
#endif

int IrcClient::sendall( int fd, const char *s, int len, int flags ) {
	int left = len;
	int val;

	while ( left > 0 ) {
		val = send( fd, s+len-left, left, flags );

		if ( val < 0 ) {
			printf( "WARNING: send errored: %s (errno %d)\n", strerror( errno ), errno );
			return -1;
		}

		left -= val;
	}

	// update time last packet was sent
	this->packetTime = std::time( NULL );

	return 0;
}

IrcClient::IrcClient()
: nick( NULL ), channel( NULL ), sock( 0 ), connected( false ), msgnum( 0 ), sentUSER ( false ), packetTime ( 0 )
{
	data[0] = 0;
}

IrcClient::~IrcClient()
{
	Disconnect( "Process killed" );
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
		printf( "Connected to %s:%s failed: getaddrinfo returned %d: %s\n", server, port, ret, gai_strerror( ret ) );
		return false;
	}

	sock = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
	if ( ( ret = connect( sock, res->ai_addr, res->ai_addrlen ) ) != 0 ) {
		printf( "Connected to %s:%s failed: connect returned %d: %s\n", server, port, ret, gai_strerror( ret ) );
		return false;
	}

	freeaddrinfo( res );

	// set socket as non-blocking
	{
#ifdef _WIN32
		u_long val = 1;
		ioctlsocket( sock, FIONBIO, &val );
#else
		int flags = fcntl( sock, F_GETFL, 0 );
		fcntl( sock, F_SETFL, flags | O_NONBLOCK );
#endif
	}

	RequestNick( nick );

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

			// debug helper
			//printf("%s: MESSAGE %d: %s\n", this->nick, msgnum, buf );
			msgnum++;

			if ( !strncmp( buf, "PING ", 5 ) ) {
				// server sent PING request, change to PONG and send back
				buf[1] = 'O';
				sendall( sock, buf, strlen(buf), 0 );
				//printf("%s: SENT: %s\n", this->nick, buf );
			}
			else if ( buf[0] == ':' ) {
				char *user = NULL;
				char *command = NULL;
				char *where = NULL;
				char *message = NULL;
				char *ctcp = NULL;

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

							if ( !strcmp( command, "NICK" ) ) {
								// nick has no where argument
								goto setMessage;
							}
							break;
						case 2:
							where = oldhead;

							// Handle :server 433 * Nick :Nickname is already in use.
							if ( !strcmp( command, "433" ) ) {
								user = head;
								oldhead = head;

								head = strchr( head, ' ' );

								if ( !head )
									break;

								*head = '\0';
								head++;

							}

setMessage:
							// CTCP is formatted as :\x01VERSION\x01, :\x01PING 1403415318\x01, etc
							if ( head[0] == ':' && head[1] == 0x01 ) {
								ctcp = head+2;

								end = strchr( ctcp, 0x01 );
								if ( end ) {
									*end = '\0';

									// split args from command
									end = strchr( ctcp, ' ' );
									if ( end ) {
										*end = '\0';
										message = end+1;
									}
								}
								else
								{
									ctcp = NULL;
									message = NULL;
								}
							}
							// message
							else if ( head[0] == ':' ) {
								message = head+1;
							}
							break;
					}

					oldhead = head;
				}

				// debug helper
				//printf( "user=[%s], command=[%s], where=[%s], ctcp=[%s], message=[%s]\n", user, command, where, ctcp, message );

				if ( !command ) {
					printf("WARNING: IRC message with no command. user=%s, where=%s, ctcp=%s, message=%s\n", user, where, ctcp, message );
					continue;
				}

				if ( !strcmp( command, "001" ) ) {
					sprintf( msg, "JOIN %s\r\n", this->channel );
					sendall( sock, msg, strlen(msg), 0 );
				}
				else if ( !strcmp( command, "433" ) && user ) {
					// Try nick with an underscore after it
					sprintf( msg, "%s_", user );

					ANGEL_IRC_NickChange( this->nick, msg );
					RequestNick( msg );
					UpdateNick( msg );
				}
				else if ( !strcmp( command, "PONG" ) && where && message ) {
					// server replied to our PING request
#if 0 // only useful for debugging
					time_t sentTime = atol( message );
					time_t currentTime = time( NULL );
					double diffSeconds = difftime( currentTime, sentTime );
					printf("%s: ping response from %s (%.f seconds)\n", this->nick, where, diffSeconds );
#endif
				}
				else if ( !strcmp( command, "NICK" ) && user && message ) {
					const char *oldname = user;
					const char *newname = message;

					if ( !strcmp( oldname, this->nick ) ) {
						UpdateNick( newname );
					}
					ANGEL_IRC_NickChange( oldname, newname );
				}
				else if ( !strcmp( command, "PRIVMSG" ) && user && where && message ) {
					if ( ctcp ) {
						// handle "ACTION" /me messages
						if ( !strcmp( ctcp, "ACTION" ) ) {
							char *channelName;

							if ( !strcmp( where, nick ) ) {
								channelName = NULL; // direct message
							} else {
								channelName = where;
							}

							sprintf( msg, "/me %s", message );
							ANGEL_IRC_ReceiveMessage( nick, user, channelName, msg );
						}
						else if ( !strcmp( ctcp, "PING" ) ) {
							sprintf( msg, "NOTICE %s :\001PING %s\001\r\n", user, message );
							sendall( sock, msg, strlen(msg), 0 );
						}
						// FIXME?: missing timezone, though time info doesn't really matter for bots currently...
						else if ( !strcmp( ctcp, "TIME" ) ) {
							time_t curtime;
							struct tm *loctime;

							curtime = time (NULL);
							loctime = localtime (&curtime);

							sprintf( msg, "NOTICE %s :\001TIME :%s\001\r\n", user, asctime (loctime) );
							sendall( sock, msg, strlen(msg), 0 );
						}
						else if ( !strcmp( ctcp, "VERSION" ) ) {
							sprintf( msg, "NOTICE %s :\001VERSION %s\001\r\n", user, ANGEL_IRC_VERSION );
							sendall( sock, msg, strlen(msg), 0 );
						}
						else {
							// this might be a bad idea... I almost missed adding ACTION which resulted in bot messaging anyone who used /me
							sprintf( msg, "NOTICE %s :\001ERRMSG %s : Query is unknown\001\r\n", user, ctcp );
							sendall( sock, msg, strlen(msg), 0 );

							printf("WARNING: Received unknown CTCP tag name (%s) from %s, acked ERRMSG back\n", ctcp, user);
						}
					} else {
						char *channelName;

						if ( !strcmp( where, nick ) ) {
							channelName = NULL; // direct message
						} else {
							channelName = where;
						}

						ANGEL_IRC_ReceiveMessage( nick, user, channelName, message );
					}
				}
				else {
					printf("WARNING: Unhandled IRC command (%s). user=%s, where=%s, ctcp=%s, message=%s\n", command, user, where, ctcp, message );
				}
			}
		}

		if ( *eol ) {
			memmove( data, eol, strlen( eol ) + 1 );
		} else {
			data[0] = 0;
		}
    }

	if ( newlen == 0 ) {
		Disconnect( "Connection closed" );
		return;
	}
	// EAGAIN/EWOULDBLOCK just means no data available right now, try again later
	else if ( newlen < 0 && errno != EAGAIN
#ifdef _WIN32 // Windows doesn't define EWOULDBLOCK and has errno set to 0 when nothing is available?
		&& errno != 0
#else
		&& errno != EWOULDBLOCK
#endif
		) {
		printf( "WARNING: recv errored: %s (errno %d)\n", strerror( errno ), errno );
	}

	// ping server to keep connection alive
	time_t currentTime = time( NULL );
	if ( difftime( currentTime, this->packetTime ) >= IDLE_PING_SECONDS ) {
		// TODO: use a msec time, time(NULL) is seconds. would need to fix the PONG handler too.
		sprintf( msg, "PING %ld\r\n", (long)currentTime );
		sendall( sock, msg, strlen(msg), 0 );
	}
}

void IrcClient::Disconnect( const char *reason ) {
	char buf[513];

	if ( !connected ) {
		return;
	}

	sprintf( buf, "QUIT :%s\r\n", reason );
	sendall( sock, buf, strlen( buf ), 0 );

	closesocket( sock );
	sock = 0;

	printf( "Disconnected (%s)\n", reason );

	connected = false;
	sentUSER = false;
	packetTime = 0;
}

void IrcClient::RequestNick( const char *nick ) {
	char buf[513];

	if ( this->nick && !strcmp( this->nick, nick ) )
		return;

	if ( !sentUSER ) {
		sprintf( buf, "USER %s 0 * :%s\r\n", nick, nick );
		sendall( sock, buf, strlen( buf ), 0 );
		sentUSER = true;
	}

	sprintf( buf, "NICK %s\r\n", nick );
	sendall( sock, buf, strlen( buf ), 0 );

	printf("IRC_CLIENT: Requested nick \"%s\".\n", nick );
}

void IrcClient::UpdateNick( const char *nick ) {
	char buf[513];

	printf("IRC_CLIENT: Update nick, \"%s\" -> \"%s\"\n", this->nick, nick );

	if ( this->nick && !strcmp( this->nick, nick ) )
		return;

	if ( this->nick ) {
		free( this->nick );
	}

	this->nick = strdup( nick );
}

void IrcClient::SayTo( const char *target, const char *message ) {
	char msg[513];

	if ( !connected ) {
		return;
	}

	if ( !strncmp( message, "/me", 3 ) && ( message[3] == ' ' || message[3] == '\0' ) ) {
		sprintf( msg, "PRIVMSG %s :\001ACTION%s\001\r\n", target, &message[3] );
	}
	else {
		sprintf( msg, "PRIVMSG %s :%s\r\n", target, message );
	}
	sendall( sock, msg, strlen(msg), 0 );
}

const char *IrcClient::GetNick() const
{
	return nick;
}

int IrcClient::GetSocket() const
{
	return sock;
}

bool IrcClient::Connected() const
{
	return connected;
}

