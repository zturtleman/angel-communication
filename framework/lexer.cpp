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

#include "lexer.h"

namespace AngelCommunication
{

Lexer::Lexer()
{
}

Lexer::Lexer(const String &text)
{
    parse( text );
}

Lexer::~Lexer()
{
    clear();
}

void Lexer::clear()
{
	this->tokens.clear();
}

// Adds the tokens to the lexer from text separated by white space.
void Lexer::parse(const String &text)
{
	int tokenStart = -1;

    for (size_t i = 0, len = text.getLen()+1; i < len; i++)
    {
        if (text[i] == ' ' || text[i] == '\t' || text[i] == '\r'
            || text[i] == '\n' || text[i] == '\0')
        {
            if (tokenStart != -1) {
				this->tokens.push_back(text.subscript(tokenStart, i - 1));
				tokenStart = -1;
			}
        }
        else
        {
            if (tokenStart == -1) 
			{
				tokenStart = i;
			}
        }
    }
}

String Lexer::getToken(unsigned int index) const
{
    if (index >= this->tokens.size())
    {
        return String();
    }

    return this->tokens[index];
}

String Lexer::operator[](unsigned int index) const
{
    return getToken(index);
}

size_t Lexer::getNumTokens() const
{
    return this->tokens.size();
}

int Lexer::findExact(const String &needle) const
{
	for (int i = 0; i < this->tokens.size(); ++i)
	{
		if ( needle == this->tokens[i] )
			return i;
	}

	return -1;
}

int Lexer::findPartial(const String &needle) const
{
	for (int i = 0; i < this->tokens.size(); ++i)
	{
		if ( this->tokens[i].findString(needle) )
			return i;
	}

	return -1;
}

} // end namespace AngelCommunication

