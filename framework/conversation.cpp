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

namespace AngelCommunication
{

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

	// notify
	for ( int i = 0; i < this->personas.size(); i++ )
	{
		if ( this->personas[i] == persona )
			continue;

		this->personas[i]->personaConnect( *this, *persona );
	}
}

void Conversation::removePersona( Persona *persona )
{
	for ( int i = 0; i < this->personas.size(); i++ )
	{
		if ( this->personas[i] == persona )
		{
			this->personas.erase( this->personas.begin() + i );
			break;
		}
	}
}

void Conversation::addMessage( Persona *speaker, const String & message )
{
	// FIXME: showing message needs to be done outside the framework.
	printf("%s> %s\n", speaker->getName().c_str(), message.c_str() );

	for ( int i = 0; i < this->personas.size(); i++ )
	{
		if ( this->personas[i] == speaker )
			continue;

		this->personas[i]->receiveMessage( *this, *speaker, message );
	}
}

} // end namespace AngelCommunication

