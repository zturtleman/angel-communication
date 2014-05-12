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

namespace AngelCommunication
{

enum Gender
{
	GENDER_NONE,
	GENDER_FEMALE,
	GENDER_MALE
};

class Persona
{
	private:
		String name;
		String namePossesive;
		Gender gender;
		String messageBate; // next message persona wants to hear
		bool funReplies;
		bool waitForReply;

		Lexer tokens; // unprocessed message tokens.

		std::clock_t lastUpdate;
		//int			thinkDelay;

	public:
		Persona();

		void setName( const String &name );
		void setGender( Gender gender );

		void welcome( Persona &target );
		void tell( Persona &target, String message );

		bool checkSubject( int subject );
		void think();
		void say( const String &message );
};

}

#endif // ANGEL_PERSONA_INCLUDED

