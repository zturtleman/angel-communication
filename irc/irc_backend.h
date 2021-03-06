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

#ifndef ANGEL_IRC_BACKEND_INCLUDED
#define ANGEL_IRC_BACKEND_INCLUDED

#include "../framework/angel.h"
#include <ctime>

void ANGEL_IRC_ReceiveMessage( const char *to, const char *from, const char *channel, const char *message );
void ANGEL_IRC_NickChange( const char *oldnick, const char *newnick );

// Version reported to other IRC clients / shown in terminal at start up
// FIXME?: version is suppose to be formatted as 'client:version:platform'?
#define ANGEL_IRC_VERSION "Angel Communication IRC Client"

class IrcClient {
	private:
		bool connected;
		char *nick;
		char *channel;
		int sock; // socket handle
		int msgnum;
		char data[1025]; // hold up to 2 512 character IRC messages
		time_t packetTime;

		void UpdateNick( const char *nick );

		int sendall( int fd, const char *s, int len, int flags );

	public:
		// after not sending a packet to server for 30 seconds,
		// send a ping to keep the connection alive.
		static const int IDLE_PING_SECONDS = 30;

		IrcClient();
		~IrcClient();
		bool Connect( const char *server, const char *port, const char *nick, const char *ident, const char *realName, const char *channel );
		void Update();
		void Disconnect( const char *reason );

		void RequestNick( const char *nick );
		void SayTo( const char *target, const char *message );

		const char *GetNick() const;
		int GetSocket() const;
		bool Connected() const;
};

#endif // ANGEL_IRC_BACKEND_INCLUDED

