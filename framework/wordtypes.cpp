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

#include "wordtypes.h"

namespace AngelCommunication
{

const char *fillerWords[] = {
	"a", "an", "the", "so", "be", "eh", "um", "ah", "oh", "hhhhhh", "mm", "mmm"
};

const char *cancelWords[] = {
	"nothing", "nevermind", "nm", "never mind" /* FIXME can't have spaces in words yet */
};

const char *falseWords[] = {
	"false", "no",
};

const char *trueWords[] = {
	"true", "yes", "yeah", "okay", "mm", "mmm"
};

int	WordType( const String & str ) {
	int i;
	int type = 0;
	Lexer tokens( str );

	// assume filler unless proven otherwise
	type |= WT_FILLER;

	for ( int w = 0; w < tokens.getNumTokens(); w++ ) {
		for ( i = 0; i < ARRAY_LEN( fillerWords ); i++ ) {
			if ( tokens[w].icompareTo( fillerWords[i] ) == 0 ) {
				break;
			}
		}

		// this word isn't a filler, remove filler flag
		if ( i == ARRAY_LEN( fillerWords ) ) {
			type &= ~WT_FILLER;
		}

		for ( i = 0; i < ARRAY_LEN( cancelWords ); i++ ) {
			if ( tokens[w].icompareTo( cancelWords[i] ) == 0 ) {
				type |= WT_CANCEL_QUEST;
				break;
			}
		}

		for ( i = 0; i < ARRAY_LEN( falseWords ); i++ ) {
			if ( tokens[w].icompareTo( falseWords[i] ) == 0 ) {
				type |= WT_FALSE;
				break;
			}
		}

		for ( i = 0; i < ARRAY_LEN( trueWords ); i++ ) {
			if ( tokens[w].icompareTo( trueWords[i] ) == 0 ) {
				type |= WT_TRUE;
				break;
			}
		}
	}

	return type;
}

} // end namespace AngelCommunication

