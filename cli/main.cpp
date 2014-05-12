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

#include <iostream>
#include <cmath>
#include <stdio.h>
#ifndef _WIN32
#include <termios.h>
#endif

#include "../framework/angel.h"

using namespace AngelCommunication;

bool charAvailable( float waitInSeconds ) {
	fd_set rfds;
	struct timeval tv;
	int retval;

	double fractpart, intpart;

	/* Watch stdin (fd 0) to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	fractpart = modf(waitInSeconds , &intpart);

	/* Wait up to half a second. */
	tv.tv_sec = intpart;
	tv.tv_usec = 1000000.0f * fractpart;

	retval = select(1, &rfds, NULL, NULL, &tv);

	if (retval == -1)
		return 0;

	if (FD_ISSET(0, &rfds))
		return 1;

	return 0;
}

int main( int argc, char **argv )
{
#ifndef _WIN32
	struct termios oldt, newt;

	tcgetattr( STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	//newt.c_lflag &= ~(ICANON|ECHO|IEXTEN);
	/* also set TIME to 0 and MIN to 1 (getc will block until there 
		is atleast 1 char avalible, and return as soon as there is)*/
	//newt.c_cc[VTIME] = 0;
	//newt.c_cc[VMIN] = 1;
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);
#endif

	printf("Angel Communication CLI\n");
	printf("Type 'quit' for exit Angel Communication.\n");

	Persona user, bot;

	user.setName( "User" );
	user.setGender( GENDER_MALE );

	bot.setName( "Angel" );
	bot.setGender( GENDER_FEMALE );

	std::string text;
	int ch;

	while (1)
	{
		if ( charAvailable( 5.0f ) )
			ch = getchar();
		else
			ch = EOF;

		//printf("hit %d\n", ch);

		if (ch == '\n')
		{
			if ( text == "quit" )
			{
				printf("\rQuiting Angel Communication\n");
				fflush(stdout);
#ifndef _WIN32
				tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
#endif
				return 0;
			}

			if (text.size() > 0)
			{
				printf("\r");
				fflush(stdout);

				user.say( text.c_str() );

				user.tell( bot, text.c_str() );

				text.clear();
			}
		}
		else if ( ch == 127 ) // Mac OS X 'delete' (usually called backspace)
		{
			if ( text.size() > 0 )
				text.erase(text.size()-1);
			printf( "\r%s \b", text.c_str() );
			fflush(stdout);
		}
		else if ( ch != EOF && ( ch > 32 || ch == ' ' ) )
		{
			text.push_back(ch);
			putchar(ch);
			fflush(stdout);
		}

		bot.think();
	}

	// never reached
	return 0;
}

