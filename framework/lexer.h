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

#ifndef ANGEL_LEXER_INCLUDED
#define ANGEL_LEXER_INCLUDED

#include <vector>

#include "string.h"

namespace AngelCommunication
{

/*
    Lexer class
    Parse and store a string of text as tokens.
*/
class Lexer
{
    private:
        std::vector<String> tokens;

    public:
        Lexer(void);
		Lexer(const String &text);
        ~Lexer(void);
        void clear(void);
        void parse(const String &text);
        size_t getNumTokens(void) const;
        String getToken(unsigned int index) const;
        String operator[](unsigned int index) const;

		int findExact(const String &needle) const;
		int findPartial(const String &needle) const;

		String toString(unsigned int first = 0, unsigned int last = -1) const;
};


} // end namespace AngelCommunication

#endif // ANGEL_LEXER_INCLUDED

