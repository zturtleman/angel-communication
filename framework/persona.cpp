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
#include "lexer.h"

namespace AngelCommunication
{

Persona::Persona()
{
	this->name = "unknown";
	this->gender = GENDER_NONE;
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
	Lexer tokens;

	tokens.parse( message );

	// ZTM: Tempory debug messages
	printf( "numTokens=%d\n", tokens.getNumTokens() );
	for (int i = 0; i < tokens.getNumTokens(); ++i )
		printf("token[%d]='%s'\n", i, tokens[i].c_str());

}

} // end namespace AngelCommunication

