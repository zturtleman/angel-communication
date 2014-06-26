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

#ifndef ANGEL_PERSONA_INCLUDED
#define ANGEL_PERSONA_INCLUDED

#include <ctime>

#include "string.h"
#include "lexer.h"
#include "conversation.h"

namespace AngelCommunication
{

enum Gender
{
	GENDER_NONE,
	GENDER_FEMALE,
	GENDER_MALE
};

enum WaitReply
{
	WR_NONE,
	WR_SPECIFIED, // expects a specific reply
	WR_COMPLETE_LAST, // user gave incomplete message
	WR_AM_I_RIGHT, // I like it when you tell me I'm right
	WR_LISTENING_TO_ME, // are you even listening to me?

	WR_MAX
};

class Expectation;
class Message;

class Persona
{
	private:
		bool   autoChat;
		String name;
		String namePossesive;
		Gender gender;
		bool funReplies;

		std::vector<Expectation*> expectations; // expected reply information
		std::vector<Message*> messages; // unprocessed messages

		std::time_t nextUpdateTime;

	public:
		Persona();

		float getSleepTime();
		void think();
		bool processMessage( Message *message );

		void tryName( const String &name ); // try to rename
		void updateName( const String &name ); // actually rename

		void setGender( Gender gender );
		void setAutoChat( bool autoChat );

		const String &getName( void ) const;


		// Conversation communication
		void receiveMessage( Conversation *con, Persona *speaker, const String &text, int messageNum );
		void personaConnect( Conversation *con, Persona *persona );

		void addExpectation( Conversation *c, Persona *f, WaitReply wr );
		void addExpectation( Conversation *c, Persona *f, WaitReply wr, const String &str );
};

class Expectation
{
	public:
		Conversation	*con;
		Persona			*from;
		int				messageNum;		// latest messageNum at time of expectation (allows ignoring earlier messages)
		WaitReply		waitForReply;	// expectation type
		String			expstr;			// varies by expectation type

		// NOTE: putting things in ": blah(b), blah(b)" list is magical,
		//       just assigning vars doesn't work correct, causes con to be NULL and from to be wrong
		//       when storing in a vector<type*>.
		//       FIXME: Why???
		Expectation( Conversation *c, Persona *f, int num, WaitReply wr )
			: con( c ), from ( f ), messageNum( num ), waitForReply( wr ), expstr()
		{
		}

		Expectation( Conversation *c, Persona *f, int num, WaitReply wr, const String &str )
			: con( c ), from ( f ), messageNum( num ), waitForReply( wr ), expstr( str )
		{
		}

		Expectation operator=(const Expectation &e)
		{
			this->con = e.con;
			this->from = e.from;
			this->waitForReply = e.waitForReply;
			this->expstr = e.expstr;
			return *this;
		}
};

class Message
{
	public:
		Conversation	*con;
		Persona			*from;
		String			text; // unprocessed message tokens.
		int				messageNum;

		Message( Conversation *c, Persona *f, const String & t, int num )
			: con( c ), from( f ), text( t ), messageNum( num )
		{
		}

		Message operator=(const Message &m)
		{
			this->con = m.con;
			this->from = m.from;
			this->text = m.text;
			this->messageNum = m.messageNum;
			return *this;
		}
};

}

#endif // ANGEL_PERSONA_INCLUDED

