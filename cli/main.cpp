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

#include <iostream>
#include <stdio.h>
#include "../framework/angel.h"

using namespace AngelCommunication;

int main( int argc, char **argv ) {
	printf("Angel Communication CLI\n");
	printf("Type 'quit' for exit Angel Communication.\n");

	std::string username = "Link";
	Persona user, bot;

	user.setName( username.c_str() );
	user.setGender( GENDER_MALE );

	bot.setName( "Angel" );
	bot.setGender( GENDER_FEMALE );

	// FIXME: bot cannot think unless user does input.
	while (1)
	{
		std::string text;

		// FIXME: read multiple words at once...
		std::cin >> text;

		if ( text == "quit" )
		{
			return 0;
		}

		printf( "%s: %s\n", username.c_str(), text.c_str() );

		user.tell( bot, text.c_str() );
	}

	return 0;
}

