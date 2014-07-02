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

#include <cctype>
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
	this->spaceAfterToken.clear();
}

static bool incharset( char c, const char *charset ) {
	const char *p = charset;

	while ( *p ) {
		if ( c == *p ) {
			return true;
		}
		++p;
	}

	return false;
}

static const char *strchrset( const char *str, const char *charset ) {
	const char *p = str;

	while ( *p ) {
		if ( incharset( *p, charset ) )
			return p;

		++p;
	}

	return NULL;
}

// Check if should split text at index.
// This assumes you want to split here because of a special character, .!?
bool checkPunctSplit( const String &text, int index ) {
	const char *str = text.c_str();

	// don't end in middle of a number or a acronym
	if ( !isspace( str[index+1] ) && str[index+1] != '\0' ) {
		return true;
	}

	// determine if this is a word break.

	if ( str[index] == '.' ) {
		// check if it's end of a acronym
		// find start of the token
		int	tokenStart = index;
		while ( tokenStart > 0 && str[tokenStart-1] != ' ' ) {
			tokenStart--;
		}

		// count '.'s, upper case, and lower case
		int dotsInToken = 0, nonDotsInToken = 0;
		const char *s = &str[tokenStart];
		while ( *s && *s != ' ' ) {
			if ( *s == '.' )
				dotsInToken++;
			else
				nonDotsInToken++;
			s++;
		}

		// Might be an initial as part of a name (ex: K. Falcon) or [o]k[ay]. falcon. ;/
		//if ( nonDotsInToken == 1 && dotsInToken == 1 ) {
		//	return false;
		//}

		// Don't skip acronyms (ex: a.n.g.e.l. blah)
		if ( dotsInToken > 1 ) {
			return false;
		}
	}

	return true;
}

// Adds the tokens to the lexer from text separated by white space.
// graphic character that are not alphabet or numbers are always separated from beginning and end of tokens.
void Lexer::parse(const String &text)
{
	int tokenStart = -1;
	bool marks;
	const char punctuation[] = ".!?"; // sentence punctuation

    for (size_t i = 0, len = text.getLen()+1; i < len; i++)
    {
		if ( incharset( text[i], punctuation ) && !checkPunctSplit( text, i ) )
		{
			continue;
		}

		marks = false;

		if ( ispunct( text[i] ) ) {
			// split off from beginning
			if ( tokenStart == -1 )
				marks = true;
			// split off from end
			else if ( i < len - 1 )
			{
				bool hasCharBeforeSpace = false;

				for ( size_t j = i+1; j < len; j++ ) {
					if ( text[j] == ' ' ) {
						break;
					}
					if ( isalnum( text[j] ) ) {
						hasCharBeforeSpace = true;
						break;
					}
				}

				if ( !hasCharBeforeSpace || !text[i+1] )
					marks = true;
			}
		}

        if ( isspace( text[i] ) || text[i] == '\0' || marks )
        {
            if (tokenStart != -1) {
				this->spaceAfterToken.push_back( isspace( text[i] ) );
				this->tokens.push_back(text.subscript(tokenStart, i - 1));
				tokenStart = -1;
			}

			if (marks) {
				this->spaceAfterToken.push_back( ( i < len-1 && isspace( text[i+1] ) ) );
				this->tokens.push_back(text.subscript(i, i));
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

/*
	splitSentences: Split text into separate sentences for individual parsing later

	Split at each .!? if

		* contains multiple punctuation characters in a row
		* or has a space after the punctuation character
		* if punctuation is '.', only splits if proceeding token does not contain multiple '.'s without spaces (ex: A.N.G.E.L. or ._.)

	Others may be numbers (ex: 1.0) or acronyms (ex: A.N.G.E.L.) or emoticons (ex: ._.).

	Tokens contain multiple '.'s without spaces (ex: A.N.G.E.L. or ._.) must have 2 '.'s to end a sentence (ex: I like A.N.G.E.L..).
		FIXME: I think usually each acronym may or may not end sentence. No special double dot. Do I care?

	FIXME: What if it's part of a name? blah K. Falcon blah

	TODO: Check if this handles emoticons correct.
*/
void Lexer::splitSentences(const String &text) {
	const char *dot, *p, *tokenStart;
	const char *start = text.c_str();
	const char punctuation[] = ".!?";

	p = dot = tokenStart = start;
	while ( dot ) {
		dot = strchrset( p, punctuation );
		if ( !dot ) {
			String s( text.subscript( (int)( tokenStart - start ), text.getLen() ) );
			s.trim();

			// make sure not to add an empty string.
			if ( s.getLen() > 0 )
				tokens.push_back( s );

			// NOTE: implicate ending that might be continued in next message
			break;
		}

		//
		int numDots = 1;
		while ( incharset( dot[numDots], punctuation ) ) {
			numDots++;
		}

		// always end at blah..blah or blah...... or B.L.A.H..
		if ( numDots == 1 && !checkPunctSplit( text, (int)( dot - start ) ) ) {
			p = dot + 1;
			continue;
		}

		String s( text.subscript( (int)( tokenStart - start ), (int)( dot - start ) + numDots ) );
		s.trim();
		tokens.push_back( s );

		// skip to after this '.' (or group of ..s) to find the next.
		p = tokenStart = dot + numDots;
	}
}

void Lexer::removeToken(unsigned int index) {
	if ( index >= this->tokens.size() ) {
		return;
	}

	this->tokens.erase( this->tokens.begin() + index );
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

String Lexer::toString( unsigned int first, unsigned int last, bool forceSpaces ) const
{
	if ( getNumTokens() == 0 )
		return String();

	String s(this->tokens[first]);

	if ( last > this->tokens.size()-1 )
	{
		last = this->tokens.size()-1;
	}

	for (int i = first+1; i <= last; ++i)
	{
		if ( forceSpaces || this->spaceAfterToken.size() < i || this->spaceAfterToken[i - 1] == true )
			s.append(" ");
		s.append(this->tokens[i]);
	}

	return s;
}

} // end namespace AngelCommunication

