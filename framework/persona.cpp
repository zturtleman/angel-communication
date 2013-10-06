/*
Angel Communication
Copyright (C) 2013 Zack Middleton <zturtleman@gmail.com>

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

#include <stdio.h>

#include "persona.h"

namespace AngelCommunication
{

Persona::Persona()
{
	this->name = "unknown";
	this->gender = GENDER_NONE;

	this->lastUpdate = std::clock();
}

void Persona::setName( const String &name )
{
	this->name = name;
}

void Persona::setGender( Gender gender )
{
	this->gender = gender;
}

void Persona::tell( Persona &target, String message )
{
	// FIXME: target needs to store who said the tokens... might want to separate lines too?
	target.tokens.parse( message );
}

void Persona::say( const String &message )
{
	printf("%s> %s\n", this->name.c_str(), message.c_str() );
}

static double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks)/(CLOCKS_PER_SEC/1000);
    return diffms;
}

void Persona::think() {
#if 0 // ZTM: doesn't work if using select sleep in CLI main.cpp ...
	std::clock_t time = std::clock();

	// msec
	double sinceLastUpdate = diffclock(time, this->lastUpdate);

	// wait 2 seconds between updates.
	if ( sinceLastUpdate < 2000 ) {
		printf("skip think, frametime=%f\n", sinceLastUpdate );
		return;
	}

	this->lastUpdate = time;

	printf("frametime=%f\n", sinceLastUpdate );
#endif

	if ( !this->tokens.getNumTokens() ) {
		return;
	}

	// fun ugly hack for testing, answer all questions with your name
	if ( this->tokens.findPartial("name") != -1 && this->tokens.findPartial("?") != -1 )
	{
		String s("My name is ");
		s.append(this->name);
		s.append(".");
		say(s);
	}
	else
	{
		String s("What does '");
		s.append(this->tokens[0]);
		s.append("' mean?");
		say(s);
	}

	this->tokens.clear();
}

} // end namespace AngelCommunication

