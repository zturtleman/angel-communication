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

		std::clock_t lastUpdate;
		//int			thinkDelay;

	public:
		Persona();

		void think();
		bool processMessage( Message *message );

		void setName( const String &name );
		void setGender( Gender gender );
		void setAutoChat( bool autoChat );

		const String &getName( void ) const;


		// Conversation communication
		void receiveMessage( Conversation *con, Persona *speaker, const String &text );
		void personaConnect( Conversation *con, Persona *persona );
};

class Expectation
{
	public:
		Conversation	*con;
		Persona			*from;
		WaitReply		waitForReply;	// expectation type
		String			expstr;			// varies by expectation type

		Expectation( Conversation *c, Persona *f, WaitReply wr )
			: expstr()
		{
			this->con = con;
			this->from = from;
			this->waitForReply = wr;
		}

		Expectation( Conversation *c, Persona *f, WaitReply wr, const String &str )
			: expstr( str )
		{
			this->con = con;
			this->from = from;
			this->waitForReply = wr;
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

		Message( Conversation *c, Persona *f, const String & text )
			: con( c ), from( f ), text( text )
		{
		}

		Message operator=(const Message &m)
		{
			this->con = m.con;
			this->from = m.from;
			this->text = m.text;
			return *this;
		}
};

}

#endif // ANGEL_PERSONA_INCLUDED

