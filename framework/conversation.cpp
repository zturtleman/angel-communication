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

#include <stdio.h> // for printf
#include <cassert>
#include "conversation.h"
#include "persona.h"
#include "angel.h" // include imported functions

namespace AngelCommunication
{

Conversation::Conversation()
	: messageNum( 0 )
{
}

size_t Conversation::getMessageNum( ) {
	return this->messageNum;
}

size_t Conversation::numPersonas( ) {
	return this->personas.size();
}

void Conversation::addPersona( Persona *persona )
{
	assert( persona != NULL );

	// check if already in list
	for ( int i = 0; i < this->personas.size(); i++ )
	{
		if ( this->personas[i] == persona )
		{
			return;
		}
	}

	// add to list
	this->personas.push_back( persona );
	this->lastAddressee.push_back( ""  );	// default will be *nobody or *anybody depending on number of personas in conversation

	// notify
	for ( int i = 0; i < this->personas.size(); i++ )
	{
		if ( this->personas[i] == persona )
			continue;

		this->personas[i]->personaConnect( this, persona );
	}
}

void Conversation::removePersona( Persona *persona )
{
	for ( int i = 0; i < this->personas.size(); i++ )
	{
		if ( this->personas[i] == persona )
		{
			this->personas.erase( this->personas.begin() + i );
			this->lastAddressee.erase( this->lastAddressee.begin() + i );
			break;
		}
	}
}

void Conversation::addMessage( Persona *speaker, const String & message )
{
	Lexer lines;
	Lexer messageLine;
	String addressee, greetingAddressee;

	messageNum++;

	lines.splitSentences( message );

	ANGELC_PrintMessage( this, speaker, message.c_str() );

#if 0
	// print out split sentences for debugging
	for ( int j = 0; j < lines.getNumTokens(); j++ ) {
		String sentence;

		sentence.snprintf( 1024, "Sentence %d: %s", j, lines[j].c_str() );

		ANGELC_PrintMessage( this, speaker, sentence.c_str() );
	}
#endif

	//
	// Try to detect who the message is addressed to
	//
	// This is far from perfect. It will help bots know if someone is not talking to them though.
	//
	// TODO: use time to determine if continuing to talk to someone or replying to someone else without using their name.
	// TODO: try to detect cases where: Alice: Dave, I don't understand. Bob: Hi Alice. Alice: Hi. (Alice isn't talking to Dave)
	//
	for ( int j = 0; j < lines.getNumTokens(); j++ ) {
		messageLine.clear();
		messageLine.parse( lines[j].c_str() );

		int greetingNum = Persona::GetGreetingAddressee( messageLine, greetingAddressee );

		if ( greetingNum != -1 ) {
			if ( greetingAddressee.isEmpty() ) {
				// Ex: "Hi"
				// FIXME: if this greeting is a reply to someone elses greeting, should probably use "*nobody" so bot doesn't reply with a greeting
				addressee = "*anybody";
			} else {
				// Ex: "Hi everyone"
				if ( !greetingAddressee.icompareTo( "everyone" ) ) {
					addressee = "*anybody";
				}
			}
		} else {
			// Ex: "Bob: hi" or even just "Bob"
			greetingAddressee = messageLine[0];

			// Ex: Anyone know what's up?
			if ( !greetingAddressee.icompareTo( "anyone" ) ) {
				addressee = "*anybody";
			}
		}

		if ( !greetingAddressee.isEmpty() ) {
			for ( int i = 0; i < this->personas.size(); i++ ) {
				if ( !this->personas[i]->getNick().icompareTo( greetingAddressee ) ) {
					addressee = greetingAddressee;
					break;
				}
			}

			// TODO: handle cases where greeted someone who isn't here?
		}

		// use last person they addressed or update last addressee
		for ( int i = 0; i < this->personas.size(); i++ )
		{
			if ( this->personas[i] != speaker )
				continue;

			if ( addressee.isEmpty() ) {
				addressee = this->lastAddressee[i];

				// default to nobody if it's a group chat, as bot may join a IRC channel and should not thank everyone is talking to it.
				if ( addressee.isEmpty() ) {
					if ( numPersonas() > 2 ) {
						addressee = "*nobody";
					} else {
						addressee = "*anybody";
					}
				}
			} else {
				this->lastAddressee[i] = addressee;
			}
			break;
		}

#if 0
		// show addressee for debugging
		String bigBrother;
		bigBrother.snprintf( 1024, "Big Brother: Addressed to %s: %s", addressee.c_str(), lines[j].c_str() );
		ANGELC_PrintMessage( this, speaker, bigBrother.c_str() );
#endif

		// give message line to personas that it's addressed to
		for ( int i = 0; i < this->personas.size(); i++ )
		{
			if ( this->personas[i] == speaker )
				continue;

			this->personas[i]->receiveMessage( this, speaker, lines[j], messageNum, addressee );
		}
	}
}

} // end namespace AngelCommunication

